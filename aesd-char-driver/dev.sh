#!/bin/sh
make modules
sudo ./aesdchar_unload
sudo ./aesdchar_load
echo "test" > /dev/aesdchar
cat /dev/aesdchar

