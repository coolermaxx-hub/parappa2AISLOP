"""Compare data symbol placement with the original executable, section by section.

For .data, .sdata, .sbss and .bss (or the sections given as arguments), take every symbol of
iso/SCPS_150.17 in address order, find the same symbol in build/SCPS_150.17.elf (names that occur
more than once are matched in address order), and print each point where its offset from the
section start changes. A clean section prints a single line with delta 0.

.bss and .sbss are not part of the ROM image, so a layout drift there is invisible to image
comparisons; run this after changing any uninitialized data or the slinky configuration.
Symbols the original lacks are ignored; a name present in both but placed elsewhere shows up as a
delta change that reverts at the next symbol."""
import collections
import subprocess
import sys

ORIGINAL = 'iso/SCPS_150.17'
OURS = 'build/SCPS_150.17.elf'


def sections(path):
    out = subprocess.run(['readelf', '-S', '-W', path], capture_output=True, text=True).stdout
    result = {}
    for line in out.splitlines():
        if ']' not in line:
            continue
        fields = line.split(']')[1].split()
        if len(fields) > 5 and fields[1] in ('PROGBITS', 'NOBITS'):
            result[fields[0]] = (int(fields[2], 16), int(fields[4], 16))
    return result


def symbols(path, section):
    start, size = sections(path)[section]
    out = subprocess.run(['mipsel-linux-gnu-nm', '-n', path], capture_output=True, text=True).stdout
    result = []
    for line in out.splitlines():
        fields = line.split()
        if len(fields) == 3:
            addr = int(fields[0], 16)
            if start <= addr < start + size:
                result.append((addr - start, fields[2]))
    return result


def compare(section):
    ours = collections.defaultdict(list)
    for offset, name in symbols(OURS, section):
        ours[name].append(offset)
    seen = collections.Counter()
    previous = None
    for offset, name in symbols(ORIGINAL, section):
        index = seen[name]
        seen[name] += 1
        if index < len(ours[name]):
            delta = ours[name][index] - offset
            if delta != previous:
                print('  %-32s original +%#x, ours +%#x, delta %#x' % (name, offset, ours[name][index], delta))
                previous = delta


def main():
    for section in sys.argv[1:] or ['.data', '.sdata', '.sbss', '.bss']:
        print(section)
        compare(section)


if __name__ == '__main__':
    main()
