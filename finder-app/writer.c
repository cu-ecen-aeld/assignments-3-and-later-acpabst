/* Writer
 *
 * Author: Andy Pabst
 * Date: 2/2/24 */

#include <dirent.h>
#include <errno.h>

int main (int argc, char*argv[]) {
    if (argc != 2) {
        // TODO syslog error message
	return 1;
    }

    // TODO does this work with the full pathname?
    DIR* dir = opendir("mydir");
    if (dir) {
        /* Directory exists. */
        closedir(dir);
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
    } else {
        /* opendir() failed for some other reason. */
    }    

}
