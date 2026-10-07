"""List or extract the files in a stage .INT archive (\\DATA\\*.INT, \\DATAS\\*.INT).

Usage: python3 tools/dev/audit/int_archive.py <file.int> [output-dir]

Follows CdctrlMemIntgDecode and PackIntDecode (src/main/cdctrl.c). An archive is
a run of sections, each a PACKINT_FILE_STR header (magic 0x44332211, file count,
FILE_TYPE_ENUM, header, name and data sizes, then fnum + 1 file offsets into the
decoded data), a name block, and LZSS-compressed data. The name block holds a
(name offset, exact size) pair per file followed by the NUL-terminated names;
the decoded offsets are padded to 16 bytes. The game stops at the first section whose type
it does not handle; this reader stops there too. With an output directory each
file is written decompressed under its stored name (keep the directory out of
Git: the files are original game data). Run from the repository root.
"""
import struct
import sys
from pathlib import Path

PACKINT_MAGIC = 0x44332211
FILE_TYPES = {1: 'VRAM', 2: 'SND', 3: 'ONMEM', 4: 'R1', 5: 'R2', 6: 'R3', 7: 'R4'}
N, F, THRESHOLD = 4096, 18, 2
MAGICS = {b'\x0a\x54\xdf\x18': 'SPM', b'TIM2': 'TIM2'}


def lzss_decode(data, offset):
    """PackIntDecode: a u32 decoded size, a skipped word, then the LZSS stream."""
    size, = struct.unpack_from('<I', data, offset)
    pos = offset + 8
    ring = bytearray(N + F - 1)
    rp = N - F
    out = bytearray()
    flags = 0
    while len(out) < size:
        flags >>= 1
        if not flags & 0x100:
            flags = data[pos] | 0xff00
            pos += 1
        if flags & 1:
            c = data[pos]
            pos += 1
            out.append(c)
            ring[rp] = c
            rp = (rp + 1) & (N - 1)
        else:
            c1, c2 = data[pos], data[pos + 1]
            pos += 2
            c1 |= (c2 & 0xf0) << 4
            length = (c2 & 0x0f) + THRESHOLD
            for i in range(length + 1):
                c = ring[(c1 + i) & (N - 1)]
                out.append(c)
                ring[rp] = c
                rp = (rp + 1) & (N - 1)
    if len(out) != size:
        raise ValueError('LZSS stream overran its size at %#x' % offset)
    return bytes(out), pos


def sections(data):
    offset = 0
    while offset + 0x20 <= len(data):
        magic, fnum, ftype, head_size, name_size, data_size = struct.unpack_from('<6i', data, offset)
        if magic != PACKINT_MAGIC:
            raise ValueError('bad INT section magic at %#x' % offset)
        addresses = struct.unpack_from('<%dI' % (fnum + 1), data, offset + 0x20) if fnum else ()
        name_block = offset + head_size
        names = []
        for i in range(fnum):
            name_offset, size = struct.unpack_from('<2I', data, name_block + i * 8)
            start = name_block + fnum * 8 + name_offset
            name = data[start:data.index(b'\0', start)].decode('latin1')
            names.append((name, size))
        decoded = b''
        if data_size:
            decoded, _ = lzss_decode(data, offset + head_size + name_size)
        yield offset, ftype, fnum, addresses, names, decoded
        if ftype not in FILE_TYPES:
            return
        offset += head_size + name_size + data_size


def files(fnum, addresses, names, decoded):
    for i in range(fnum):
        name, size = names[i]
        start = addresses[i]
        if start + size > addresses[i + 1] or addresses[i + 1] > len(decoded):
            raise ValueError('file %s overruns its section' % name)
        yield name, decoded[start:start + size]


def main():
    data = Path(sys.argv[1]).read_bytes()
    out_dir = Path(sys.argv[2]) if len(sys.argv) > 2 else None
    for offset, ftype, fnum, addresses, names, decoded in sections(data):
        kind = FILE_TYPES.get(ftype, 'end (type %d)' % ftype)
        print('section at %#x: %s, %d files, %d bytes decoded' % (offset, kind, fnum, len(decoded)))
        if ftype not in FILE_TYPES:
            break
        for name, blob in files(fnum, addresses, names, decoded):
            magic = MAGICS.get(blob[:4], blob[:4].hex())
            print('  %-16s %8d bytes  %s' % (name, len(blob), magic))
            if out_dir:
                out_dir.mkdir(parents=True, exist_ok=True)
                (out_dir / name).write_bytes(blob)


if __name__ == '__main__':
    main()
