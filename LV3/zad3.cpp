#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

#define MATRIX_SIZE 1000
#define NO_THREADS 1

struct timeval start;
struct timeval end;

typedef struct data {
	int startIndex;
	int endIndex;
} DATA, *PDATA;

volatile float A[MATRIX_SIZE * MATRIX_SIZE];

// The function to be executed by all threads
void *myThreadFun(void *lpParam)
{
	int i, j, k;
	
	PDATA pDataArray = (PDATA)lpParam;
	if(pDataArray == NULL){
		printf("pDataArray is NULL!");
	}

	float temp = 0;
	
	for (i = pDataArray->startIndex; i < pDataArray->endIndex; i++)
	{
		for (j = 0; j < MATRIX_SIZE; j++)
		{
			temp = 0;
			for (k = 0; k < i; ++k)
			{
				temp += k * sin(j) - j * cos(k);
			}
			A[i * MATRIX_SIZE + j] = temp;
		}
	}
	pthread_exit(NULL);
	return 0;
}

int main()
{
    int i, j, errorcheck; 
    float sum = 0;
    pthread_t tid;
    pthread_t threadIDs[NO_THREADS];

	PDATA data;

	gettimeofday(&start, NULL);
	
	data = (PDATA)calloc(sizeof(DATA), 1);

    for (i = 0; i < NO_THREADS; i++)
    {
		data->startIndex = i * MATRIX_SIZE / NO_THREADS;
        data->endIndex = (i + 1) * MATRIX_SIZE / NO_THREADS;
		printf("start: %d, end: %d\n", data->startIndex, data->endIndex);
		errorcheck = pthread_create(&tid, NULL, myThreadFun, (void *)data);
		if(errorcheck != 0){			
			printf("Something went wrong on creating pthread");
		}
		threadIDs[i]=tid;
    }

	for (i = 0; i < NO_THREADS; i++)
	{
		pthread_join(threadIDs[i], NULL);
	}

    for (i = 0; i < MATRIX_SIZE; i++)
	{
		for (j = 0; j < MATRIX_SIZE; j++)
		{
			sum += A[i * MATRIX_SIZE + j];
		}
	}

	printf("Sum: %f\n", sum);

	gettimeofday(&end, NULL);
	
	printf("Time taken: %ld\n", ((end.tv_sec*1000000+end.tv_usec)-(start.tv_sec*1000000+start.tv_usec))/1000);

	free(data);
    return 0;
}

