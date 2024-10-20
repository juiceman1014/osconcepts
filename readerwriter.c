#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 10
#define TRUE 1

int shared_array[SIZE];

pthread_mutex_t mutex;
pthread_mutex_t actions_mutex;
pthread_cond_t cond;

int active_writers = 0;
int active_readers = 0;
int waiting_writers = 0;
int actions = 0;
int priority = 0;
int current_active_priority = 1;

void *writer(void *params);
void *reader(void *params);

int main(int argc, char *argv[]){
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&actions_mutex, NULL);

	pthread_t workers[15];

	srand(time(0));

	for(int i = 0; i < 2; i++){
		pthread_create(&workers[i], NULL, &writer, NULL);
	}

	for(int i = 2; i < 15; i++){
		pthread_create(&workers[i], NULL, &reader, NULL);
	}

	for(int i = 0; i < 15; i++){
		pthread_join(workers[i], NULL);
	}

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&actions_mutex);
	pthread_cond_destroy(&cond);

	return 0;
}

void *writer(void *params){

	while(TRUE){
		int sleep_time = rand() % 5;
		sleep(sleep_time);

		pthread_mutex_lock(&actions_mutex);
		if(actions>=100){
			pthread_mutex_unlock(&actions_mutex);
			break;
		}

		pthread_mutex_lock(&mutex);
		priority++;
		int writer_priority = priority;
		waiting_writers++;
		while(active_readers > 0 || active_writers > 0 || writer_priority != current_active_priority){
			pthread_cond_wait(&cond, &mutex);
			printf("Writer waiting...\n");
			printf("%d active readers %d active writers %d waiting writers\n", active_readers, active_writers, waiting_writers);
		}
		active_writers++;
		pthread_mutex_unlock(&mutex);

		printf("\n");
		printf("It's P%d's turn\n", priority);
		pthread_mutex_lock(&mutex);
		waiting_writers--;
		pthread_mutex_unlock(&mutex);
		for(int i = 0; i < 10; i++){
			int val = rand() % 100;
			shared_array[i] = val;
			printf("%d active readers %d active writers %d waiting writers\n", active_readers, active_writers, waiting_writers);
			printf("P%d Writer wrote: %d to index %d.\n", writer_priority, val, i);
			sleep(1);
			actions++;
			printf("%d actions\n", actions);
			if(actions == 100){
				break;
			}
		}
		printf("\n");
		pthread_mutex_unlock(&actions_mutex);

		pthread_mutex_lock(&mutex);
		active_writers--;
		current_active_priority++;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);

	}

	pthread_exit(0);
}

void *reader(void *params){

	while(TRUE){

		int sleep_time = rand() % 5;
		sleep(sleep_time);

		pthread_mutex_lock(&actions_mutex);
		if(actions>=100){
			pthread_mutex_unlock(&actions_mutex);
			break;
		}	

		pthread_mutex_lock(&mutex);
		if(waiting_writers > 0){
			pthread_cond_wait(&cond, &mutex);
			printf("Reader waiting...\n");
		}
		while(active_writers > 0){
			pthread_cond_wait(&cond, &mutex);
			printf("Reader waiting...\n");
		}
		active_readers++;
		pthread_mutex_unlock(&mutex);

		printf("\n");
		int val = rand() % 10;
		printf("%d active readers %d active writers %d waiting writers\n", active_readers, active_writers, waiting_writers);
		printf("Reader read: %d from index %d.\n", shared_array[val], val);
		sleep(1);

		actions++;
		printf("%d actions\n\n", actions);
		pthread_mutex_unlock(&actions_mutex);

		pthread_mutex_lock(&mutex);
		active_readers--;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);

	}

	pthread_exit(0);
}