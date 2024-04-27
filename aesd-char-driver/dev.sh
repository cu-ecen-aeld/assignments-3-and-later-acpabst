#!/bin/sh

device=/dev/aesdchar
read_file=test
make modules
sudo ./aesdchar_unload
sudo ./aesdchar_load

read_with_seek()
{
        local seek=$1
        local device=$2
        local read_file=$3
        dd if=${device} skip=${seek} of=${read_file} bs=1 > /dev/null 2>&1
}

echo "write1" > ${device}
echo "write2" > ${device}
echo "write3" > ${device}
echo "write4" > ${device}
echo "write5" > ${device}
echo "write6" > ${device}
echo "write7" > ${device}
echo "write8" > ${device}
echo "write9" > ${device}
echo "write10" > ${device}

read_with_seek 2 ${device} ${read_file}
cat ${read_file}
#rm ${read_file}

#./../server/aesdsocket

