#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

/*
 *  Times with N threads:
 *
 *  2 : 1,232s
 *  4 : 0,502s
 *  6 : 0,377s
 *  8 : 0,307s
 *  10: 0,277s
 *  12: 0,255s
 *  14: 0,259s
 *  16: 0,253s
 *  18: 0,253s
 *  20: 0,251s
 *  26: 0,251s
 *
 *
 *
 */

#define ROW_SIZE    257 // include terminator
/* #define ROW_SIZE    11 // include terminator */
#define A_FILE      "input_c2.txt"
#define B_FILE      "res_c2.txt"
#define OUTPUT_BUFFER_SIZE 32

/* #define THREAD_NUM  4 */

unsigned long row_number;
unsigned long* results;
char* full_data;  

typedef struct {
    unsigned long start;
    size_t size;
    unsigned long index_start;
    size_t index_size;
} thread_data;

void *count_H_thread(void *arg) {
    thread_data *task = (thread_data *)arg;
    int c = 0;
    for (int i = task->start; i < task->start + task->size; i+=ROW_SIZE) {
        for (int j = 0; j < ROW_SIZE; j++){
            if (full_data[i+j] == 'H') {
                results[task->index_start + c]++;
            }
        }
        c++;
    }
    return NULL;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        printf("usage: progran <thread_num>\n");
        return 1;
    }
    
    unsigned long THREAD_NUM = atoi(argv[1]);

    int fd_in = open(A_FILE, O_RDONLY);
    if (fd_in < 0) {
        perror("Error opening input file");
        return 1;
    }

    thread_data *tasks = malloc(sizeof(thread_data) * THREAD_NUM);
    pthread_t *threads = malloc(sizeof(pthread_t) * THREAD_NUM);
    if (!tasks || !threads) {
        perror("Allocation failed");
        close(fd_in);
        return 1;
    }

    size_t size = lseek(fd_in, 0, SEEK_END);
    lseek(fd_in, 0, SEEK_SET);
    
    int line_count = 0;
    full_data = malloc(size*sizeof(char));
    row_number = size / ROW_SIZE;
    printf("File size: %lu %lu %d\n", size, row_number, ROW_SIZE);

    results = calloc(row_number, sizeof(unsigned long));
    ssize_t bytes_read;

    // read everything all at once
    if (read(fd_in, full_data, size) != size) {
        perror("Reading file");
        return 1;
    }

    unsigned long row_per_thread = row_number / (THREAD_NUM-1);
    size_t size_per_thread = row_per_thread*ROW_SIZE;

    printf("Creating threads...\n");
    for (int i = 0; i < THREAD_NUM; i++) {
        tasks[i].index_start = i*row_per_thread;
        if (i == THREAD_NUM-1)
            tasks[i].index_size = row_number % (THREAD_NUM-1); // handle remaining rows
        else
            tasks[i].index_size = row_per_thread;

        tasks[i].start = i*size_per_thread;
        if (i == THREAD_NUM-1)
            tasks[i].size = size - (tasks[i].start + tasks[i].size); // handle remaining rows
        else
            tasks[i].size = size_per_thread;


        /* printf("Creating thread %d: %ld %ld %lx %lx\n",  i, tasks[i].start, tasks[i].size, tasks[i].index_start,tasks[i].index_size); */
        if (pthread_create(&threads[i], NULL, count_H_thread, &tasks[i]) != 0) {
            perror("Failed to create thread");
            break;
        }

    }


    close(fd_in);

    int fd_out = open(B_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
        perror("Error opening output file");
        free(tasks);
        free(threads);
        return 1;
    }


    // Join all threads
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d done!\n", i);
        char output[OUTPUT_BUFFER_SIZE];
        for (int j = tasks[i].index_start; j < tasks[i].index_start + tasks[i].index_size; j++) {
            int len = snprintf(output, sizeof(output), "%ld\n", results[j]);
            write(fd_out, output, len);
        }
    }

    close(fd_out);
    free(tasks);
    free(threads);
    free(full_data);
    free(results);


    printf("Processed %d lines with ROW_SIZE = %d. Output written to %s\n", line_count, ROW_SIZE, B_FILE);
    return 0;
}

