#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "../aesd-char-driver/aesd_ioctl.h"

#define OUTPUT_FILE "/dev/aesdchar"

int main (int argc, char*argv[]) {

    int output_file = open(OUTPUT_FILE, O_WRONLY, 0666);
    if (output_file < 0) {
        printf("Could not open file. Error: %d", errno);
        close(output_file);
        return -1;
    }

    int res = ioctl(output_file, AESDCHAR_FLUSH);
    if (res) {    
        printf("Error invoking ioctl for %s\n", OUTPUT_FILE);
	printf("Result: %i, Errno: %d\n", res, errno):
        close(output_file);
        return -1;
    }
    return 0;
}


