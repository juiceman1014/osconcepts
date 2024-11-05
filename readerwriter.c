#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 10
#define TRUE 1

int shared_array[SIZE];

pthread_mutex_t mutex;
pthread_mutex_t print_mutex;
pthread_cond_t cond_read;
pthread_cond_t cond_write;

int active_writers = 0;
int active_readers = 0;
int waiting_writers = 0;
int waiting_readers = 0;
int actions = 0;
int queue_number = 0;
int current_queue_number = 0;

void *writer(void *params);
void *reader(void *params);

int main(int argc, char *argv[]){

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&print_mutex, NULL);
	pthread_cond_init(&cond_read, NULL);
	pthread_cond_init(&cond_write, NULL);
	
	int readers = 15;
	int writers = 8;
	int total_threads = readers + writers;
	pthread_t workers[total_threads];

	srand(time(0));

	for(int i = 0; i < writers; i++){
		pthread_create(&workers[i], NULL, &writer, NULL);
	}

	for(int i = writers; i < total_threads; i++){
		pthread_create(&workers[i], NULL, &reader, NULL);
	}

	for(int i = 0; i < total_threads; i++){
		pthread_join(workers[i], NULL);
	}

	printf("Total actions: %d\n", actions);

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&print_mutex);
	pthread_cond_destroy(&cond_read);
	pthread_cond_destroy(&cond_write);

	return 0;
}

void *writer(void *params){

	while(TRUE){
		int sleep_time = rand() % 5;
	    sleep(sleep_time);

		pthread_mutex_lock(&mutex);

		if(actions >= 100){
			pthread_mutex_unlock(&mutex);
			break;
		}

		int my_queue_number = queue_number;
		queue_number++;

		while((active_writers + active_readers) > 0 || my_queue_number != current_queue_number){
			waiting_writers++;
			printf("Writer waiting...\n");
			pthread_cond_wait(&cond_write, &mutex);
			waiting_writers--;
		}
		active_writers++;
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&print_mutex);
		printf("\nQueue number %d's turn\n", current_queue_number);
		printf("My queue number is %d\n", my_queue_number);
		printf("%d active readers | ",active_readers);
		printf("%d active writers | ",active_writers);
		printf("%d waiting readers | ",waiting_readers);
		printf("%d waiting writers\n",waiting_writers);
		pthread_mutex_unlock(&print_mutex);
	
		for(int i = 0; i < 10; i++){
			if(actions >= 100){
				break;
			}
			int val = rand() % 100;
			shared_array[i] = val;
			actions++;
			printf("Action %d - ", actions);
			printf("Writer wrote: %d to index %d\n", val, i);
		}

		printf("\n");

		pthread_mutex_lock(&mutex);
		active_writers--;
		current_queue_number++;
		if(waiting_writers > 0){
			pthread_cond_signal(&cond_write);
		}else if(waiting_readers > 0){
			pthread_cond_broadcast(&cond_read);
		}

		pthread_mutex_unlock(&mutex);
	}

	pthread_exit(0);

}

void *reader(void *params){

	while(TRUE){
		int sleep_time = rand() % 5;
	    sleep(sleep_time);

		pthread_mutex_lock(&mutex);

		if(actions >= 100){
			pthread_mutex_unlock(&mutex);
			break;
		}

		while((active_writers + waiting_writers) > 0){
			waiting_readers++;
			printf("Reader waiting...\n");
			pthread_cond_wait(&cond_read, &mutex);
			waiting_readers--;
		}
		active_readers++;
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&print_mutex);
		printf("\n%d active readers | ",active_readers);
		printf("%d active writers | ",active_writers);
		printf("%d waiting readers | ",waiting_readers);
		printf("%d waiting writers\n",waiting_writers);
		pthread_mutex_unlock(&print_mutex);

		if(actions >= 100){
			break;
		}

		int val = rand() % 10;
		actions++;
		printf("Action %d - ", actions);
		printf("Reader read: %d from index %d\n\n", shared_array[val], val);
		
		pthread_mutex_lock(&mutex);
		active_readers--;
		if(active_readers == 0 && waiting_writers > 0){
			pthread_cond_signal(&cond_write);
		}

		pthread_mutex_unlock(&mutex);
	}

	pthread_exit(0);
	
}