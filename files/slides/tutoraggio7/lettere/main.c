#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <strings.h>
#include <unistd.h>

#define NUM_WRITERS 5
#define MAX_LINE_LEN 100

// Structure to pass information to each writer thread
typedef struct {
    char id;
    FILE *file;
    char to_write;
} WriterArgs;

void *writer_thread(void *arg) {
    char id = ((WriterArgs *)arg)->id;
    FILE *file = ((WriterArgs *)arg)->file;
    char* to_write = &(((WriterArgs *)arg)->to_write);

    while(1) {
        /* printf("%d: Waiting...\n", id); */
        while (*to_write == 0) usleep(1000);
        /* printf("%d: Unlocked %c...\n", id, *to_write); */
        
        fputc(*to_write, file);
        fputc('\n', file);
        *to_write = 0;
    }

    pthread_exit(NULL);
}

int main() {
    FILE *input = fopen("A.txt", "r");
    if (!input) {
        perror("Error opening A.txt");
        return EXIT_FAILURE;
    }

    FILE *output[NUM_WRITERS];
    pthread_t writers[NUM_WRITERS];
    WriterArgs* args = calloc(NUM_WRITERS, sizeof(WriterArgs));
    char filename[10];

    // Open all output files B0.txt - B4.txt and create writer threads
    for (int i = 0; i < NUM_WRITERS; i++) {
        snprintf(filename, sizeof(filename), "B%d.txt", i);
        output[i] = fopen(filename, "w");
        if (!output[i]) {
            perror("Error opening output files");
            return EXIT_FAILURE;
        }
        args[i].id = i;
        args[i].file = output[i];
        pthread_create(&writers[i], NULL, writer_thread, &args[i]);
    }

    // Main thread reads input file line by line and dispatches characters to output files
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), input)) {
        int index;
        char letter;

        if (sscanf(line, "%d-%c", &index, &letter) == 2) {
            if (index >= 0 && index < NUM_WRITERS) {
                /* printf("Waiting on %d: %c\n", index, letter); */
                while (args[index].to_write != 0) usleep(1000);
                /* printf("Writing on %d: %c\n", index, letter); */
                args[index].to_write = letter;
            } else {
                fprintf(stderr, "Index out of range: %d\n", index);
            }
        } else {
            fprintf(stderr, "Invalid line format: %s", line);
        }
    }

    // Clean up: close files and wait for writer threads to terminate
    fclose(input);
    for (int i = 0; i < NUM_WRITERS; i++) {
        fclose(output[i]);
        pthread_cancel(writers[i]);
    }

    return 0;
}


