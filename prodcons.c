#include "buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define TRUE 1

buffer_item buffer[BUFFER_SIZE];

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

int insert_pointer = 0;
int remove_pointer = 0;

void *producer(void *param);
void *consumer(void *param);

int insert_item(buffer_item item);
int remove_item(buffer_item *item);


int main(int argc, char *argv[]){

	if(argc != 4){
		fprintf(stderr, "Useage: <sleep time> <producer threads> <consumer threads>\n");
	}

	//1. Get command line arguments
	int sleep_time = atoi(argv[1]);
	int num_producer = atoi(argv[2]);
	int num_consumer = atoi(argv[3]);

	//2. Initialize buffer
	printf("%d\n", pthread_mutex_init(&mutex,NULL));
	printf("%d\n", sem_init(&empty,0,5));
	printf("%d\n", sem_init(&full,0,0));

	int num = 100;
	int num2 = 200;
	sem_getvalue(&empty, &num);
	printf("Empty semaphore is %d\n", num);
	sem_getvalue(&full, &num2);
	printf("Full semaphore is %d\n", num2);

	srand(time(0));

	//3. Create producer threads
	for(int i = 0; i < num_producer; i++){
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid,&attr,producer,NULL);
	}

	//4. Create consumer threads
	for(int j = 0; j < num_consumer; j++){
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid,&attr,consumer,NULL);
	}

	//5. Sleep
	sleep(sleep_time);

	//6. Exit
	return 0;
}

void *producer(void *param){
	buffer_item item;
	int r;
	while(TRUE){
		r = rand() % 5;
		sleep(r);
		item = rand() % 100;
		if(insert_item(item) == -1){
			fprintf(stderr, "Error Producing");
		}
		printf("Producer produced %d\n", item);
	}	
}

void *consumer(void *param){
	buffer_item item;
	int r;
	while(TRUE){
		r = rand() % 5;
		sleep(r);
		if(remove_item(&item) == -1){
			fprintf(stderr, "Error Consuming");
		}
		printf("Consumer consumed %d\n", item);
	}
}

int insert_item(buffer_item item){
	sem_wait(&empty);
	pthread_mutex_lock(&mutex);
	buffer[insert_pointer++] = item;
	insert_pointer = insert_pointer % 5;
	pthread_mutex_unlock(&mutex);
	sem_post(&full);
	return 0;
}

int remove_item(buffer_item *item){
	sem_wait(&full);
	pthread_mutex_lock(&mutex);
	*item = buffer[remove_pointer++];
	remove_pointer = remove_pointer % 5;
	pthread_mutex_unlock(&mutex);
	sem_post(&empty);
	return 0;
}
