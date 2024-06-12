#!/bin/sh

device=/dev/aesdchar

# load and unload aesdchar driver
sudo ./../aesd-char-driver/aesdchar_unload
sudo ./../aesd-char-driver/aesdchar_load

# write data to the circular buffer
echo "write1" >> ${device}
echo "write2" >> ${device}
echo "write3" >> ${device}
echo "write4" >> ${device}
echo "Contents of aesdchar:"
cat ${device}

# flush
./flush_test

echo "Contents of aesdchar post flush:"
cat ${device}

echo "Test Finished"
