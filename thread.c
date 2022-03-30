

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>


#define NUMBERTEAMS 3
#define BUFFERSIZE 10
#define GOAL 50

int puntuation[NUMBERTEAMS];
int buffer[BUFFERSIZE];
//int numberProducers= 0, numberConsumers = 0, wantProduce = 0, wantConsume = 0;
int numberInside = 0, wantInside =0;
int freePosition = 0, firstValue = 0, filled = 0, winner =0;

pthread_mutex_t mutex;
pthread_cond_t  producer_cond, consumer_cond;
pthread_t consumer, producers[NUMBERTEAMS];

//This will be the function of the producers threads
void *race(int id);

//Pre-process/consume and post produce/consume to be sure that the access to the memory is exclusive
void pre_produce();

void post_produce();

void pre_consume();

void post_consume();


//This function will fill the buffer with the id of this thread
void write_buffer(int id);



//the consumer thread will read the buffer, if consumer read a value writed of his team then will obtain double puntuation otherway just one point
void *counter();








int main(int argc, char* argv[])
{		
	int i, ctrl=0;
	//initialize the mutex
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&consumer_cond, NULL);
	pthread_cond_init(&producer_cond, NULL);

	//Launch the threads
	for(i=0; i<NUMBERTEAMS; i++)
	{	
		ctrl = pthread_create(&producers[i], NULL, race, i+1);	
		if(ctrl != 0)
		{
			perror("Error creating producer thread\n");
		}		
	}
	//The consumer thread
	ctrl = pthread_create(&consumer, NULL, counter,NULL);
	if(ctrl != 0)
	{
		perror("Error creating consumer thread\n");
	}
	
	//joining the consumer thread
	ctrl = pthread_join(consumer, NULL);
	if(ctrl != 0)
	{
		perror("Error joining producer thread\n");
	}
	
	printf("\n\nEND\n\n");
	return 0;
}



//This will be the function of the producers threads
void *race(int id)
{
	//if there is a winner then finish the thread
	printf("producer of the team %i starting\n", id);
	sleep(2);
	while(winner==0)
	{
		//Wait until this thread can access to the buffer
		pre_produce();
		if(filled==BUFFERSIZE)
		{
			//the buffer is complete, bad luck for this thread, try the next time.
			pthread_yield();
		}
		else
		{
			write_buffer(id);
		}		
		post_produce();	
		
		//sleep a little to avoid that the first thread can win
		usleep(rand()%10);

	}
}


//Pre-process to be sure that the access to the memory is exclusive
void pre_produce()
{
	pthread_mutex_lock(&mutex);	
	while(numberInside>0)
	{
		wantInside++;
		pthread_cond_wait(&producer_cond, &mutex);	
		wantInside++;
	}
	numberInside++;
	pthread_mutex_unlock(&mutex);	
	
}



void post_produce()
{
	pthread_mutex_lock(&mutex);
	numberInside--;
	//Unlock the consumers threads only if the buffer is complete
	if(filled == BUFFERSIZE )
	{
		pthread_cond_signal(&consumer_cond);
	}else
	{
		pthread_cond_signal(&producer_cond);	
	}
	
	pthread_mutex_unlock(&mutex);
}

//This function will fill the buffer with the id of this thread
void write_buffer(int id)
{
	
	buffer[freePosition]=id;
	
	filled++;
	freePosition++;
	
	if(freePosition == BUFFERSIZE)
	{
		freePosition = 0;
	}	
}

void read_buffer()
{
	//Maybe this thread was waiting to read when other thread found the winner
	if(winner !=0)
	{
		return;
	}
	
	if(filled == 0)
	{
		return;
	}

	puntuation[(buffer[firstValue])-1]++;

	//Check if this team has winned the race
	if(puntuation[ (buffer[firstValue])-1] >=GOAL)
	{
		winner = puntuation[ (buffer[firstValue])-1];

		printf("\nThe team number %i, has won\n", (buffer[firstValue]) );
		for(int j =0; j<NUMBERTEAMS; j++)
		{
			printf("The puntuation of the %i team was: %i\n", j+1 ,puntuation[j] );
		}

	}else
	{				
		firstValue++;
		filled--;
		if(firstValue==10)
		{
			firstValue=0;
		}				
	}
	

}



//the consumer thread will read the buffer, if consumer read a value writed of his team then will obtain double puntuation otherway just one point
void *counter()
{
	printf("consumer starting\n");
	//if there is a winner then finish the thread
	while(winner==0)
	{
		//Wait until this thread can access to the buffer
		pre_consume();		
		read_buffer();		
		post_consume();
		pthread_yield();
	}

}



void pre_consume()
{
	pthread_mutex_lock(&mutex);
	while(numberInside> 0)
	{
		wantInside++;
		pthread_cond_wait(&consumer_cond, &mutex);
		wantInside--;
	}
	numberInside++;
	pthread_cond_signal(&consumer_cond);
	pthread_mutex_unlock(&mutex);
}


void post_consume()
{
	pthread_mutex_lock(&mutex);
	numberInside--;
	if(numberInside == 0)
	{
		//if the buffer is empty wake up a producer
		if(filled == 0)
		{			
			pthread_cond_signal(&producer_cond);	
		}else
		{
			pthread_cond_signal(&consumer_cond);
		}
		
	}
	pthread_mutex_unlock(&mutex);
	//sleep the thread
	pthread_yield();
}






















