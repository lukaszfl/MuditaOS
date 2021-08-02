#!/usr/bin/python3
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import pytest
from harness import log
from harness.interface.defs import Method, Endpoint
from harness.rt_harness_discovery import get_rt1051_harness
from harness.harness import Harness
from harness.api.filesystem import put_file, get_file
from harness.api.developermode import PhoneModeLock
from harness.api.update import PhoneReboot, Reboot
from assets.update_package_generator import gen_update_asset, get_last_version

# !!! Please update your updater.bin path to your local path.
updater_bin_path = "/home/maciej/Repos/PureUpdater2/PureUpdater/build/updater/PureUpdater_RT.bin"

def get_version(harness: Harness):
    r = harness.request(Endpoint.DEVICEINFO, Method.GET, {}).response
    version = r.body["version"]
    sha = r.body["gitRevision"]
    return f"version: {version} sha: {sha}"


def disable_some_logs(harness: Harness):
    from harness.interface.defs import PureLogLevel
    for val in ["SysMgrService", "ServiceDesktop_w2", "CellularMux"]:
        ret = harness.request(Endpoint.DEVELOPERMODE, Method.POST, {
                              "log": True, "service": val, "level": PureLogLevel.LOGERROR.value})
        log.info(f"{ret.response}")


def general_test(harness: Harness, filename, expects='FAIL'):
    log.info(get_version(harness))
    PhoneModeLock(False).run(harness)
    put_file(harness, filename, "/sys/user")
    PhoneReboot(Reboot.UPDATE).run(harness)
    assert harness.connection.watch_port_reboot(300)

    harness = get_rt1051_harness(60 * 6)
    import time
    time.sleep(15)
    harness.unlock_phone()
    PhoneModeLock(False).run(harness)

    log.info(get_version(harness))
    get_file(harness, "updater.log", "./")
    with open("updater.log") as f:
        line = f.readline()
        assert expects in line
    #PhoneModeLock(True).run(harness)
    log.info("update done!")


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_simple_update(harness: Harness):
    filename = gen_update_asset(
       updater=updater_bin_path,
       updater_version = get_last_version())
    general_test(harness, filename, 'OK')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_checksum_fail_bad(harness: Harness):
    '''
    we can update only when checksum is proper
    '''
    filename = gen_update_asset(updater=updater_bin_path,
                                updater_checksum="deadbeefdeadbeefdeadbeefdeadbeef",
                                updater_version = get_last_version())
    general_test(harness, filename, 'FAIL')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_checksum_fail_none(harness: Harness):
    '''
    we can update only when checksum is proper
    '''
    filename = gen_update_asset(
        updater=updater_bin_path, updater_checksum="", updater_version = get_last_version())
    general_test(harness, filename, 'FAIL')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_checksum_fail_zero(harness: Harness):
    '''
    we can update only when checksum is proper
    '''
    filename = gen_update_asset(
        updater=updater_bin_path, updater_checksum="0", updater_version = get_last_version())
    general_test(harness, filename, 'FAIL')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_version_fail(harness: Harness):
    '''
    first release of updater.bin is set to 0.0.1, defaults are 0.0.0 in gen_version
    therefore update of updater should not happen and update should fail
          generally it's possible to update updater and boot.bin separatelly with separate package
          so this shouldn't be an issue
          on the other hand - if we want to not fail with partial update success - then
          it should make logic considerably more convoluted
    '''
    filename = gen_update_asset(
        updater=updater_bin_path, updater_version="0.0.0")
    general_test(harness, filename, 'FAIL')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_version_same(harness: Harness):
    '''
    we should fail with the latest version -as we should already have latest version
    '''
    
    filename = gen_update_asset(
        updater=updater_bin_path, updater_version= get_last_version())
    general_test(harness, filename, 'OK')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_version_newer(harness: Harness):
    '''
    we should succeed with version loaded +1 bigger
    '''
    version = get_last_version()
    version = version.split('.')
    version[-1] = str(int(version[-1]) + 1)
    version = ".".join(version)

    filename = gen_update_asset(
        updater=updater_bin_path, updater_version=version)
    general_test(harness, filename, 'OK')
