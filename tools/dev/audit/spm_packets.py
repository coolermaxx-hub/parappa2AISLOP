"""Inspect a user-supplied common.ipk or standalone SPM; never extracts assets.

Offsets here are serialized file-format offsets, independent of host C++ layouts.
Bounds checks reject truncated/invalid archives instead of following arbitrary data.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct


# PrGetInputVertexParameterNum and the corresponding vump_*.vsm load loops.
# Contour (module 2) is inspected separately as paired position/color vertices.
PRIMARY_VERTEX_QUADWORDS = {0: 3, 1: 3, 3: 4, 4: 3, 5: 3}


class Record:
    def __init__(self, data):
        self.data = data

    def read(self, fmt, offset):
        size = struct.calcsize(fmt)
        if offset < 0 or offset + size > len(self.data):
            raise ValueError('record access outside file at %#x (%d bytes)' % (offset, size))
        return struct.unpack_from(fmt, self.data, offset)

    def word(self, offset):
        return self.read('<I', offset)[0]


def onmem_files(data):
    archive = Record(data)
    offset = 0
    while True:
        count, kind, size, _ = archive.read('<4I', offset)
        if kind == 0:
            raise ValueError('archive has no ONMEM category')
        if size < 16 or offset + size > len(data):
            raise ValueError('invalid archive category size')
        if kind == 3:
            if count + 1 > (size - 16) // 4:
                raise ValueError('archive file table exceeds category')
            addresses = archive.read('<%dI' % (count + 1), offset + 16)
            if addresses[0] < 16 + 4 * (count + 1):
                raise ValueError('archive file overlaps table')
            for index, (start, end) in enumerate(zip(addresses, addresses[1:])):
                if not start <= end <= size:
                    raise ValueError('invalid archive file bounds')
                yield index, data[offset + start:offset + end]
            return
        offset += size


def chunks(model, packet):
    # Every node packet starts with the 0x1a0-byte node upload.
    offset = packet + 0x1a0
    while True:
        qwc, control = model.read('<HH', offset)
        end = offset + (qwc + 1) * 16
        if qwc < 5 or end > len(model.data):
            raise ValueError('invalid DMA chunk size at %#x' % offset)
        tag_id = (control >> 12) & 7
        if tag_id not in (1, 6):
            raise ValueError('unsupported DMA chunk id %d' % tag_id)
        # SendDisplayHeader establishes STCYCL(4, 4), STMOD(0) and no mask.
        # The tag carries NOP + unmasked V4-32 UNPACK at TOPS-relative zero.
        nop, unpack = model.read('<2I', offset + 8)
        if nop != 0 or unpack & 0xff00ffff != 0x6c008000:
            raise ValueError('unsupported chunk VIF upload at %#x' % offset)
        upload_quadwords = (unpack >> 16) & 0xff
        if upload_quadwords == 0:
            upload_quadwords = 256  # VIF NUM=0 encodes 256, not an empty upload.
        upload_end = offset + 16 + upload_quadwords * 16
        if upload_quadwords < 5 or upload_end + 16 != end:
            raise ValueError('VIF upload does not fit DMA chunk at %#x' % offset)
        # MSCNT resumes the selected microprogram; the other three words are NOP.
        if model.read('<4I', upload_end) != (0x17000000, 0, 0, 0):
            raise ValueError('unexpected chunk VIF continuation at %#x' % upload_end)
        if any(model.read('<8I', offset + 0x30)):
            raise ValueError('nonzero reserved chunk quadwords')
        yield offset, upload_end
        if tag_id == 6:
            return
        offset = end


def inspect_model(data):
    model = Record(data)
    if data[:4] != b'\x0a\x54\xdf\x18':
        return None
    version, flags = model.read('<HH', 4)
    if version != 5 or flags & 1:
        raise ValueError('requires unrelocated SPM version 5')
    count, table = model.word(0x68), model.word(0x74)
    if count > len(data) // 4:
        raise ValueError('invalid node count')
    nodes = model.read('<%dI' % count, table)
    report = Counter(models=1, nodes=count)
    modules = Counter()
    clipping_prims = Counter()
    for node in nodes:
        flags = model.word(node + 0x154)
        packets = model.read('<2I', node + 0x16c)
        geometry_positions = set()
        geometry = next((packet for packet in packets if packet), None)
        for packet in packets:
            if not packet:
                continue
            report['packets'] += 1
            module = model.word(packet + 0x60)
            modules[str(module)] += 1
            for start, upload_end in chunks(model, packet):
                report['chunks'] += 1
                prefix, _, _, vertices = model.read('<4I', start + 0x10)
                if module not in PRIMARY_VERTEX_QUADWORDS:
                    raise ValueError('unhandled primary microprogram %d' % module)
                stride = PRIMARY_VERTEX_QUADWORDS[module] * 16
                first_vertex = start + 0x60 + prefix * 48
                vertex_end = first_vertex + vertices * stride
                if vertex_end > upload_end:
                    raise ValueError('vertex stream exceeds VIF upload')
                # Deformation sources use the first non-null context-1 packet.
                if packet == geometry:
                    geometry_positions.update(range(first_vertex, vertex_end, stride))
                lo, regs = model.read('<QQ', start + 0x20)
                if lo & 0x7fff or not (lo >> 46) & 1 or (lo >> 58) & 3 or lo >> 60 != 3 or regs != 0x512:
                    raise ValueError('unexpected clipped polygon GIF template')
                prim = (lo >> 47) & 0x7ff
                if prim & 7 != 5:
                    raise ValueError('clipped polygon is not a triangle fan')
                clipping_prims[hex(prim)] += 1
        if flags & 0x40:
            pair_count, mapping, contour = model.read('<3I', node + 0x19c)
            if pair_count > len(data) // 8:
                raise ValueError('invalid contour map size')
            pair_addresses = set()
            for start, upload_end in chunks(model, contour):
                report['contour_chunks'] += 1
                prefix, _, _, vertices = model.read('<4I', start + 0x10)
                if vertices % 2:
                    raise ValueError('odd contour vertex count')
                first_pair = start + 0x60 + prefix * 48
                if first_pair + vertices * 32 > upload_end:
                    raise ValueError('contour stream exceeds VIF upload')
                pair_addresses.update(range(first_pair, first_pair + vertices * 32, 64))
            if geometry is None:
                raise ValueError('contour node has no source geometry packet')
            for index in range(pair_count):
                source, destination = model.read('<2I', mapping + index * 8)
                if geometry + source * 16 not in geometry_positions:
                    raise ValueError('contour source is not a geometry position')
                if contour + destination * 16 not in pair_addresses:
                    raise ValueError('contour destination is not a current/history pair')
            report['contour_pairs'] += pair_count
            report['contour_nodes'] += 1
    return report, modules, clipping_prims


def inspect_archive(data):
    totals, modules, prims = Counter(), Counter(), Counter()
    for index, data in onmem_files(data):
        try:
            result = inspect_model(data)
        except (ValueError, StopIteration) as error:
            raise ValueError('ONMEM file %d: %s' % (index, error)) from error
        if result is not None:
            totals.update(result[0])
            modules.update(result[1])
            prims.update(result[2])
    return dict(totals=totals, primary_packet_modules=modules, clipped_polygon_prims=prims)


def inspect_file(data, file_format='auto'):
    if file_format == 'auto':
        file_format = 'spm' if data[:4] == b'\x0a\x54\xdf\x18' else 'ipk'
    if file_format == 'ipk':
        return inspect_archive(data)
    result = inspect_model(data)
    if result is None:
        raise ValueError('not an SPM file')
    return dict(totals=result[0], primary_packet_modules=result[1],
                clipped_polygon_prims=result[2])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('asset', type=Path)
    parser.add_argument('--format', choices=('auto', 'ipk', 'spm'), default='auto')
    args = parser.parse_args()
    try:
        report = inspect_file(args.asset.read_bytes(), args.format)
    except (OSError, ValueError) as error:
        parser.error(str(error))
    print(json.dumps(report, indent=2, sort_keys=True))


if __name__ == '__main__':
    main()
