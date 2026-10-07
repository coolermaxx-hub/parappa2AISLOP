import importlib.util
from pathlib import Path
import struct
import sys
import unittest

AUDIT = Path(__file__).resolve().parents[2] / 'tools/dev/audit'
sys.path.insert(0, str(AUDIT))
spec = importlib.util.spec_from_file_location('spm_reserved_fields', AUDIT / 'spm_reserved_fields.py')
fields = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fields)

spec = importlib.util.spec_from_file_location('spm_packets_test', Path(__file__).resolve().parent / 'spm_packets.py')
packet_tests = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packet_tests)

NODE = 0x80


def model_fixture():
    """The packet test's contour node, with the converter totals filled in.

    Its opaque packet at 0x240 is 36 quadwords long and holds one vertex, and
    its contour packet at 0x480 is 37 quadwords long. The node has no deform
    positions of its own.
    """
    data = packet_tests.model_fixture()
    struct.pack_into('<I', data, NODE + 0x168, 1)        # m_vertexCount
    struct.pack_into('<2I', data, NODE + 0x174, 36, 0)   # m_context1PacketQwords
    struct.pack_into('<I', data, NODE + 0x190, 1)        # unk190
    struct.pack_into('<I', data, NODE + 0x1a8, 37)       # m_contourPacketQwords
    struct.pack_into('<2I', data, 0x54, 1, 0)            # m_context1PacketCounts
    struct.pack_into('<I', data, 0x5c, 1)                # m_vertex_num
    return data


def broken(checks):
    return sorted(name for name, counter in checks.items() if counter[False])


class ReservedFieldTests(unittest.TestCase):
    def test_fixture_satisfies_every_relation(self):
        checks = fields.check_model(model_fixture())
        self.assertEqual(broken(checks), [])
        self.assertEqual(checks['m_context1PacketQwords = packet quadwords'][True], 1)
        self.assertEqual(checks['m_contourPacketQwords = contour packet quadwords'][True], 1)

    def test_packet_size_must_include_node_upload(self):
        data = model_fixture()
        struct.pack_into('<I', data, NODE + 0x174, 36 - 0x1a0 // 16)
        self.assertEqual(broken(fields.check_model(data)), ['m_context1PacketQwords = packet quadwords'])

    def test_null_packet_has_no_size(self):
        data = model_fixture()
        struct.pack_into('<I', data, NODE + 0x178, 4)
        self.assertEqual(broken(fields.check_model(data)), ['null packet has 0 quadwords'])

    def test_vertex_count_and_header_total(self):
        data = model_fixture()
        struct.pack_into('<I', data, NODE + 0x168, 2)
        self.assertEqual(broken(fields.check_model(data)),
                         ['m_vertexCount = packet vertices', 'm_vertex_num = sum of m_vertexCount',
                          'unk190 = m_vertexCount + m_deformPositionCount'])

    def test_deform_positions_add_to_unk190_and_header(self):
        data = model_fixture()
        struct.pack_into('<I', data, NODE + 0x194, 3)    # m_deformPositionCount
        self.assertEqual(broken(fields.check_model(data)),
                         ['m_totalDeformPositionCount = sum of m_deformPositionCount',
                          'unk190 = m_vertexCount + m_deformPositionCount'])
        struct.pack_into('<I', data, NODE + 0x190, 4)
        struct.pack_into('<I', data, 0x60, 3)
        self.assertEqual(broken(fields.check_model(data)), [])

    def test_packet_counts_follow_the_packet_slot(self):
        data = model_fixture()
        struct.pack_into('<2I', data, 0x54, 0, 1)
        self.assertEqual(broken(fields.check_model(data)), ['m_context1PacketCounts = nodes with each packet'])


if __name__ == '__main__':
    unittest.main()
