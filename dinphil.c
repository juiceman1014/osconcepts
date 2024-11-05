#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdilib.h>
#include <unitstd.h>

#define NUMBER 5
#define MAX_SLEEP_TIME 5

enum{THINKING, HUNGRY, EATING} state[NUMBER];

int thread_id[NUMBER];

pthread_cond_t cond_vars[NUMBER];
pthread_mutex_t mutex_lock;

int left_neighbor(int number){
	if(number == 0){
		return NUMBER - 1;
	}else{
		return number - 1;
	}
}

int right_neighbor(int number){
	if(number == NUMBER - 1){
		return 0;
	}else{
		return number + 1;
	}
}

void test(int i){
	if((state[left_neightbor(i)] != EATING) && (state[i] == HUNGRY) && (state[right_neighbor(i)] !+ EATING)) {
		state[i] = EATING;	//set philosopher i's state to eating
		pthread_cond_signal(&cond_vars[i]); //signal philospher i to start eating
	}
}

void pickup_forks(int number){
	pthread_mutex_lock(&mutex_lock);

	state[number] = HUNGRY;	//set philosopher state to hungry
	test(number); 	//check if philosopher can start eating

	//wait if philosopher can not eat
	while(state[number] != EATING){
		pthread_cond_wait(&cond_vars[number], &mutex_lock); //wait for signal to start eating
	}

	pthread_mutex_unlock(&mutex_lock);
}

void return_forks(int number){
	pthread_mutex_lock(&mutex_lock);

	state[number] = THINKING;
	test(left_neighbor(number));
	test(right_neighbor(number));

	pthread_mutex_unlock(&mutex_lock);
}

void eating(int sleep_time){
	sleep(sleep_time);
}

void thinking(int sleep_time){
	sleep(sleep_time);
}

void *philosopher(void *param){
	int *lnumber = (int *) param; //get philosopher number
	int number = *lnumber;
	int sleep_time;
	int times_through_loop = 0;

	srandom((unsigned)time(NULL));

	while(times_through_loop<5){
		sleep_time = (int)((random() % MAX_SLEEP_TIME + 1));
		thinking(sleep_time);	//philosopher starts by thinkign

		pickup_forks(number);	//attempt to pick up forks

		printf("Philosopher %d is eating\n",number);	//if pickup forks is successful, that means philosopher is eating
                printf(" left is ");
                if (state[left_neighbor(number)] == THINKING)
                    printf("THINKING");
                else if (state[left_neighbor(number)] == HUNGRY)
                    printf("HUNGRY");
                else
                    printf("EATING");

                printf(" and right is ");
                if (state[right_neighbor(number)] == THINKING)
                    printf("THINKING");
                else if (state[right_neighbor(number)] == HUNGRY)
                    printf("HUNGRY");
                else
                    printf("EATING");

        sleep_time = (int)((random() % MAX_SLEEP_TIME) +1);
        eating(sleep_time);	//eating time

        printf("Philosopher %d is thinking\n",number);	//phil is done eating and now must think
		return_forks(number);
		
		++times_through_loop;
	}
	return lnumber;
}

pthread_t tid[NUMBER];

void init()
{
int i;

	for (i = 0; i < NUMBER; i++) {
		state[i] = THINKING;
		thread_id[i] = i;
		pthread_cond_init(&cond_vars[i],NULL);
	}

	pthread_mutex_init(&mutex_lock, NULL);
}

void create_philosophers()
{
int i;

	for (i = 0; i < NUMBER; i++) {
		pthread_create(&tid[i], 0, philosopher, (void *)&thread_id[i]);
	}
}

int main(void)
{
int i;

	init();

	create_philosophers();

	for (i = 0; i < NUMBER; i++)
		pthread_join(tid[i],NULL);

	return 0;
}














