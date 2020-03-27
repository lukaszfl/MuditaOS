import pytest
from harness.common import Serial
import logging


class TestBasic:

    @pytest.fixture(autouse=True)
    def setup_env(self):

        print("setup")

        logging.basicConfig(format='%(asctime)s %(message)s', level=logging.DEBUG)
        self.log = logging.getLogger(__name__)
        self.conn = Serial()

        yield "setup_env"

        print("teardown")

    def test_basic_unlock(self):
        # write pin
        self.conn.key(ord('\n'))
        self.conn.key(ord('#'))
        self.conn.key(3)
        self.conn.key(3)
        self.conn.key(3)
        self.conn.key(3)
        self.conn.key(ord('\n'))
        # enter menu
        self.conn.key(ord('\n'))
        self.conn.read(30)
