#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define BUF_SIZE (1 << 20)  // 1 MiB per thread

typedef struct {
    int src_fd;
    int dst_fd;
    off_t start;
    off_t end;
    int thread_id;
    int result;
} thread_arg_t;

static void *copy_chunk(void *arg_void) {
    thread_arg_t *arg = (thread_arg_t *)arg_void;
    char *buf = malloc(BUF_SIZE);
    if (!buf) {
        arg->result = ENOMEM;
        return NULL;
    }

    off_t offset = arg->start;
    while (offset < arg->end) {
        size_t to_read = BUF_SIZE;
        off_t remaining = arg->end - offset;
        if ((off_t)to_read > remaining)
            to_read = (size_t)remaining;

        ssize_t r = pread(arg->src_fd, buf, to_read, offset);
        if (r < 0) {
            arg->result = errno;
            free(buf);
            return NULL;
        }
        if (r == 0) {
            break;
        }

        ssize_t written_total = 0;
        while (written_total < r) {
            ssize_t w = pwrite(arg->dst_fd,
                               buf + written_total,
                               (size_t)(r - written_total),
                               offset + written_total);
            if (w < 0) {
                arg->result = errno;
                free(buf);
                return NULL;
            }
            written_total += w;
        }

        offset += r;
    }

    arg->result = 0;
    free(buf);
    return NULL;
}

static int parse_threads(const char *s) {
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (!s || *s == '\0' || *end != '\0' || v <= 0 || v > 1024)
        return -1;
    return (int)v;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <src> <dst> <num_threads>\n", argv[0]);
        return 1;
    }

    const char *src_path = argv[1];
    const char *dst_path = argv[2];
    int num_threads = parse_threads(argv[3]);
    if (num_threads <= 0) {
        fprintf(stderr, "num_threads non valido\n");
        return 1;
    }

    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) {
        perror("open src");
        return 1;
    }

    struct stat st;
    if (fstat(src_fd, &st) < 0) {
        perror("fstat src");
        close(src_fd);
        return 1;
    }

    off_t file_size = st.st_size;

    int dst_fd = open(dst_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (dst_fd < 0) {
        perror("open dst");
        close(src_fd);
        return 1;
    }

    if (file_size > 0) {
        int rc = posix_fallocate(dst_fd, 0, file_size);
        if (rc != 0) {
            errno = rc;
            perror("posix_fallocate");
            close(src_fd);
            close(dst_fd);
            return 1;
        }
    }

    if (num_threads > 1 && file_size < num_threads)
        num_threads = (int)file_size > 0 ? (int)file_size : 1;

    pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
    thread_arg_t *args = calloc(num_threads, sizeof(thread_arg_t));
    if (!threads || !args) {
        perror("calloc");
        close(src_fd);
        close(dst_fd);
        free(threads);
        free(args);
        return 1;
    }

    off_t chunk = file_size / num_threads;
    off_t rem = file_size % num_threads;

    off_t current = 0;
    for (int i = 0; i < num_threads; i++) {
        off_t len = chunk + (i < rem ? 1 : 0);
        args[i].src_fd = src_fd;
        args[i].dst_fd = dst_fd;
        args[i].start = current;
        args[i].end = current + len;
        args[i].thread_id = i;
        args[i].result = 0;
        current += len;

        int rc = pthread_create(&threads[i], NULL, copy_chunk, &args[i]);
        if (rc != 0) {
            errno = rc;
            perror("pthread_create");
            num_threads = i;
            break;
        }
    }

    int ret = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        if (args[i].result != 0) {
            errno = args[i].result;
            perror("thread copy");
            ret = 1;
        }
    }

    if (fsync(dst_fd) < 0) {
        perror("fsync");
        ret = 1;
    }

    free(threads);
    free(args);
    close(src_fd);
    close(dst_fd);
    return ret;
}
