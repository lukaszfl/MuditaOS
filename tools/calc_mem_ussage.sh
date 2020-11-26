#!/bin/bash
# Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

TEXT_SECTIONS=(
    boot_hdr
    text
    sysdata
    data
    bss
    ARM.extab
    ARM.exidx
)
#relative to source root
LINKER_FILE="board/rt1051/ldscripts/memory.ld"
SCRIPT=$(readlink -f $0)⏎
SCRIPT_DIR="$(dirname ${SCRIPT})"
SRC_ROOT=$(readlink -f ${SCRIPT_DIR}/..)


function help() {
    cat <<-MSGEND
		Calculate memory ussage in binnary.
		ussage:
		    $0 <build-directory>
		MSGEND
}

function calculate_mem_ussage() {
    OBJDUMP_RAW=$( objdump -h ${ELF} )
    SECTIONS_COUNT=${#TEXT_SECTIONS[@]}
    I=0
    SUM=0
    while [[ $I -lt ${SECTIONS_COUNT} ]]
    do
        SECTION=${TEXT_SECTIONS[$I]}
        SECTION_HEX_VALUE=$( echo "${OBJDUMP_RAW}" | grep "\.${SECTION}" | tr -s " " | cut -f4 -d" ")
        SECTION_VALUE=$( printf "%d" 0x${SECTION_HEX_VALUE} )
        #echo "-${SECTION}:${SECTION_HEX_VALUE}:${SECTION_VALUE}"
        SUM=$(( $SUM + $SECTION_VALUE ))
        I=$(( I + 1 ))
    done
    echo ${SUM}
}

function getRamLength() {
    LINKER_PATH=${SRC_ROOT}/${LINKER_FILE}
    LINKER_MEM_HEX=$( cat ${LINKER_PATH} | grep "BOARD_SDRAM_TEXT" | sed -E 's#(.* LENGTH = )([0-9a-fA-Fx]+) (.*)#\2#g' )
    LINKER_MEM=$( printf "%d" $LINKER_MEM_HEX )
    echo ${LINKER_MEM}
}


if [[ $# -ne 1 ]]; then
    help
    exit 1
fi

BUILD_DIR=$1
ELF=${BUILD_DIR}/PurePhone.elf

if [[ ! -e ${ELF} ]]; then
    echo "File '$ELF' not found"
    help
    exit 2
fi

USED_MEM=$( calculate_mem_ussage )
AVAILABLE_MEM=$( getRamLength )

MEM_USSAGE=$( echo "scale=2;  ${USED_MEM} / ${AVAILABLE_MEM}  * 100" | bc )

#echo 'USED:AVAILABLE:%'
echo "${USED_MEM}:${AVAILABLE_MEM}:${MEM_USSAGE}"

