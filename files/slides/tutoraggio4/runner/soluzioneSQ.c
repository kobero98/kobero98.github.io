#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define N 300 
int winner = 0;

char* ranking[N];
int rank_index = 0;

typedef struct {
    int id;
    char name[20];
} Runner;

void* runner(void* arg) {
    Runner* r = (Runner*)arg;
    int wait_time = rand() % 5 + 1; 
    sleep(wait_time);
    
    if (winner == 0) {
        winner = 1;
        printf("%s ha vinto la gara!\n", r->name);
    }
    
    ranking[rank_index++] = r->name;
    
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t* threads = malloc(N * sizeof(pthread_t));
    Runner* runners = malloc(N * sizeof(Runner));
    
    for (int i = 0; i < N; i++) {
        runners[i].id = i + 1;
        snprintf(runners[i].name, 20, "thread_%d", i + 1);
        pthread_create(&threads[i], NULL, runner, &runners[i]);
    }
    
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("\nClassifica finale:\n");
    for (int i = 0; i < N; i++) {
        if (ranking[i] != NULL) {
            printf("%d. %s\n", i + 1, ranking[i]);
        }
    }
    
    free(threads);
    free(runners);
    
    return 0;
}
