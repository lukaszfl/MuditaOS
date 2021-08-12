#!/usr/bin/python3
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import pytest
from harness import log
from harness.interface.defs import Method, Endpoint
from harness.harness import Harness
from harness.api.filesystem import put_file, get_file
from harness.api.update import Reboot
from assets.update_package_generator import gen_update_asset, get_last_version, PackageOpts
from harness.harnesscache import HarnessCache


def updater_bin_path(request):
    return request.config.option.updater_bin


def ecoboot_bin_path(request):
    return request.config.option.ecoboot_bin


def boot_bin_path(request):
    return request.config.option.boot_bin


def update_binaries(request):
    '''
    selected binaries list, possible options:
        boot
        ecoboot
        updater
    '''
    binaries = request.config.option.update_opts
    binaries = binaries.split(',')
    return binaries


def get_version(harness: Harness):
    r = harness.request(Endpoint.DEVICEINFO, Method.GET, {}).response
    version = r.body["version"]
    sha = r.body["gitRevision"]
    return f"version: {version} sha: {sha}"


@pytest.fixture
def update_options():
    opts = PackageOpts()
    return opts


@pytest.fixture()
def update_options_current_updater(request, update_options):
    update_options.binaries = update_binaries(request)
    update_options.updater = updater_bin_path(request)
    update_options.updater_version = get_last_version()
    update_options.ecoboot = ecoboot_bin_path(request)
    return update_options


@pytest.fixture(params=["", "0", "deadbeefdeadbeefdeadbeefdeadbeef"])
def update_options_failure_checksum(request):
    opts = PackageOpts()
    opts.updater_checksum = request.param
    opts.updater = updater_bin_path(request)
    opts.updater_version = get_last_version()
    opts.binaries = update_binaries(request)
    return opts


def general_test(harness: Harness, filename, expects='FAIL'):
    put_file(harness, filename, "/sys/user/")

    harness = HarnessCache.reset_phone(Reboot.UPDATE)

    log.info(get_version(harness))
    get_file(harness, "updater.log", "./")
    with open("updater.log") as f:
        line = f.readline()
        assert expects in line
    log.info("update done!")


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop", "phone_unlocked", "phone_mode_unlock")
def test_clean_json(harness: Harness, request, update_options_current_updater):
    opts = PackageOpts()
    opts.empty_clean = True
    opts.binaries = []
    filename = gen_update_asset(opts)
    general_test(harness, filename, 'OK')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop", "phone_unlocked", "phone_mode_unlock")
def test_simple_update(harness: Harness, update_options_current_updater):
    filename = gen_update_asset(update_options_current_updater)
    general_test(harness, filename, 'OK')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop", "phone_unlocked", "phone_mode_unlock")
def test_checksum_fail(harness: Harness, request, update_options_failure_checksum):
    '''
    tests for bad checksums
    '''
    filename = gen_update_asset(update_options_failure_checksum)
    general_test(harness, filename, 'FAIL')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop", "phone_unlocked", "phone_mode_unlock")
def test_version_fail(harness: Harness, update_options_current_updater: PackageOpts):
    '''
    first release of updater.bin is set to 0.0.1, defaults are 0.0.0 in gen_version
    therefore update of updater should not happen and update should fail
          generally it's possible to update updater and boot.bin separatelly with separate package
          so this shouldn't be an issue
          on the other hand - if we want to not fail with partial update success - then
          it should make logic considerably more convoluted
    '''
    opts = update_options_current_updater
    opts.updater_version = "0.0.0"
    filename = gen_update_asset(opts)
    general_test(harness, filename, 'FAIL')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop", "phone_unlocked", "phone_mode_unlock")
def test_version_same(harness: Harness, update_options_current_updater: PackageOpts):
    '''
    we should succeed with the same version (update of the same version, to same version is ok)
    '''
    filename = gen_update_asset(update_options_current_updater)
    general_test(harness, filename, 'OK')


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_in_desktop", "phone_unlocked", "phone_mode_unlock")
def test_version_newer(harness: Harness, request, update_options_current_updater):
    '''
    we should succeed with version loaded +1 bigger
    '''
    def bigger_version():
        version = get_last_version()
        version = version.split('.')
        version[-1] = str(int(version[-1]) + 1)
        version = ".".join(version)
        return version

    update_options = update_options_current_updater
    update_options.updater_version = bigger_version()

    filename = gen_update_asset(update_options)
    general_test(harness, filename, 'OK')
