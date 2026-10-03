"""Check the derived wave model against the actual VU setup instructions.

This small evaluator covers only the straight-line setup block, using host real
arithmetic. It does not simulate VU rounding, latency, ESIN or rendered output.
An unsupported instruction fails rather than silently approximating execution.
"""
from pathlib import Path
import math
import re
import unittest

SOURCE = Path(__file__).resolve().parents[2] / 'src/prlib/vu1/vump_menderer_create_texture.vsm'
LANES = 'xyzw'


def texture_setup(records):
    vectors = {'VF%02d' % i: [0.0] * 4 for i in range(32)}
    vectors['VF00'][3] = 1.0
    accumulator = [0.0] * 4
    immediate = quotient = 0.0
    integers = {'VI00': 0}

    def read(operand):
        if operand == 'I':
            return [immediate] * 4
        if operand == 'Q':
            return [quotient] * 4
        if operand == 'ACC':
            return accumulator[:]
        if re.fullmatch(r'VF\d\d[xyzw]', operand):
            return [vectors[operand[:4]][LANES.index(operand[-1])]] * 4
        return vectors[operand][:]

    for line in SOURCE.read_text().splitlines():
        if line == 'menderer_process_sprite:':
            break
        if not line.strip() or line.endswith(':'):
            continue
        upper, lower = re.split(r'\s{2,}', line.strip(), maxsplit=1)
        if upper != 'NOP':
            operation, operands = upper.split(None, 1)
            match = re.fullmatch(r'(ADD|ADDA|MADD|MUL|SUB)([xyzwqiI]?)(?:\[i\])?\.([xyzw]+)', operation)
            if not match:
                raise ValueError('Unsupported upper instruction: ' + upper)
            family, _, mask = match.groups()
            destination, lhs, rhs = operands.split(', ')
            left, right = read(lhs), read(rhs)
            result = read(destination)
            for lane in mask:
                index = LANES.index(lane)
                if family in ('ADD', 'ADDA'):
                    result[index] = left[index] + right[index]
                elif family == 'SUB':
                    result[index] = left[index] - right[index]
                elif family == 'MUL':
                    result[index] = left[index] * right[index]
                elif family == 'MADD':
                    result[index] = accumulator[index] + left[index] * right[index]
            if destination == 'ACC':
                accumulator = result
            else:
                vectors[destination] = result

        # Lower LOI applies after the paired upper instruction reads old I.
        operation, *tail = lower.split(None, 1)
        operands = tail[0] if tail else ''
        if operation == 'LOI':
            immediate = float(operands)
        elif operation == 'LQI.xyzw':
            destination, address = operands.split(', ')
            register = address[1:-3]  # (VI02++)
            vectors[destination] = records[integers[register]][:]
            integers[register] += 1
        elif operation == 'IADDIU':
            destination, lhs, value = operands.split(', ')
            integers[destination] = integers[lhs] + int(value, 0)
        elif operation == 'DIV':
            destination, lhs, rhs = operands.split(', ')
            if destination != 'Q':
                raise ValueError('Unsupported DIV destination')
            quotient = read(lhs)[0] / read(rhs)[0]
        elif operation == 'MFIR.xz' or operation == 'MFIR.yw':
            # Buffer address bookkeeping does not enter the wave calculation.
            destination, register = operands.split(', ')
            for lane in operation.split('.')[1]:
                vectors[destination][LANES.index(lane)] = integers[register]
        elif operation != 'NOP':
            raise ValueError('Unsupported lower instruction: ' + lower)
    return vectors


class TextureModelTest(unittest.TestCase):
    def setup_case(self, amplitude, spatial, frequency, offset, time, band):
        records = [amplitude + [0], spatial + [0], frequency + [0],
                   offset + [0], time + [band]] + [[0] * 4 for _ in range(10)]
        state = texture_setup(records)
        for lane in range(3):
            self.assertAlmostEqual(state['VF30'][lane], 0.2734375 * amplitude[lane] / sum(amplitude), places=7)
            self.assertAlmostEqual(state['VF29'][lane], 2 * math.pi * spatial[lane] / 256, places=7)
            self.assertAlmostEqual(state['VF28'][lane], 2 * math.pi * (frequency[lane] * time[lane] + offset[lane]), places=6)
        self.assertEqual(state['VF25'][1], band)
        self.assertEqual(state['VF23'][0:2], [1 / 256, 1])
        self.assertEqual(state['VF26'], [0, 0, 1, 0])
        return state

    def test_three_independent_waves(self):
        self.setup_case([2, 3, 5], [1, 7, 13], [.3, .1, .5], [.1, .3, 1], [.6, .1, .25], 32)

    def test_single_active_wave(self):
        self.setup_case([0, 0, 8], [3, 5, 1], [.2, .4, .1], [0, 0, .25], [0, 0, .5], 64)

    def test_amplitude_normalization(self):
        arguments = ([1, 2, 3], [.3, .4, .5], [0, .25, .5], [1, 2, 3], 16)
        first = self.setup_case([2, 3, 5], *arguments)
        second = self.setup_case([4, 6, 10], *arguments)
        self.assertEqual(first['VF30'], second['VF30'])


if __name__ == '__main__':
    unittest.main()
