"""Single 48-byte pose frame layout check against relay.c."""

import os
import unittest

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
RELAY_C = os.path.join(REPO_ROOT, "tracker-smoothtrack", "android", "relay.c")


class TestFrameLayout(unittest.TestCase):
    def test_packet_size_macro_is_48(self):
        with open(RELAY_C, encoding="utf-8") as fh:
            src = fh.read()
        self.assertIn("#define PACKET_SIZE 48", src)


if __name__ == "__main__":
    unittest.main(verbosity=2)
