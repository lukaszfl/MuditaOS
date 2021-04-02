# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
import time
import pytest

from harness import log
from harness.interface.defs import key_codes


PING_PONG_COUNT = 100


def get_message_by_text(harness, message: str, phone_number: str):
    body = {"category": "message", "messageBody": message, "phoneNumber": str(phone_number)}
    return harness.endpoint_request("messages", "get", body)["body"]


def get_text_count(harness, phone_number, text):
    msgs = get_message_by_text(harness, text, phone_number)
    return len(msgs)


def get_ping_count(harness, phone_number):
    return get_text_count(harness, phone_number, "ping")


def get_pong_count(harness, phone_number):
    return get_text_count(harness, phone_number, "pong")


def start_sms_app(harness):
    harness.connection.send_key_code(key_codes["enter"])
    harness.open_application("messages")
    assert harness.get_application_name() == "ApplicationMessages"


def send_sms(harness, text):
    log.info(f"sending text: {text}")
    harness.send_text(text)
    harness.connection.send_key_code(key_codes["enter"])


def send_ping(harness):
    send_sms(harness, "ping")


def send_pong(harness):
    send_sms(harness, "pong")


def wait_for_text(harness, phone_number, text, expected_count):
    wait = 3
    retires = 40
    while retires > 0:
        time.sleep(wait)
        count = get_text_count(harness, phone_number, text)
        if count == expected_count:
            return True
        else:
            retires -= 1
            log.info(f"Did not receive {text}. Retries left: {retires}. Waiting {wait}s...")

    return False


def wait_for_ping(harness, phone_number, expected_count):
    return wait_for_text(harness, phone_number, "ping", expected_count)


def wait_for_pong(harness, phone_number, expected_count):
    return wait_for_text(harness, phone_number, "pong", expected_count)


def setup_for_test(harness, phone_number):
    start_sms_app(harness)

    harness.connection.send_key_code(key_codes["left"])
    harness.send_text(str(phone_number))
    harness.connection.send_key_code(key_codes["down"])


