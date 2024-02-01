#!/bin/sh
# Writer
# This script writes contect to a file
# Author: Andy Pabst
# Date: 1/30/24

if ! [ $# -eq 2 ]
then
    echo "Missing arguments."
    exit 1
fi

path=$1
content=$2

if [ -d "$(dirname $path)" ]
then
    touch $path
else
    mkdir -p "$(dirname $path)" && touch $path
fi

if [ -f "$path" ]
then
    echo $content > $path
else
    echo "File could not be created."
    exit 1
fi
