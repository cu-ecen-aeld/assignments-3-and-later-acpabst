#!/bin/sh
# Finder
# This script searches for content within a provided directory
# Author: Andy Pabst
# Date 1/30/24

if [ $# != 2 ]
then
    echo "Missing arguments."
    exit 1
fi

path = $1
content = $2

if [ -d "$path" ]
then
    numFiles = grep -r ${conteent} ${path} -cH | wc -l
    numlines =  grep -r ${content} ${path} -oc
    echo "The number of files are ${numFiles} and the number of matching lines are ${numLines}"
else
    echo "Directory provided does not exist."
    exit 1
fi
