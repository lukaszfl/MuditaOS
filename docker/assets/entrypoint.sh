#!/bin/bash
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

# entry point in docker, just runs "./docker/run.sh"

./docker/ci_actions.sh $@
