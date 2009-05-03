#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <net/if.h>
#include <string.h>

#define PATH "/dev/ifacepref"

int
main(int argc, char ** argv)
{
    size_t count;
    switch (argc) {
        case 1:
            count = 1; /* the empty string size is 1 */
            break;
        case 2:
            count = strlen(argv[1]) + 1; /* also copy the ending '\0' */
            break;
        default:
            fprintf(stderr, "Use no arguments to clear ifacepref\n");
            fprintf(stderr, "or 1 arg to store it in ifacepref\n");
            exit(EXIT_FAILURE);
    }

    int fd;
    fd = open(PATH, O_WRONLY);
    if (fd == -1) {
        perror(PATH);
        exit(EXIT_FAILURE);
    }

    ssize_t nw;
    nw = write(fd, (const void *) argv[1], count);
    if (nw == -1) {
        perror("write()");
        exit(EXIT_FAILURE);
    }

    close(fd);
    exit(EXIT_SUCCESS);
}
