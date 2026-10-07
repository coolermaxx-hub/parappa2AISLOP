"""Summarize the SPM node and header fields that the engine never reads.

Usage: python3 tools/dev/audit/spm_reserved_fields.py assets/common_ipk.databin.bin

Walks every unrelocated version-5 SPM in a common.ipk (or one .spm file) with
the same readers as spm_packets.py and prints, per field, how many distinct
values occur and the most common ones, then checks the relations documented
in docs/spm-geometry-layout.md. Offsets are serialized file offsets.

A node record without a deform payload (flags & 0xff0) is 0x190 bytes long,
so fields at 0x190 and above are counted only for deform nodes. Bytes past
the end of a record belong to the next record and are never counted.
"""
import struct
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import spm_packets  # noqa: E402

SPM_MAGIC = b'\x0a\x54\xdf\x18'
IDENTITY = struct.pack('<16f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
DEFORM_PAYLOAD_MASK = 0xff0
CONTOUR_NODE = 0x40
SHORT_RECORD_SIZE = 0x190
HEADER_FIELDS = {'unk8': (0x8, 8), 'm_context1PacketCounts': (0x54, 8),
                 'm_totalDeformPositionCount': (0x60, 4), 'unk64': (0x64, 4)}
NODE_FIELDS = {'unk100': (0x100, 64), 'm_vertexCount': (0x168, 4),
               'm_context1PacketQwords': (0x174, 8), 'm_reserved18C': (0x18C, 4),
               'unk190': (0x190, 4), 'm_contourPacketQwords': (0x1A8, 4),
               'm_reserved1AC': (0x1AC, 4), 'm_reserved1BC': (0x1BC, 4)}


def describe(raw):
    if len(raw) == 64:
        return 'identity matrix' if raw == IDENTITY else raw.hex()
    return ' '.join('%#x' % word for word in struct.unpack('<%dI' % (len(raw) // 4), raw))


def models(data):
    if data[:4] == SPM_MAGIC:
        yield data
        return
    for _, blob in spm_packets.onmem_files(data):
        if blob[:4] == SPM_MAGIC:
            yield blob


def packet_shape(model, packet):
    """Return (quadwords, vertices) of a packet, node upload included."""
    end, vertices = packet, 0
    for start, upload_end in spm_packets.chunks(model, packet):
        vertices += model.word(start + 0x1c)
        end = upload_end + 16
    return (end - packet) // 16, vertices


def check_relations(model, node, flags, checks):
    packets = model.read('<2I', node + 0x16c)
    sizes = model.read('<2I', node + 0x174)
    present = [packet for packet in packets if packet]
    checks['at most one context-1 packet'][len(present) <= 1] += 1
    vertices = 0
    for packet, size in zip(packets, sizes):
        if not packet:
            checks['null packet has 0 quadwords'][size == 0] += 1
            continue
        quadwords, count = packet_shape(model, packet)
        vertices += count
        checks['m_context1PacketQwords = packet quadwords'][size == quadwords] += 1
    checks['m_vertexCount = packet vertices'][model.word(node + 0x168) == vertices] += 1
    if not flags & DEFORM_PAYLOAD_MASK:
        return
    deform_positions = model.word(node + 0x194)
    checks['unk190 = m_vertexCount + m_deformPositionCount'][
        model.word(node + 0x190) == model.word(node + 0x168) + deform_positions] += 1
    if flags & CONTOUR_NODE:
        quadwords, _ = packet_shape(model, model.word(node + 0x1a4))
        checks['m_contourPacketQwords = contour packet quadwords'][
            model.word(node + 0x1a8) == quadwords] += 1


def check_header(model, nodes, checks):
    packet_counts = [0, 0]
    vertices = deform_positions = 0
    translucent_groups_in_range = True
    for node in nodes:
        flags = model.word(node + 0x154)
        packets = model.read('<2I', node + 0x16c)
        for k, packet in enumerate(packets):
            packet_counts[k] += packet != 0
        vertices += model.word(node + 0x168)
        if flags & DEFORM_PAYLOAD_MASK:
            deform_positions += model.word(node + 0x194)
        # Only the translucent packet is queued under m_sortGroup.
        if packets[1] and model.word(node + 0x188) > 10:
            translucent_groups_in_range = False
    checks['m_context1PacketCounts = nodes with each packet'][
        list(model.read('<2I', 0x54)) == packet_counts] += 1
    checks['m_vertex_num = sum of m_vertexCount'][model.word(0x5c) == vertices] += 1
    checks['m_totalDeformPositionCount = sum of m_deformPositionCount'][
        model.word(0x60) == deform_positions] += 1
    checks['translucent nodes use sort groups 0 to 10'][translucent_groups_in_range] += 1


CHECKS = ('m_context1PacketCounts = nodes with each packet',
          'm_vertex_num = sum of m_vertexCount',
          'm_totalDeformPositionCount = sum of m_deformPositionCount',
          'translucent nodes use sort groups 0 to 10',
          'at most one context-1 packet', 'null packet has 0 quadwords',
          'm_context1PacketQwords = packet quadwords', 'm_vertexCount = packet vertices',
          'unk190 = m_vertexCount + m_deformPositionCount',
          'm_contourPacketQwords = contour packet quadwords')


def check_model(blob, checks=None):
    """Count, per relation, how many records of one SPM hold (True) or break it (False)."""
    if checks is None:
        checks = {}
    for name in CHECKS:
        checks.setdefault(name, Counter())
    model = spm_packets.Record(blob)
    count, table = model.word(0x68), model.word(0x74)
    nodes = model.read('<%dI' % count, table)
    check_header(model, nodes, checks)
    for node in nodes:
        check_relations(model, node, model.word(node + 0x154), checks)
    return checks


def main():
    data = Path(sys.argv[1]).read_bytes()
    values = {name: Counter() for name in list(HEADER_FIELDS) + list(NODE_FIELDS)}
    absent = Counter()
    checks = {name: Counter() for name in CHECKS}
    model_count = node_count = 0
    for blob in models(data):
        model = spm_packets.Record(blob)
        model_count += 1
        for name, (offset, size) in HEADER_FIELDS.items():
            values[name][describe(blob[offset:offset + size])] += 1
        check_model(blob, checks)
        count, table = model.word(0x68), model.word(0x74)
        for node in model.read('<%dI' % count, table):
            node_count += 1
            flags = model.word(node + 0x154)
            for name, (offset, size) in NODE_FIELDS.items():
                in_short_record = offset + size <= SHORT_RECORD_SIZE
                if (not in_short_record and not flags & DEFORM_PAYLOAD_MASK) or node + offset + size > len(blob):
                    absent[name] += 1
                    continue
                values[name][describe(blob[node + offset:node + offset + size])] += 1
    print('%d models, %d nodes' % (model_count, node_count))
    for name, counter in values.items():
        top = '; '.join('%s x%d' % item for item in counter.most_common(5))
        extra = ', %d records without the field' % absent[name] if absent[name] else ''
        print('%s: %d distinct%s; %s' % (name, len(counter), extra, top))
    for name, counter in checks.items():
        print('%s: %d of %d' % (name, counter[True], counter[True] + counter[False]))


if __name__ == '__main__':
    main()
