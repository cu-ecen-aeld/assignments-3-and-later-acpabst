#!/bin/sh

device=/dev/aesdchar

make modules
sudo ./aesdchar_unload
sudo ./aesdchar_load

echo -n "wr" > ${device}
echo -n "it" > ${device}
echo "e1"  > ${device}
echo "write2" > ${device}
echo "write3" > ${device}
echo "write4" > ${device}
echo "write5" > ${device}
echo "write6" > ${device}
echo "write7" > ${device}
echo "write8" > ${device}
echo "write9" > ${device}
echo -n "wr" > ${device}
echo -n "it" > ${device}
echo "e10" > ${device}
echo "write11" > ${device}
cat /dev/aesdchar

