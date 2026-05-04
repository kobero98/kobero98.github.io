#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    int id;
    int num_cpus;
} thread_arg_t;

// ---- RDTSC ----
static __inline uint64_t rdtsc_start() {
    int info[4];
    __cpuid(info, 0);
    return __rdtsc();
}

static __inline uint64_t rdtsc_end() {
    unsigned int aux;
    uint64_t t = __rdtscp(&aux);
    int info[4];
    __cpuid(info, 0);
    return t;
}
// ----------------

DWORD WINAPI thread_func(LPVOID arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int num_cpus = targ->num_cpus;

    int *visited = (int*)calloc(num_cpus, sizeof(int));
    int visited_count = 0;

    unsigned int seed = (unsigned int)time(NULL) ^ (targ->id * 1234567);

    uint64_t start = rdtsc_start();

    while (visited_count < num_cpus) {
        int cpu = rand_r(&seed) % num_cpus;

        DWORD_PTR mask = ((DWORD_PTR)1 << cpu);

        // imposta affinità del thread
        SetThreadAffinityMask(GetCurrentThread(), mask);

        // verifica CPU reale
        int actual_cpu = GetCurrentProcessorNumber();

        printf("Thread %d richiesto CPU %d, eseguito su CPU %d\n",
               targ->id, cpu, actual_cpu);

        if (!visited[cpu]) {
            visited[cpu] = 1;
            visited_count++;
        }

        Sleep(10); // 10 ms
    }

    uint64_t end = rdtsc_end();

    printf("Thread %d finito in %llu cicli CPU\n",
           targ->id, (unsigned long long)(end - start));

    free(visited);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <num_thread>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int num_cpus = sysinfo.dwNumberOfProcessors;

    printf("CPU disponibili: %d\n", num_cpus);

    HANDLE *threads = (HANDLE*)malloc(sizeof(HANDLE) * N);
    thread_arg_t *args = (thread_arg_t*)malloc(sizeof(thread_arg_t) * N);

    for (int i = 0; i < N; i++) {
        args[i].id = i;
        args[i].num_cpus = num_cpus;

        threads[i] = CreateThread(
            NULL,
            0,
            thread_func,
            &args[i],
            0,
            NULL
        );
    }

    WaitForMultipleObjects(N, threads, TRUE, INFINITE);

    for (int i = 0; i < N; i++) {
        CloseHandle(threads[i]);
    }

    free(threads);
    free(args);

    return 0;
}

