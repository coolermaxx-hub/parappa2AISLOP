"""Inspect original SPM packets in a user-supplied common.ipk; never extracts assets.

Offsets here are serialized file-format offsets, independent of host C++ layouts.
Bounds checks reject truncated/invalid archives instead of following arbitrary data.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct


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
        yield offset, end
        if tag_id == 6:
            return
        offset = end


def inspect_model(data):
    model = Record(data)
    if model.word(0) != 0x18df540a:
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
        for packet in packets:
            if not packet:
                continue
            report['packets'] += 1
            module = model.word(packet + 0x60)
            modules[str(module)] += 1
            for start, end in chunks(model, packet):
                report['chunks'] += 1
                prefix, _, _, vertices = model.read('<4I', start + 0x10)
                # normal, both-face and screen use three quadwords per vertex.
                if module not in (0, 1, 4):
                    raise ValueError('unhandled primary microprogram %d' % module)
                if start + 0x60 + prefix * 48 + vertices * 48 > end:
                    raise ValueError('vertex stream exceeds DMA chunk')
                lo, regs = model.read('<QQ', start + 0x20)
                if lo & 0x7fff or not (lo >> 46) & 1 or (lo >> 58) & 3 or lo >> 60 != 3 or regs != 0x512:
                    raise ValueError('unexpected clipped polygon GIF template')
                prim = (lo >> 47) & 0x7ff
                if prim & 7 != 5:
                    raise ValueError('clipped polygon is not a triangle fan')
                clipping_prims[hex(prim)] += 1
                if any(model.read('<8I', start + 0x30)):
                    raise ValueError('nonzero reserved chunk quadwords')
        if flags & 0x40:
            pair_count, mapping, contour = model.read('<3I', node + 0x19c)
            if pair_count > len(data) // 8:
                raise ValueError('invalid contour map size')
            pair_addresses = set()
            for start, end in chunks(model, contour):
                prefix, _, _, vertices = model.read('<4I', start + 0x10)
                if vertices % 2:
                    raise ValueError('odd contour vertex count')
                first_pair = start + 0x60 + prefix * 48
                if first_pair + vertices * 32 > end:
                    raise ValueError('contour stream exceeds DMA chunk')
                pair_addresses.update(range(first_pair, first_pair + vertices * 32, 64))
            geometry = next(packet for packet in packets if packet)
            for index in range(pair_count):
                source, destination = model.read('<2I', mapping + index * 8)
                model.read('<4f', geometry + source * 16)
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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    args = parser.parse_args()
    print(json.dumps(inspect_archive(args.archive.read_bytes()), indent=2, sort_keys=True))


if __name__ == '__main__':
    main()
