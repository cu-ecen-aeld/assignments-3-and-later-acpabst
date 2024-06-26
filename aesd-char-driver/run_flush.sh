#!/bin/sh

device=/dev/aesdchar

# load and unload aesdchar driver
/usr/bin/aesdchar_unload
/usr/bin/aesdchar_load

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
