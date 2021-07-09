#!/usr/env python3
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import hashlib
import sys
import json


# $1 version.json
# $2 where are the binaries
if __name__ == "__main__":
    final = ""

    with open(sys.argv[1], 'r') as version:
        j = json.load(version)
        for key in j:
            if 'filename' in j[key]:
                f_name = j[key]['filename']
                m = hashlib.md5()
                with open(sys.argv[2] + "/" + f_name, 'rb') as f:
                    m.update(f.read())
                j[key]['md5'] = m.hexdigest()
        final = json.dumps(j, sort_keys=True, indent=4)

    if final != "":
        with open(sys.argv[1], 'w') as version:
            version.write(final)
