#!/bin/bash
# Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#AUTHOR: Radosław Wicik
#E-MAIL: 
#Date: 2020-11-05 09:48:38

# get the wiki
# generate plot
# update links in plot
# add plot to repo
# add data to repo
# commit updates
# push updates

WIKI_DIR="/home/rockford/mudita/MuditaOS.wiki"
DATA_DIR="/home/rockford/mudita/Pure-master/tools"
DATA_FILE="mem_ussage.txt"
DATA_PATH=${DATA_DIR}/${DATA_FILE}

WIKI_DATA_FILE="mem_data.txt"
WIKI_TEMPLATE="Memory-Usage-data.md.template"
WIKI_PAGE="Memory-Usage-data.md"
WIKI_PLOT="memory_usage.svg"
PLOT_SCRIPT="plot_mem.gnu"
PLOT_OUT="mem.svg"

WIKI_TEMPLATE_PATH=${WIKI_DIR}/${WIKI_TEMPLATE}
WIKI_PAGE_PATH=${WIKI_DIR}/${WIKI_PAGE}
WIKI_DATA_PATH=${WIKI_DIR}/${WIKI_DATA_FILE}
WIKI_PLOT_PATH=${WIKI_DIR}/${WIKI_PLOT}

echo "" > ${WIKI_DATA_PATH}

DATA=( $( cat ${DATA_PATH} ) )

I=0
SIZE=${#DATA[@]}
#SIZE=3

while [[ $I -lt $SIZE ]];
do
    ITEM=${DATA[${I}]}
    #echo -en "${ITEM}"
    RE='^([a-zA-Z0-9]{40}):([0-9]+):([0-9]+):(.*)'
    if [[ ${ITEM} =~ $RE ]]; then
        #echo -e "YES"
        HASH=${BASH_REMATCH[1]}
        MEM=${BASH_REMATCH[2]}
        RAM=${BASH_REMATCH[3]}
        #echo -e "\t${HASH}--${MEM}--${RAM}"
        USAGE=$( echo "scale=6; ${MEM} / ${RAM} * 100 " | bc )
        #echo -e "\t${HASH}--${MEM}--${RAM}--${USAGE}"
        echo "${HASH} ${MEM} ${RAM} ${USAGE}" >> ${WIKI_DATA_PATH}
        #WIKI_DATA[${#WIKI_DATA[@]}]="(${HASH}) | ${MEM} | ${RAM} | ${USAGE}"
        WIKI_DATA=$(echo -e "${WIKI_DATA:+${WIKI_DATA}\n}|[${HASH}](https://github.com/mudita/MuditaOS/commit/${HASH}) | ${MEM} | ${RAM} | ${USAGE}|")
    
    else
        echo -e "${IETEM}---:(NO"
    fi

    I=$(( $I + 1 ))
done
sed -e "s#@WIKI_DATA_PATH@#${WIKI_DATA_PATH}#g;s#@PLOT_OUT@#${PLOT_OUT}#g" ${PLOT_SCRIPT} |gnuplot

sed -E 's#(<text><tspan font-family="arial" >)([0-9a-zA-Z]{40})(</tspan></text>)#<a xlink:href="https://github.com/mudita/MuditaOS/commit/\2" target="_blank">\1\2\3</a>#g' ${PLOT_OUT} > ${WIKI_PLOT_PATH}


cp ${WIKI_TEMPLATE_PATH} ${WIKI_PAGE_PATH}
echo -e "${WIKI_DATA}" >> ${WIKI_PAGE_PATH}
