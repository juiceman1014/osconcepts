#include "buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define TRUE 1

buffer_item buffer[BUFFER_SIZE];

pthread_mutex_t mutex;
sem_t *empty;
sem_t *full;

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
	if((empty = sem_open("/empty_sem", O_CREAT, 0644, 5)) == SEM_FAILED){
		fprintf(stderr, "Empty error\n");
		return 1;
	}else{
		printf("Empty success\n");
	}

	if((full = sem_open("/full_sem", O_CREAT, 0644, 0)) == SEM_FAILED){
		fprintf(stderr, "Empty error\n");
		return 1;
	}else{
		printf("Full success\n");
	}

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

	printf("Sleeping now\n");
	//5. Sleep
	sleep(sleep_time);
	printf("Awake now\n");
	

	//6. Exit
	if(sem_close(empty) == -1){
		fprintf(stderr, "Empty close error\n");
		return 1;
	}else{
		printf("empty close success\n");
	}

	if(sem_close(full) == -1){
		fprintf(stderr, "Full close error\n");
		return 1;
	}else{
		printf("full close success\n");
	}

	if(sem_unlink("/empty_sem") == -1){
		fprintf(stderr, "Empty unlink error\n");
		return 1;
	}else{
		printf("empty unlink success\n");
	}

	if(sem_unlink("/full_sem") == -1){
		fprintf(stderr, "Full unlink error\n");
		return 1;
	}else{
		printf("full unlink success\n");
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
			fprintf(stderr, "Error Producing");
		}
		printf("Producer produced %d\n", item);
	}	
	return NULL;
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
	return NULL;
}

int insert_item(buffer_item item){
	sem_wait(empty);
	pthread_mutex_lock(&mutex);
	buffer[insert_pointer++] = item;
	insert_pointer = insert_pointer % 5;
	pthread_mutex_unlock(&mutex);
	sem_post(full);
	return 0;
}

int remove_item(buffer_item *item){
	sem_wait(full);
	pthread_mutex_lock(&mutex);
	*item = buffer[remove_pointer++];
	remove_pointer = remove_pointer % 5;
	pthread_mutex_unlock(&mutex);
	sem_post(empty);
	return 0;
}
