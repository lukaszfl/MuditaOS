# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import time

import pytest

import sys
import os.path

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir)))

from harness import log
from harness.harness import Harness
from harness import utils
from harness.interface.CDCSerial import Keytype, CDCSerial as serial
from harness.interface.defs import key_codes
from harness.harnesscache import HarnessCache


def pytest_addoption(parser):
    parser.addoption("--port", type=str, action="store", required=False)
    parser.addoption("--timeout", type=int, action="store", default=15)
    parser.addoption("--phone_number", type=int, action="store")
    parser.addoption("--call_duration", type=int, action="store", default=30)
    parser.addoption("--sms_text", type=str, action="store", default='')
    parser.addoption("--bt_device", type=str, action="store", default='')
    parser.addoption("--update_opts", type=str, action="store", default='', help='what update binaries we want to test with, space limited: boot,ecoboot,update select any')
    parser.addoption("--updater_bin", type=str, action="store", default=None, help='path to load updater_bin from - if not added, will try to load it from updater bin release page')
    parser.addoption("--ecoboot_bin", type=str, action="store", default=None, help='path to load ecoboot from - if not added, will try to load it from ecoboot bin release page')
    parser.addoption("--boot_bin", type=str, action="store", default=None, help='path to load boot from - if not added, will not add it')


@pytest.fixture(scope='session')
def phone_number(request):
    phone_number = request.config.option.phone_number
    assert phone_number
    return phone_number


@pytest.fixture(scope='session')
def call_duration(request):
    call_duration = request.config.option.call_duration
    assert call_duration
    return call_duration


@pytest.fixture(scope='session')
def sms_text(request):
    sms_text = request.config.option.sms_text
    assert sms_text != ''
    return sms_text


@pytest.fixture(scope='session')
def bt_device(request):
    bt_device = request.config.option.bt_device
    return bt_device


@pytest.fixture(scope='function')
def harness(request):
    '''
    Gets harness connection to be used with tests
    tries to init one Pure phone with:
    * automatically - depending on harness discovery aka get_pures method
    * serial port path if provided via config.option.port ( `--port` parameter)
    * virtual com used with simulator always named `/tmp/purephone_pts_name`
    '''
    port_name = request.config.option.port
    TIMEOUT = request.config.option.timeout
    RETRY_EVERY_SECONDS = 1.0

    if HarnessCache.cached() and HarnessCache.is_operational():
        return HarnessCache.harness

    try:
        HarnessCache.get(port_name, TIMEOUT, RETRY_EVERY_SECONDS)
    except utils.Timeout:
        pytest.exit("couldn't find any viable port. exiting")
    except ValueError as err:
        pytest.exit(f"harness discovery error! {err}")

    return HarnessCache.harness


@pytest.fixture(scope='session')
def harnesses():
    '''
    Automatically init at least two Pure phones
    '''
    found_pures = serial.find_Pures()
    harnesses = [Harness(pure) for pure in found_pures]
    if not len(harnesses) >= 2:
        pytest.skip("At least two phones are needed for this test")
    assert len(harnesses) >= 2
    return harnesses


@pytest.fixture()
def phone_unlocked(harness):
    harness.unlock_phone()
    assert not harness.is_phone_locked()


@pytest.fixture(scope='session')
def phone_locked(harness):
    harness.lock_phone()
    assert harness.is_phone_locked()


@pytest.fixture(scope='session')
def phones_unlocked(harnesses):
    for harness in harnesses:
        harness.unlock_phone()
        assert not harness.is_phone_locked()


@pytest.fixture()
def phone_in_desktop(harness: Harness):
    # go to desktop
    if harness.get_application_name() != "ApplicationDesktop":
        harness.return_to_home_screen()
        # in some cases we have to do it twice
        if harness.get_application_name() != "ApplicationDesktop":
            harness.return_to_home_screen()
    # assert that we are in ApplicationDesktop
    assert harness.get_application_name() == "ApplicationDesktop"


@pytest.fixture(scope='function')
def phone_ends_test_in_desktop(harness):
    yield
    target_application = "ApplicationDesktop"
    target_window     = "MainWindow"
    log.info(f"returning to {target_window} of {target_application} ...")
    time.sleep(1)

    if harness.get_application_name() != target_application :
        body = {"switchApplication" : {"applicationName": target_application, "windowName" : target_window }}
        harness.endpoint_request("developerMode", "put", body)
        time.sleep(1)

        max_retry_counter = 5
        while harness.get_application_name() != target_application:
            max_retry_counter -= 1
            if max_retry_counter == 0:
                break

            log.info(f"Not in {target_application}, {max_retry_counter} attempts left...")
            time.sleep(1)
    else :
        # switching window in case ApplicationDesktop is not on MainWindow:
        body = {"switchWindow" : {"applicationName": target_application, "windowName" : target_window }}
        harness.endpoint_request("developerMode", "put", body)
        time.sleep(1)

    # assert that we are in ApplicationDesktop
    assert harness.get_application_name() == target_application
    time.sleep(1)


@pytest.fixture(scope='function')
def phone_mode_unlock(harness):
    from harness.api.developermode import PhoneModeLock
    PhoneModeLock(False).run(harness)
    yield
    PhoneModeLock(True).run(HarnessCache.harness)
