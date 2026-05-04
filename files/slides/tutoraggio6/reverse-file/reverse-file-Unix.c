/******* write strings to a file in reverse order - POSIX version ********/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE 1024

static void Error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int fd;
    char input[BUFFER_SIZE];
    char *oldbuf = NULL;
    off_t oldsize;
    ssize_t n;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("file name is: %s - give me the input\n", argv[1]);

    fd = open(argv[1], O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) Error("open");

    while (1) {
        if (scanf("%1023s", input) != 1)
            break;

        if (strcmp(input, ".") == 0)
            break;

        size_t len = strlen(input);
        if (len + 1 >= BUFFER_SIZE) {
            fprintf(stderr, "Input too long\n");
            close(fd);
            return EXIT_FAILURE;
        }

        input[len++] = '\n';
        input[len] = '\0';

        oldsize = lseek(fd, 0, SEEK_END);
        if (oldsize < 0) Error("lseek SEEK_END");

        oldbuf = malloc(oldsize);
        if (oldsize > 0 && oldbuf == NULL) Error("malloc");

        if (lseek(fd, 0, SEEK_SET) < 0) Error("lseek SEEK_SET");

        if (oldsize > 0) {
            n = read(fd, oldbuf, oldsize);
            if (n < 0) Error("read");
            if (n != oldsize) {
                fprintf(stderr, "short read\n");
                free(oldbuf);
                close(fd);
                return EXIT_FAILURE;
            }
        }

        if (lseek(fd, 0, SEEK_SET) < 0) Error("lseek rewind");

        n = write(fd, input, len);
        if (n < 0 || (size_t)n != len) Error("write input");

        if (oldsize > 0) {
            n = write(fd, oldbuf, oldsize);
            if (n < 0 || n != oldsize) Error("write old content");
        }

        if (ftruncate(fd, len + oldsize) < 0) Error("ftruncate");

        free(oldbuf);
        oldbuf = NULL;
    }

    close(fd);
    return EXIT_SUCCESS;
}
