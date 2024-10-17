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
	if(pthread_mutex_init(&mutex,NULL) == 0){
		printf("mutex successfully initialized\n");
	}
	if(sem_init(&empty,0,5) == 0){
		printf("empty semaphore successfully initialized\n");
	}
	if(sem_init(&full,0,0) == 0){
		printf("full semaphore successfully initialized\n");
	}

	int num = 100;
	int num2 = 200;
	sem_getvalue(&empty, &num);
	printf("Empty semaphore has %d open slots\n", num);
	sem_getvalue(&full, &num2);
	printf("Full semaphore has %d occupied slots\n", num2);

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
	printf("Sleeping now\n\n");
	sleep(sleep_time);
	printf("\nAwake now\n");

	//6. Exit
	//cleanup semaphore and mutex
	if(pthread_mutex_destroy(&mutex) != 0){
		fprintf(stderr, "mutex destroy error\n");
		return 1;
	}else{
		printf("mutex destroyed successfully\n");
	}
	if(sem_destroy(&empty) != 0){
		fprintf(stderr, "empty semaphore destroy error\n");
		return 1;
	}else{
		printf("empty semaphore destroyed successfully\n");
	}
	if(sem_destroy(&full) != 0){
		fprintf(stderr, "full semaphore destroy error\n");
	}else{
		printf("full semaphore destroyed successfully\n");
	}

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
			fprintf(stderr, "Error Producing\n");
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
			fprintf(stderr, "Error Consuming\n");
		}
		printf("Consumer consumed %d\n", item);
	}
}

int insert_item(buffer_item item){
	if(sem_wait(&empty) != 0){
		fprintf(stderr, "sem wait error\n");
		return -1;
	}
	if(pthread_mutex_lock(&mutex) != 0){
		fprintf(stderr, "mutex lock error\n");
		return -1;
	}
	buffer[insert_pointer++] = item;
	insert_pointer = insert_pointer % 5;
	if(pthread_mutex_unlock(&mutex) != 0){
		fprintf(stderr, "mutex unlock error\n");
		return -1;
	}
	if(sem_post(&full) != 0){
		fprintf(stderr, "sem post error\n");
		return -1;
	}
	return 0;
}

int remove_item(buffer_item *item){
	if(sem_wait(&full) != 0){
		fprintf(stderr, "sem wait error\n");
		return -1;
	}
	if(pthread_mutex_lock(&mutex) != 0){
		fprintf(stderr, "mutex lock error\n");
		return -1;
	}
	*item = buffer[remove_pointer++];
	remove_pointer = remove_pointer % 5;
	if(pthread_mutex_unlock(&mutex) != 0){
		fprintf(stderr, "mutex unlock error\n");
		return -1;
	}
	if(sem_post(&empty) != 0){
		fprintf(stderr, "sem post error\n");
		return -1;
	}
	return 0;
}
