"""Summarize the values of the unnamed SPM node and header fields.

Usage: python3 tools/dev/audit/spm_reserved_fields.py assets/common_ipk.databin.bin

Walks every unrelocated version-5 SPM in a common.ipk (or one .spm file) with
the same readers as spm_packets.py and prints, per field, how many distinct
values occur and the most common ones. Offsets are serialized file offsets.
A record that ends before a field is counted as "past end", never as zero.
"""
import struct
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import spm_packets  # noqa: E402

SPM_MAGIC = b'\x0a\x54\xdf\x18'
IDENTITY = struct.pack('<16f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
HEADER_FIELDS = {'unk8': (0x8, 8), 'unk54': (0x54, 8), 'unk60': (0x60, 4), 'unk64': (0x64, 4)}
NODE_FIELDS = {'unk100': (0x100, 64), 'unk168': (0x168, 4), 'unk174': (0x174, 8),
               'unk18C': (0x18C, 8), 'unk1A8': (0x1A8, 8), 'm_reserved1BC': (0x1BC, 4)}


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


def main():
    data = Path(sys.argv[1]).read_bytes()
    values = {name: Counter() for name in list(HEADER_FIELDS) + list(NODE_FIELDS)}
    past_end = Counter()
    model_count = node_count = 0
    for blob in models(data):
        model = spm_packets.Record(blob)
        model_count += 1
        for name, (offset, size) in HEADER_FIELDS.items():
            values[name][describe(blob[offset:offset + size])] += 1
        count, table = model.word(0x68), model.word(0x74)
        for node in model.read('<%dI' % count, table):
            node_count += 1
            for name, (offset, size) in NODE_FIELDS.items():
                if node + offset + size > len(blob):
                    past_end[name] += 1
                    continue
                values[name][describe(blob[node + offset:node + offset + size])] += 1
    print('%d models, %d nodes' % (model_count, node_count))
    for name, counter in values.items():
        top = '; '.join('%s x%d' % item for item in counter.most_common(5))
        extra = ', %d past end' % past_end[name] if past_end[name] else ''
        print('%s: %d distinct%s; %s' % (name, len(counter), extra, top))


if __name__ == '__main__':
    main()
