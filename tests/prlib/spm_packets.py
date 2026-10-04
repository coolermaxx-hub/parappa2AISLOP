import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location(
    'spm_packets', Path(__file__).resolve().parents[2] / 'tools/dev/audit/spm_packets.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def model_fixture():
    data = bytearray(0x800)
    struct.pack_into('<IHH', data, 0, 0x18df540a, 5, 0)
    struct.pack_into('<I', data, 0x68, 1)
    struct.pack_into('<II', data, 0x74, 0x7c, 0)
    struct.pack_into('<I', data, 0x7c, 0x80)
    struct.pack_into('<I', data, 0x80 + 0x154, 0x40)
    struct.pack_into('<II', data, 0x80 + 0x16c, 0x240, 0)
    struct.pack_into('<3I', data, 0x80 + 0x19c, 1, 0x6d0, 0x480)
    # One primary vertex and its clipped triangle-fan GIF template.
    struct.pack_into('<HH', data, 0x3e0, 9, 0x6000)
    struct.pack_into('<4I', data, 0x3f0, 0, 0, 0x3f800000, 1)
    struct.pack_into('<QQ', data, 0x400, (3 << 60) | (5 << 47) | (1 << 46), 0x512)
    struct.pack_into('<2I', data, 0x3e8, 0, 0x6c088000)
    struct.pack_into('<I', data, 0x470, 0x17000000)
    # One current/history contour pair: two position/color vertices.
    struct.pack_into('<HH', data, 0x620, 10, 0x6000)
    struct.pack_into('<4I', data, 0x630, 0, 0, 0x3f800000, 2)
    struct.pack_into('<2I', data, 0x628, 0, 0x6c098000)
    struct.pack_into('<I', data, 0x6c0, 0x17000000)
    struct.pack_into('<II', data, 0x6d0, 0x20, 0x20)
    return data


class PacketInspectionTests(unittest.TestCase):
    def test_primary_and_contour_records(self):
        counts, modules, prims = audit.inspect_model(model_fixture())
        self.assertEqual(counts['chunks'], 1)
        self.assertEqual(counts['contour_pairs'], 1)
        self.assertEqual(modules, {'0': 1})
        self.assertEqual(prims, {'0x5': 1})

    def test_rejects_truncated_chunk(self):
        with self.assertRaisesRegex(ValueError, 'DMA chunk size'):
            audit.inspect_model(model_fixture()[:0x470])

    def test_rejects_history_as_pair_start(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x6d4, 0x22)
        with self.assertRaisesRegex(ValueError, 'current/history pair'):
            audit.inspect_model(data)

    def test_rejects_unexplained_reserved_data(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x410, 1)
        with self.assertRaisesRegex(ValueError, 'reserved chunk'):
            audit.inspect_model(data)

    def test_rejects_wrong_upload_destination(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x3ec, 0x6c088001)
        with self.assertRaisesRegex(ValueError, 'VIF upload'):
            audit.inspect_model(data)

    def test_rejects_upload_over_dma_boundary(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x3ec, 0x6c098000)
        with self.assertRaisesRegex(ValueError, 'VIF upload does not fit'):
            audit.inspect_model(data)

    def test_zero_upload_count_means_256(self):
        data = bytearray(0x1a0 + 258 * 16)
        struct.pack_into('<HH', data, 0x1a0, 257, 0x6000)
        struct.pack_into('<2I', data, 0x1a8, 0, 0x6c008000)
        struct.pack_into('<I', data, 0x11b0, 0x17000000)
        self.assertEqual(list(audit.chunks(audit.Record(data), 0)), [(0x1a0, 0x11b0)])

    def test_rejects_missing_continuation(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x470, 0)
        with self.assertRaisesRegex(ValueError, 'VIF continuation'):
            audit.inspect_model(data)

    def test_rejects_vertex_in_vif_commands(self):
        data = model_fixture()
        # Increase the vertex count and DMA upload by only two quadwords.
        # The third vertex quadword would overlap the MSCNT/NOP trailer.
        struct.pack_into('<H', data, 0x3e0, 11)
        struct.pack_into('<I', data, 0x3ec, 0x6c0a8000)
        struct.pack_into('<I', data, 0x3fc, 2)
        struct.pack_into('<I', data, 0x490, 0x17000000)
        with self.assertRaisesRegex(ValueError, 'vertex stream exceeds VIF'):
            audit.inspect_model(data)

    def test_rejects_color_as_source_position(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x6d0, 0x21)
        with self.assertRaisesRegex(ValueError, 'source is not a geometry position'):
            audit.inspect_model(data)

    def test_reflection_vertices_need_four_quadwords(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x2a0, 3)  # Reflection microprogram.
        with self.assertRaisesRegex(ValueError, 'vertex stream exceeds VIF'):
            audit.inspect_model(data)
        # A standalone reflection node with enough upload space for its normal.
        struct.pack_into('<I', data, 0x1d4, 0)  # No contour extension.
        struct.pack_into('<H', data, 0x3e0, 10)
        struct.pack_into('<I', data, 0x3ec, 0x6c098000)
        struct.pack_into('<I', data, 0x480, 0x17000000)
        self.assertEqual(audit.inspect_model(data)[1], {'3': 1})

    def test_antiline_vertices_use_three_quadwords(self):
        data = model_fixture()
        struct.pack_into('<I', data, 0x2a0, 5)
        self.assertEqual(audit.inspect_model(data)[1], {'5': 1})

    def test_standalone_model(self):
        self.assertEqual(audit.inspect_file(model_fixture())['totals']['models'], 1)
        with self.assertRaisesRegex(ValueError, 'not an SPM'):
            audit.inspect_file(b'no', 'spm')

    def test_archive_bounds(self):
        archive = struct.pack('<6I', 1, 3, 28, 0, 24, 28) + b'test'
        self.assertEqual(list(audit.onmem_files(archive)), [(0, b'test')])
        with self.assertRaisesRegex(ValueError, 'category size'):
            list(audit.onmem_files(archive[:-1]))


if __name__ == '__main__':
    unittest.main()
