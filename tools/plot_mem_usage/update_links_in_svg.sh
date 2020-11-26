#!/bin/bash
# Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#AUTHOR: Radosław Wicik
#E-MAIL: 
#Date: 2020-11-05 09:37:53
sed -E 's#(<text><tspan font-family="arial" >)([0-9a-zA-Z]{40})(</tspan></text>)#<a xlink:href="https://github.com/mudita/MuditaOS/commit/\2" target="_blank">\1\2\3</a>#g' mem.svg > mem2.svg
