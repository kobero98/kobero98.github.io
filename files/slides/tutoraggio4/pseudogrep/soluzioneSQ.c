#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#define MAX_WORD 256
#define MAX_COUNT 255

#include <pthread.h>

typedef struct {
    const char *filename;
    const char *target;
    int c;
} thread_data_t;

int count_occurrences(const char *filename, const char *target) {
    int fd = open(filename,O_RDONLY);
    if (fd<0) {
        perror("Errore apertura file");
        exit(1);
    }
    char c;
    int count = 0;
    int r= read(fd,&c,1);
    int row=0;
    while (r > 0){
        int i=0;
        while(c==target[i]){
                int r= read(fd,&c, 1);
                if(r<0) {
                    close(fd);
                    return count;
                }
		if(c =='\n') row++;
                i++;
                if(i>=strlen(target)){
                    	printf("Found in %s at row: %d\n",filename,row);
			count++;
                    	break;
                }
            }
        if(c != target[0]){
            r=read(fd,&c,1);
	    if(c =='\n') row++;
        }
    }
    close(fd);
    return count;
}

void *thread_func(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;

    int count = count_occurrences(data->filename, data->target);

    printf("File: %s -> Occorrenze: %d\n", data->filename, count);
	
    data->c=count;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <parola> <file1> [file2 ... fileN]\n", argv[0]);
        return 1;
    }

    const char *target = argv[1];
    int num_files = argc - 2;

    pthread_t *threads = malloc(num_files * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(num_files * sizeof(thread_data_t));

    if (!threads || !thread_data) {
        perror("malloc");
        return 1;
    }

    // Creazione thread
    for (int i = 0; i < num_files; i++) {
        thread_data[i].filename = argv[i + 2];
        thread_data[i].target = target;

        if (pthread_create(&threads[i], NULL, thread_func, &thread_data[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    // Join thread
    int tot_count=0;
    for (int i = 0; i < num_files; i++) {
        pthread_join(threads[i], NULL);
    	tot_count = tot_count + thread_data[i].c;
    }
	
    printf("la parola %s è stata trovata %d all'interno dei file",target,tot_count);
    free(threads);
    free(thread_data);

    return 0;
}

