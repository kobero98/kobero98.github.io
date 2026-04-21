#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define N 10  
#define TOTAL_GENERATED 10 

#define APPLE 1
#define PEAR 2
#define ORANGE 3
#define STOP 4

int shared_fruit = 0;

typedef struct {
	int type; 
	int collected;
	int id;
} picker;

void type_to_string(int type, char** type_name) {
	printf("type: %d\n",type);
	switch (type) {
		case APPLE: *type_name = "Apples"; break;
		case PEAR: *type_name = "Pears"; break;
		case ORANGE: *type_name = "Oranges"; break;
		default: *type_name = "???"; break;
	}

}
void* collect(void* arg) {
	picker* p = (picker*)arg;
	while (1) {

		/* printf("Fruit check %d - %d\n", shared_fruit, p->type); */
		if (shared_fruit == STOP) break;
		if (shared_fruit == p->type) {
			p->collected++;
			shared_fruit = 0; 
		}
		usleep(100); 
	}
	return NULL;
}

int main() {
	srand(time(NULL));

	pthread_t threads[N];
	picker* pickers = malloc(N * sizeof(picker));
	int generated;

	// create threads
	for (int i = 0; i < N; i++) {
		pickers[i].type = (i % 3) + 1; 
		pickers[i].collected = 0;
		pickers[i].id = i +1;
		pthread_create(&threads[i], NULL, collect, &pickers[i]);
	}


	// generate loop
	for (int i = 0; i < TOTAL_GENERATED; i++) {
		char* type_name;
		while (shared_fruit != 0)
			usleep(5000);
		generated = (rand() % 3) + 1;
		type_to_string(pickers[generated].type, &type_name);
		printf("%d) About to generate %s - %d\n",i, type_name, generated);
		shared_fruit = generated; 
		usleep(50000); 
	}

	// send stop signal
	shared_fruit = STOP;

	// wait for threads
	for (int i = 0; i < N; i++) {
		pthread_join(threads[i], NULL);
	}


	// print results
	printf("\nHarvest results:\n");
	for (int i = 0; i < N; i++) {
		char* type_name;
		type_to_string(pickers[i].type, &type_name);
		printf("Picker %d (%s - %d): %d\n", pickers[i].id, type_name, pickers[i].type, pickers[i].collected);
	}

	free(pickers);
	return 0;
}

