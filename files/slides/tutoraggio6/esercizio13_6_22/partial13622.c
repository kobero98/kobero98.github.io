#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 4096
#define PATH_MAX 2048
static volatile sig_atomic_t stop = 0;

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


static ssize_t write_all(int fd, const void *buf, size_t count) {
    const char *p = (const char *)buf;
    size_t left = count;

    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += n;
        left -= (size_t)n;
    }
    return (ssize_t)count;
}

int main(int argc, char *argv[]) {
    int fd_f;
    pid_t pid;
    char shadow_name[PATH_MAX];
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_F>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (snprintf(shadow_name, sizeof(shadow_name), "shadow_%s", argv[1]) >= (int)sizeof(shadow_name)) {
        fprintf(stderr, "Nome file troppo lungo\n");
        exit(EXIT_FAILURE);
    }


    fd_f = open(argv[1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd_f < 0) die("open F");

    pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {
        int fd_src, fd_shadow;
        off_t copied = 0;

        fd_src = open(argv[1], O_RDONLY);
        if (fd_src < 0) die("child open F");

        fd_shadow = open(shadow_name, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (fd_shadow < 0) die("child open shadow_F");

        while (!stop) {
            off_t end = lseek(fd_src, 0, SEEK_END);
            if (end < 0) die("child lseek SEEK_END");

            if (end > copied) {
                off_t delta = end - copied;

                if (lseek(fd_src, copied, SEEK_SET) < 0)
                    die("child lseek SEEK_SET");

                while (delta > 0) {
                    ssize_t chunk = (delta > BUF_SIZE) ? BUF_SIZE : (ssize_t)delta;
                    ssize_t nr = read(fd_src, buf, chunk);

                    if (nr < 0) {
                        if (errno == EINTR) continue;
                        die("child read");
                    }
                    if (nr == 0) break;

                    if (write_all(fd_shadow, buf, (size_t)nr) < 0)
                        die("child write shadow");

                    copied += nr;
                    delta -= nr;
                }
            }

            sleep(10);
        	
	    printf("reupero la copia\n");
	}

        close(fd_src);
        close(fd_shadow);
        _exit(0);
    }

    while (!stop) {
        ssize_t nr = read(STDIN_FILENO, buf, sizeof(buf));
        if (nr < 0) {
            if (errno == EINTR) continue;
            die("read stdin");
        }
        if (nr == 0) break;

        if (write_all(fd_f, buf, (size_t)nr) < 0)
            die("write F");
    }

    close(fd_f);
    return 0;
}
