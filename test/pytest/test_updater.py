# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import pytest
from harness import log

def test_get_dom(harness):
    # reboot to update
    body = {"update": True, "reboot": True}
    result = harness.endpoint_request("developerMode", "post", body)
    log.info("data {}".format(result))
