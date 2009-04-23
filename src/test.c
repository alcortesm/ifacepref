#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define PATH "/dev/ifacepref"

int
main(int argc, char ** argv)
{
    /* test if device can be opened for read only */
    {
        int fd;
        fd = open(PATH, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "can't open %s as read only\n", PATH);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }


    /* test if device can be opened for write only */
    {
        int fd;
        fd = open(PATH, O_WRONLY);
        if (fd == -1) {
            fprintf(stderr, "can't open %s as write only\n", PATH);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }

    /* test if device can be opened for read and write */
    {
        int fd;
        fd = open(PATH, O_RDWR);
        if (fd == -1) {
            fprintf(stderr, "can't open %s as read/write\n", PATH);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }




    return EXIT_SUCCESS;
}
