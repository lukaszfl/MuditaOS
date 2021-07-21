#!/usr/bin python3
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import os
import sys
import json
from hashlib import md5


if __name__ == "__main__":
    # our dumb locations in json
    locations = {"boot.bin": "boot",
                 "updater.bin": "updater", "ecoboot.bin": "bootloader"}
    # we can pass versions as parameter if we wish
    versions = {"boot.bin": "0.0.0",
                "updater.bin": "0.0.1", "ecoboot.bin": "0.0.0"}

    version_json = {}
    workdir = sys.argv[1]

    for file in os.listdir(workdir):
        if file in locations.keys():
            print(f"File to md5 {file}")
            with open(file, "rb") as f:
                md5sum = md5(f.read()).hexdigest()
                print(f"checksum: {md5sum}")
                version_json[locations[file]] = {
                    "filename": file, "version": versions[file], "md5sum": md5sum}
    with open(workdir + "/version.json", "w") as f:
        print(f"saving version.json: {version_json}")
        f.write(json.dumps(version_json, indent=4, ensure_ascii=True))
