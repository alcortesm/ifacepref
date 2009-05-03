#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <net/if.h>

#define PATH "/dev/ifacepref"

int
main(int argc, char ** argv)
{
    int fd;
    fd = open(PATH, O_RDONLY);
    if (fd == -1) {
        perror(PATH);
        exit(EXIT_FAILURE);
    }

    /* we don't know if the data stored in the device
       is going to have an ending '\0', so we allocate
       1 additional char in the buffer, just to be able
       to print the buffer as a string */
    char * buf;
    buf = calloc(IFNAMSIZ+1, sizeof(char));
    if (!buf) {
        perror("calloc()");
        exit(EXIT_FAILURE);
    }

    ssize_t nr;
    size_t count = IFNAMSIZ; /* max storage size of the device */
    nr = read(fd, (void *) buf, count);
    if (nr == -1) {
        perror("read()");
        exit(EXIT_FAILURE);
    }

    close(fd);
    /* just in case there were no '\0' stored in the device */
    buf[nr] = '\0';
    printf("%s\n", buf);
    exit(EXIT_SUCCESS);
}
