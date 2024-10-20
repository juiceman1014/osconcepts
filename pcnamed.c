#include "buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

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
	if(pthread_mutex_init(&mutex,NULL) != 0){
		fprintf(stderr, "Mutex error\n");
		return 1;
	}else{
		printf("Mutex successfully initialized\n");
	}

	if((empty = sem_open("/empty_sem", O_CREAT, 0644, 5)) == SEM_FAILED){
		fprintf(stderr, "Empty error\n");
		return 1;
	}else{
		printf("Empty semaphore successfully initialized\n");
	}

	if((full = sem_open("/full_sem", O_CREAT, 0644, 0)) == SEM_FAILED){
		fprintf(stderr, "Empty error\n");
		return 1;
	}else{
		printf("Full semaphore successfully initialized\n");
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

	printf("Sleeping now\n\n");
	//5. Sleep
	sleep(sleep_time);
	printf("\nAwake now\n");
	

	//6. Exit
	//cleanup semaphores and mutex
	if(pthread_mutex_destroy(&mutex) != 0){
		fprintf(stderr, "mutex destroy error\n");
	}else{
		printf("mutex destroyed successfully\n");
	}

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
		item = rand();
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
	if(sem_wait(empty) != 0){
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
	if(sem_post(full) != 0){
		fprintf(stderr, "sem post error\n");
		return -1;
	}
	return 0;
}

int remove_item(buffer_item *item){
	if(sem_wait(full) != 0){
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
	if(sem_post(empty) != 0){
		fprintf(stderr, "sem post error\n");
		return -1;
	}
	return 0;
}
