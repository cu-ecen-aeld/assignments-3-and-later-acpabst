#!/bin/sh
make modules
sudo ./aesdchar_unload
sudo ./aesdchar_load
echo "test1" > /dev/aesdchar
echo "test2" > /dev/aesdchar
cat /dev/aesdchar

