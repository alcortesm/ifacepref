#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
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
    buf = (char *) calloc(IFNAMSIZ + 1, sizeof(char));
    if (!buf) {
        perror("calloc()");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        ssize_t nr;
        nr = read(fd, buf, IFNAMSIZ);
        if (nr == -1) {
            perror("read()");
            exit(EXIT_FAILURE);
        }
        buf[IFNAMSIZ] = '\0'; /* just in case there is no '\0' in the device */
        fprintf(stdout, "%s\n", buf);
        fflush(stdout);

        /* we want to keep reading at the beginning */
        off_t off = 0;
        off = lseek(fd, off, SEEK_SET);
        if (off == (off_t) -1) {
            perror("lseek()");
            exit(EXIT_FAILURE);
        }

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);

        int retval;
        retval = select(fd+1, &rfds, NULL, NULL, NULL);
        if (retval == -1) {
            perror("select()");
            exit(EXIT_FAILURE);
        }
    }
    
    return EXIT_SUCCESS;
}
