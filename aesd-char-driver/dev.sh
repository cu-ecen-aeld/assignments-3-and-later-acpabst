#!/bin/sh

device=/dev/aesdchar

make modules
sudo ./aesdchar_unload
sudo ./aesdchar_load

./../server/aesdsocket

