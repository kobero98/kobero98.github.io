#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    int id;
    int num_cpus;
} thread_arg_t;
static inline uint64_t rdtsc_start() {
    unsigned int lo, hi;
    __asm__ __volatile__ (
        "lfence\n\t"
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi)
    );
    return ((uint64_t)hi << 32) | lo;
}

static inline uint64_t rdtsc_end() {
    unsigned int lo, hi;
    __asm__ __volatile__ (
        "rdtscp\n\t"
        "lfence\n\t"
        : "=a"(lo), "=d"(hi)
        :
        : "%rcx"
    );
    return ((uint64_t)hi << 32) | lo;
}

void *thread_func(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int num_cpus = targ->num_cpus;
    int *visited = calloc(num_cpus, sizeof(int));
    int visited_count = 0;
    unsigned int seed = time(NULL) ^ (targ->id * 1234567);
    uint64_t start = rdtsc_start();
    while (visited_count < num_cpus) {

	int p = sched_getcpu();    
        int cpu = rand_r(&seed) % num_cpus;
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);
        if (sched_setaffinity(0, sizeof(set), &set) != 0) {
            perror("sched_setaffinity");
        }
        //printf("Thread %d su CPU %d\n", targ->id, cpu);
        //printf("stavo su %d, ho estratto %d sto girando su %d\n",p,cpu,sched_getcpu());
	if (!visited[cpu]) {
            visited[cpu] = 1;
            visited_count++;
        }
        usleep(10000);
    }
    uint64_t time = rdtsc_end()-start;
    printf("Thread %d ha visitato tutte le CPU in %ld\n", targ->id,time);
    free(visited);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <num_thread>\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    printf("CPU disponibili: %d\n", num_cpus);

    pthread_t *threads = malloc(sizeof(pthread_t) * N);
    thread_arg_t *args = malloc(sizeof(thread_arg_t) * N);

    for (int i = 0; i < N; i++) {
        args[i].id = i;
        args[i].num_cpus = num_cpus;
        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Tutti i thread hanno completato.\n");

    free(threads);
    free(args);
    return 0;
}

