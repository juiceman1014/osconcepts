#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 10
int shared_array[SIZE];

int active_readers = 0;
int active_writers = 0;
int actions = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

void *writer(void *params);
void *reader(void *params);

int main(int argc, char *argv[]){
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	int readers = 3;
	int writers = 3;

	pthread_t workers[6];

	srand(time(0));

	for(int i = 0; i < 3; i++){
		pthread_create(&workers[i], NULL, &writer, NULL);
	}

	for(int i = 3; i < 6; i++){
		pthread_create(&workers[i], NULL, &reader, NULL);
	}

	for(int i = 0; i < 6; i++){
		pthread_join(workers[i], NULL);
	}

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	return 0;
}

void *writer(void *params){
	pthread_mutex_lock(&mutex);

	int sleep_time = rand() % 5;
	sleep(sleep_time);

	while(active_writers > 0 || active_readers > 0){
		printf("Writer waiting...\n");
		pthread_cond_wait(&cond, &mutex);
	}
	active_writers++;
	for(int i = 0; i < 10; i++){
		int val = rand() % 100;
		shared_array[i] = val;
		printf("Writer wrote %d to index %d\n", shared_array[i], i);
	}
	active_writers--;

	pthread_mutex_unlock(&mutex);
	pthread_cond_broadcast(&cond);
	
	pthread_exit(0);
}

void *reader(void *params){
	pthread_mutex_lock(&mutex);

	int sleep_time = rand() % 5;
	sleep(sleep_time);
	
	while(active_writers > 0){
		printf("Reader waiting...\n");
		pthread_cond_wait(&cond, &mutex);
	}

	active_readers++;
	int val = rand() % 10;
	printf("Reader read: %d from index %d\n", shared_array[val], val);
	active_readers--;

	pthread_mutex_unlock(&mutex);

	if(active_readers == 0){
		pthread_cond_broadcast(&cond);
	}
	
	pthread_exit(0);
}