#!/usr/bin python3
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import os
import json
import tarfile
import shutil
import argparse
import logging
from hashlib import md5
from download_asset.download_asset import Getter
from tempfile import TemporaryDirectory
from dataclasses import dataclass, field

log = logging.getLogger(__name__)
logger = logging.getLogger("download_asset")
#logger.setLevel(logging.ERROR)


def get_0_versions():
    return {"boot.bin": "0.0.0",
            "updater.bin": "0.0.0", "ecoboot.bin": "0.0.0"}


def get_none_checksums():
    return {"boot.bin": None,
            "updater.bin": None, "ecoboot.bin": None}


def gen_version_json(workdir, versions, checksums=None):
    '''
    Create version.json in {workdir} for: boot.bin, updater.bin and ecoboot.bin if available in {workdir}
    - if no checksums provided, these will be calculated
    - if versions are not provided 0.0.0 versions are set
    '''
    version_json = {}
    # our dumb locations in json
    locations = {"boot.bin": "boot",
                 "updater.bin": "updater",
                 "ecoboot.bin": "bootloader"}

    for file in os.listdir(workdir):
        if file in locations.keys():
            log.info(f"File to md5 {file}")
            with open(file, "rb") as f:
                if checksums is None or checksums[file] is None:
                    md5sum = md5(f.read()).hexdigest()
                else:
                    md5sum = checksums[file]
                log.info(f"checksum: {md5sum}")
                version_json[locations[file]] = {
                    "filename": file, "version": versions[file], "md5sum": md5sum}
    with open(workdir + "/version.json", "w") as f:
        log.info(f"saving version.json: {version_json}")
        f.write(json.dumps(version_json, indent=4, ensure_ascii=True))


class Args:
    tag = ""
    asset = ""
    assetRepoName = "PureUpdater_RT.bin"
    assetOutName = "updater.bin"
    workdir = ""


class WorkOnTmp:
    def __init__(self):
        self.current = os.getcwd()
        self.tempdir = TemporaryDirectory()
        os.chdir(self.tempdir.name)

    def __del__(self):
        os.chdir(self.current)

    def name(self):
        return self.tempdir.name

    def current(self):
        return self.current


def get_last_version():
    g = Getter()
    g.repo = "PureUpdater"
    g.getReleases(None)
    return g.releases[0]["tag_name"]


@dataclass
class PackageOpts():
    '''
    options for gen_update_asset
        updater         : if set to value, copy updater.bin from directory, otherwise load latest from PurePupdater repository
        boot            : if set to value, copy bootloader to tar file - otherwise do not add boot.bin to package
        updater_version : updater version to set for updater in json file
        boot_version    : updater version to set for updater in json file
        package_name    : package name to create
        binaries        : what binaries to pack, optional: boot, ecoboot, updater
    '''
    updater: str = None
    boot: str = None
    ecoboot: str = None
    updater_version: str = None
    boot_version: str = None
    updater_checksum: str = None
    package_name: str = "update.tar"
    binaries: list = field(default_factory=lambda: ["updater"])


def gen_update_asset(opts: PackageOpts):
    '''
    Generates package: by options from PackageOpts
    '''
    g_updater = Getter()
    g_ecoboot = Getter()
    workdir = WorkOnTmp()
    versions = get_0_versions()

    if 'updater' in opts.binaries:
        if opts.updater is None:
            g_updater.repo = "PureUpdater"
            g_updater.getReleases(None)
            versions["updater.bin"] = g_updater.releases[0]["tag_name"]
            download_args = Args()
            download_args.asset = "updater.bin"
            download_args.tag = versions["updater.bin"]
            download_args.workdir = workdir.name()
            log.info(f"---> save file to: {workdir.name()}")
            g_updater.downloadRelease(download_args)
        else:
            shutil.copyfile(opts.updater, workdir.name() + "/updater.bin")

    if 'ecoboot' in opts.binaries:
        if opts.ecoboot is None:
            log.warning("BY DEFAULT WILL LOAD BOOTLOADER FOR T7")
            g_ecoboot.repo = "ecoboot"
            g_ecoboot.getReleases(None)
            versions["ecoboot.bin"] = g_ecoboot.releases[0]["tag_name"]
            download_args = Args()
            download_args.asset = "ecoboot.bin"
            download_args.assetRepoName = "ecoboot.bin"
            download_args.assetOutName = "ecoboot.bin"
            download_args.tag = versions["ecoboot.bin"]
            download_args.workdir = workdir.name()
            log.info(f"---> save file to: {workdir.name()}")
            g_ecoboot.listReleases(download_args)
            g_ecoboot.downloadRelease(download_args)
        else:
            shutil.copyfile(opts.ecoboot, workdir.name() + "/ecoboot.bin")

    if 'boot' in opts.binaries:
        shutil.copyfile(opts.boot, workdir.name() + "/boot.bin")

    checksums = get_none_checksums()

    if opts.updater_version is not None:
        versions["updater.bin"] = opts.updater_version
    if opts.boot_version is not None:
        versions["boot.bin"] = opts.boot_version
    if opts.updater_checksum is not None:
        checksums["updater.bin"] = opts.updater_checksum

    log.info("generating version json ...")

    gen_version_json("./", versions, checksums)

    log.info(f"writting {opts.package_name}...")
    with tarfile.open(name=opts.package_name, mode='x') as tar:
        for file in os.listdir("./"):
            tar.add(file)

    log.info(f"move {opts.package_name} to current location ...")
    shutil.copyfile(opts.package_name, workdir.current + '/' + opts.package_name)

    log.info(f"package generation done and copied to: {workdir.current}/{opts.package_name}!")

    return workdir.current + "/" + opts.package_name


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""package generator
    can be used to generate update package with release from updater repository or locally

    to create package with local boot.bin and updater.bin pass full paths, with updater set to version 0.0.3
        $python3 update_package_generator.py -u ~/workspace/mudita/PureUpdater2/build/updater/PureUpdater_RT.bin --updater_version "0.0.3" -b ~/workspace/mudita/MuditaMaster/build-rt1051-RelWithDebInfo/**/boot.bin
    to create default updater.bin package just call
        $python3 update_package_generator.py""",
                                     formatter_class=argparse.RawTextHelpFormatter
                                     )
    parser.add_argument('-w', '--workdir', help="Directory where package is build", default="./")
    parser.add_argument('-u', '--updater', help="Updater binary to use", default=None)
    parser.add_argument('-b', '--boot', help="Boot bin to use", default=None)
    parser.add_argument('--updater_version', help="Updater bin version to use", default=None)
    parser.add_argument('--boot_version', help="Boot bin version to use", default=None)
    args = parser.parse_args()

    log.info("creating temp dir...")

    # args.updater
    gen_update_asset()
