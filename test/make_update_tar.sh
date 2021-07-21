#!/usr/bin/env bash
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

set -eo pipefail

folder=$(mktemp -d update.XXX)
rm update.tar -f

if [[ $1 == "all" ]]; then
    pushd ../build-rt1051-RelWithDebInfo
    rm PurePhone* PurePhone-0.73.1-RT1051-Update/ -fdr
    ninja -j 7 PurePhone-UpdatePackage || echo "we cant build can we..."
    popd
    cp ../build-rt1051-RelWithDebInfo/PurePhone*Update/boot.bin $folder/boot.bin
    cp ../build-rt1051-RelWithDebInfo/PurePhone*Update/version.json $folder/version.json
fi

cp ../../PureUpdater2/build/updater/PureUpdater_RT.bin $folder/updater.bin

python3 version_json.py $folder

pushd "$folder"
tar -cf update.tar *
mv update.tar ../
popd

rm -fdr "$folder"
