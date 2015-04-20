#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include <iostream>
using namespace std;


	sem_t sem_chairs;//chair avail in waiting room
	sem_t barbs_avail;//num of avail barbs to give haircut
	sem_t sem;	
	pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

	void *barber(void *arg);
	void *client(void *arg);
	int check_waiting_room();
	int barb_avail();

	struct client_data{
		pthread_t *thrd;
	};

	pthread_t *waiting_room;
	sem_t headcount;

int main(int argc, char **argv){
	int valid;
	int size_sem_chair;
	int i;
	void *canceled;//used to check if thread had pthread_cancel called	

	if(argc != 6){
		fprintf(stderr,"usage:./hw3 num_barbers num clients num_chairs arrival_t haircut_t\n");
		exit(1);
	}

	for(i = 1; i <= argc-1 ; i++){
		valid = strtol(argv[i],NULL,10);		
		if(valid <= 0){
			fprintf(stderr,"ERROR: all arguments must be > 0\n");
			exit(1);
		}
	}

	
	int num_barbers;
	int num_clients;
	int num_chairs;
	int arrival_t;
	int haircut_t;
	
	num_barbers = strtol(argv[1],NULL,10);
	num_clients = strtol(argv[2],NULL,10);
	num_chairs = strtol(argv[3],NULL,10);
	arrival_t = strtol(argv[4],NULL,10);
	haircut_t = strtol(argv[5],NULL,10);
	

	pthread_t cap [num_chairs];//waiting room capacity
	waiting_room = cap;//transfer to global buffer
	
	//initialize semaphores
	sem_init(&sem_chairs,0,num_chairs);
	sem_init(&barbs_avail,0,num_barbers);

	pthread_t barbers[num_barbers];
	pthread_t clients[num_clients];

	//create barbers
	for(i = 0; i < num_barbers; i++){
		if(pthread_create(&barbers[i],NULL,barber,NULL) != 0){
			fprintf(stderr,"ERROR: barber pthread_create fail\n");
			exit(1);
		}
	}

	
	struct client_data* cd;
	//create clients	
	for(i = 0; i < num_clients; i++){
		cd = (struct client_data*) malloc(sizeof(cd));
		cd->thrd = &clients[i];
		cout << " clients la la : " << clients[1] << endl;
		if(pthread_create(&clients[i],NULL,client,(void *)cd) != 0){
			fprintf(stderr, "ERROR: client pthread_create fail\n");
			exit(1);
		}
		
	}

	//initialize semaphore
	if(sem_init(&sem_chairs,0,num_chairs) != 0){
		fprintf(stderr, "ERROR: sem_init fail");
		exit(1);
	}


//	sem_getvalue(&sem_chairs,&size_sem_chair);
//	cout << "this is size_sem_chair: " << size_sem_chair << endl;

	for(i = 0; i < num_barbers; i++){
		if(pthread_join(barbers[i],NULL) != 0 ){
			fprintf(stderr,"ERROR: barber pthread_join fail\n");
			exit(1);
		}
	}

	for(i = 0; i < num_clients; i++){

		if(pthread_join(clients[i],&canceled) !=0){
			fprintf(stderr,"ERROR: client pthread_join fail\n");
			exit(1);
		}
		if(canceled == PTHREAD_CANCELED){
			printf("main(),thread was canceled\n");
		}
	}
}

void *barber(void *arg){
	int i;

	if(sem_post(&barbs_avail) != 0){
		fprintf(stderr,"ERROR: sem_post fail\n");
	}
	if(pthread_mutex_lock(&lock1) !=0){
		fprintf(stderr,"ERROR: mutex lock fail\n");
		exit(1);
	}
	cout << " Barber " << endl;
	sleep(2);
	if(pthread_cancel(waiting_room[0]) != 0){
		cout << "failed " << endl;
	}
	cout << "waiting_room[0] barber cancel " << waiting_room[0] << endl;
	if(pthread_mutex_unlock(&lock1) !=0){
		fprintf(stderr,"ERROR: mutex lock fail\n");
		exit(1);
	}
}

void *client(void *arg){
	struct client_data *cl_data = (struct client_data*) arg;
	waiting_room[0] = pthread_self();

	sleep(3);

	if(pthread_mutex_lock(&lock1) !=0){
		fprintf(stderr,"ERROR: mutex lock fail");
		exit(1);
	}
	
	//logic for clients arriving to barber shop
	if(barb_avail()){
		//cut hair
		printf("%s","cut hair");
		sem_wait(&barbs_avail);
		
	}else{//barb not avail, check to see if a seat is avail
		if(check_waiting_room()){
			printf("%s","client waiting ");
		}else{//nothing avail leave
			pthread_cancel(pthread_self());
		}	
	}

	if(pthread_mutex_unlock(&lock1) !=0){
		fprintf(stderr,"ERROR: mutex lock fail");
		exit(1);
	}
}

//client arrives,must be used within mutex lock, client is seated if room is avail
int check_waiting_room(){
	int sem_value;
	
	if(sem_getvalue(&sem_chairs,&sem_value) != 0){
		fprintf(stderr,"ERROR: sem_getvalue fail");
		exit(1);
	}
	if(sem_value > 0){
		sem_wait(&sem_chairs);
		cout << "arrived inside of  if sem_value " << sem_value << endl;
		waiting_room[sem_value -1] = pthread_self();	
		cout << "waiting room: " << waiting_room[sem_value - 1] << endl;
		return 1;
	} 
	return 0;
}

int barb_avail(){
	int b_avail;
	if(sem_getvalue(&barbs_avail,&b_avail) != 0){
		fprintf(stderr,"ERROR: sem_getvalue fail");
	}
	
	if(b_avail > 0){
		return 1;
	}
	
	return 0;
}















