# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
import pytest
import send_10k_sms_utils as utils

from harness import log


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_unlocked")
@pytest.mark.usefixtures("phone_in_desktop")
def test_send_10k_sms(harness, phone_number):
    initial_ping_count = utils.get_ping_count(harness, phone_number)
    initial_pong_count = utils.get_pong_count(harness, phone_number)

    utils.setup_for_test(harness, phone_number)

    sent_pong_count = 0
    total_pongs = utils.PING_PONG_COUNT

    while sent_pong_count < total_pongs:
        assert utils.wait_for_ping(harness, phone_number, sent_pong_count + initial_ping_count + 1)
        log.info("got ping...")

        utils.send_pong(harness)
        sent_pong_count += 1

        assert sent_pong_count == utils.get_pong_count(harness, phone_number) - initial_pong_count
        log.info(f"sent pong {sent_pong_count}/{total_pongs}...")


