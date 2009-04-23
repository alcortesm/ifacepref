#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

extern int errno;

#define PATH "/dev/ifacepref"
#define BUFFSZ 20
char zbuff[BUFFSZ] = {'\0', '\0', '\0', '\0', '\0',
 '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
 '\0', '\0', '\0', '\0', '\0', '\0'};

int
main(int argc, char ** argv)
{
    /* test for some open calls and close */
    /* test if device can be opened for read only */
    {
        int fd;
        fd = open(PATH, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "[000] can't open %s as read only\n", PATH);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    /* test if device can be opened for write only */
    {
        int fd;
        fd = open(PATH, O_WRONLY);
        if (fd == -1) {
            fprintf(stderr, "[001] can't open %s as write only\n", PATH);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    /* test if device can be opened for read and write */
    {
        int fd;
        fd = open(PATH, O_RDWR);
        if (fd == -1) {
            fprintf(stderr, "[002] can't open %s as read/write\n", PATH);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }


    /* test for zero size read */
    {
        int fd;
        fd = open(PATH, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "[003] can't open %s as read only\n", PATH);
            exit(EXIT_FAILURE);
        }
        
        ssize_t nr;
        char buff[BUFFSZ];
        size_t count;

        bzero(buff, BUFFSZ);
        count=0;

        /* zero read */
        nr = read(fd, buff, count);
        if (nr != 0) {
            if (nr == -1) {
                perror("[004] read(count=0)");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "[005] a zero read didn't return 0\n");
            exit(EXIT_FAILURE);
        }
        if (memcmp(buff, zbuff, BUFFSZ)) {
            fprintf(stderr, "[006] user space buffer was modified!\n");
            exit(EXIT_FAILURE);
        }
        
        close(fd);
    }

    /* test for small read on empty device */
    {
        int fd;
        fd = open(PATH, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "[007] can't open %s as read only\n", PATH);
            exit(EXIT_FAILURE);
        }
        
        ssize_t nr;
        char buff[BUFFSZ];
        size_t count;

        bzero(buff, BUFFSZ);
        count=1;

        nr = read(fd, buff, count);
        if (nr == -1) {
            perror("[008] read(count=0)");
            exit(EXIT_FAILURE);
        }
        if (nr != 1) {
            fprintf(stderr, "[009] a read(1) on empty device return %d instead of 1\n", nr);
            exit(EXIT_FAILURE);
        }
        if (memcmp(buff, zbuff, BUFFSZ)) {
            fprintf(stderr, "[010] user space buffer was modified!\n");
            exit(EXIT_FAILURE);
        }
        
        close(fd);
    }

    /* test for big read on empty device */
    {
        int fd;
        fd = open(PATH, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "[011] can't open %s as read only\n", PATH);
            exit(EXIT_FAILURE);
        }
        
        ssize_t nr;
        char buff[BUFFSZ];
        size_t count;

        bzero(buff, BUFFSZ);
        count=5;

        nr = read(fd, buff, count);
        if (nr == -1) {
            perror("[012] read(count=0)");
            exit(EXIT_FAILURE);
        }
        if (nr != 1) {
            fprintf(stderr, "[013] a read(5) on empty device return %d instead of 1\n", nr);
            exit(EXIT_FAILURE);
        }
        if (memcmp(buff, zbuff, BUFFSZ)) {
            fprintf(stderr, "[014] user space buffer was modified!\n");
            exit(EXIT_FAILURE);
        }
        
        close(fd);
    }

    /* test for big read on empty device */
    {
        int fd;
        fd = open(PATH, O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "[015] can't open %s as read only\n", PATH);
            exit(EXIT_FAILURE);
        }
        
        ssize_t nr;
        char buff[BUFFSZ];
        size_t count;

        bzero(buff, BUFFSZ);
        count=20;

        nr = read(fd, buff, count);
        if (nr == -1) {
            perror("[016] read(count=0)");
            exit(EXIT_FAILURE);
        }
        if (nr != 1) {
            fprintf(stderr, "[017] a read(20) on empty device return %d instead of 1\n", nr);
            exit(EXIT_FAILURE);
        }
        if (memcmp(buff, zbuff, BUFFSZ)) {
            fprintf(stderr, "[018] user space buffer was modified!\n");
            exit(EXIT_FAILURE);
        }
        
        close(fd);
    }

    /* test for read on empty device for WRONLY */
    {
        int fd;
        fd = open(PATH, O_WRONLY);
        if (fd == -1) {
            fprintf(stderr, "[019] can't open %s as write only\n", PATH);
            exit(EXIT_FAILURE);
        }
        
        ssize_t nr;
        char buff[BUFFSZ];
        size_t count;

        bzero(buff, BUFFSZ);
        count=5;

        nr = read(fd, buff, count);
        if (nr != -1) {
            fprintf(stderr,"[020] read on write only device succed!");
            exit(EXIT_FAILURE);
        }
        if (errno != EBADF) {
            fprintf(stderr, "[021] read() on write-only device set errno to %d instead of %d (EBADF)\n", errno, EBADF);
            exit(EXIT_FAILURE);
        }
        close(fd);
    }


    return EXIT_SUCCESS;
}
