/*
  Murat Kaçmaz
  150140052
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>

#define SEMAPHORE_1 500
#define SEMAPHORE_2 550
#define SEMAPHORE_3 600
int sem1,sem2,sem3;
void mysignal(void) {}

void sem_signal(int semid, int val) {
	struct sembuf semafor;
	semafor.sem_num = 0;
	semafor.sem_op = val;
	semafor.sem_flg = 1;
	semop(semid, &semafor, 1);
}

void sem_wait(int semid, int val) {
	struct sembuf semafor;
	semafor.sem_num = 0;
	semafor.sem_op = (-1 * val);
	semafor.sem_flg = 1;
	semop(semid, &semafor, 1);
}

void mysigset(int num) {
	struct sigaction mysigaction;
	mysigaction.sa_handler = (void*)mysignal;
	mysigaction.sa_flags = 0;
	sigaction(num, &mysigaction, NULL);
}



void print(int array[],int N){   // printing the arrays 
	int i ;
	for( i= 0 ; i<N;i++)
	{
		printf("%d ",array[i]);
	}
	printf("\n");

}

void sortChild(int array[],int length)    // insertion sort algorithm to sort
{	int i ;
	int key,j;
	for(  i = 1;i<length ;i++ )
	{
		key = array[i];
		j = i-1;

		while(j>=0 && array[j]>key)
		{
			array[j+1] = array[j];
			j--;
		}
		array[j+1] = key;

	}
	print(array,length);

}



void randomNumberGenerator(int array[],int N)    
{	int i ;
	int randomNumber;
	srand(time(NULL));
	printf("%d ",N);
	for( i = 0; i<N;i++)  // random number generator
	{
		randomNumber = rand()%500;    // i put upper bound of 500 because to see the numbers more clearly.
		array[i]=randomNumber;

	}
	print(array,N);
}

void parallelSorting(int mainArray[],int N)
{
	int i ;
	pid_t leftChild,rightChild,mergeChild;
	leftChild = fork();
	if(leftChild < 0)  { perror ("Fork not working \n");}

	else if(leftChild == 0)  /// left child process
	{
		printf("Cocuk1 calisiyor. \n");
		printf("Cocuk1'in uzerinde calistigi dizi elemanlari : ");
	
		int leftArray[N/2];
		for( i = 0 ; i<N/2;i++)
		{
			leftArray[i] = mainArray[i];
		}
		print(leftArray,N/2);
		printf("Cocuk 1 Dizinin baslangic adresi : %p  bitis adresi : %p \n",&mainArray, &mainArray+N/2-1);
           	printf("Cocuk1 diziyi siralar: ");
		sortChild(leftArray,N/2);
    	for( i = 0 ; i<N/2;i++)
		{
			mainArray[i] =  leftArray[i]  ;
		}
		printf("\n");
    	sem_signal(sem1,1);

	}

	else
	{
		rightChild = fork();
		if(rightChild < 0)  { perror ("Fork not working \n");}

		else if(rightChild == 0)  /// right child process
		{
			printf("Cocuk2 calisiyor. \n");
		    printf("Cocuk2'in uzerinde calistigi dizi elemanlari : ");

			int rightArray[N/2];
			for( i = 0 ; i<N/2;i++)
			{
				rightArray[i] = mainArray[i+N/2];
			}
			print(rightArray,N/2);
      		printf("Cocuk 2 Dizinin baslangic adresi : %p  bitis adresi : %p \n",&mainArray+N/2, &mainArray+N);
			printf("Cocuk2 diziyi siralar: ");
			sortChild(rightArray,N/2);
      		for( i = 0 ; i<N/2;i++)
			{
				 mainArray[i+N/2] = rightArray[i] ;
	 		}
        	printf("\n");
            sem_signal(sem2,1);
		}
		else {

          sem_wait(sem1,1);
          sem_wait(sem2,1);
          mergeChild = fork();
          if(mergeChild < 0)  { perror ("Fork not working \n");}

          else if(mergeChild == 0)  // mergechild ;
          {
              int mergeArray[N];
              int l = 0;
              int j = N/2;
              int k = 0;
              while(l<N/2 && j < N)
              {

                  if(mainArray[l]>mainArray[j])
                    mergeArray[k++]= mainArray[j++];
                  else
                    mergeArray[k++]= mainArray[l++];

              }
              while(l<N/2){
                mergeArray[k++]= mainArray[l++];
              }
              while(j<N){
                  mergeArray[k++]= mainArray[j++];
              }

              printf("Cocuk 3, 1. ve 2.dizileri kaynastiriyor :" );
              for( i = 0;i<N;i++)
              {
                mainArray[i] = mergeArray[i];
                printf("%d ", mainArray[i]);
              }
              printf("\n");
              sem_signal(sem3,1);

          }

          else {
            // root
            sem_wait(sem3,1);
            printf("Anne proses: \nDizinin en kucuk elemani %d . En buyuk elemani %d \n", mainArray[0], mainArray[N-1]);

          }

		} // root

	}
}

int main()
{
	int N;
	int shm_id;  
	int* mainArray;  // main Array

  sem1 = semget(SEMAPHORE_1,1,0700|IPC_CREAT);
  semctl(sem1,0,SETVAL,0);

  sem2 = semget(SEMAPHORE_2,1,0700|IPC_CREAT);
  semctl(sem2,0,SETVAL,0);

  sem3 = semget(SEMAPHORE_3,1,0700|IPC_CREAT);
  semctl(sem3,0,SETVAL,0);

	printf("Enter the size of array: ");
	scanf("%d", &N);

	shm_id = shmget(IPC_PRIVATE, N, IPC_CREAT | 0x1C0);  //get function

	if(shm_id == -1) {
		printf("Shared memory creation failed\n");
		exit(0);
	}

	mainArray = (int *) shmat(shm_id, 0, 0);  // attachemment
	if(mainArray == (int *) -1) {
		printf("Shared memory attach failed\n");
		exit(0);
	}

	randomNumberGenerator(mainArray,N);

	parallelSorting(mainArray,N);
	
	shmdt(mainArray);   // detach shared memory 
	shmctl(shm_id,IPC_RMID,NULL);  // remove shared memory segment

	return 0 ;

}
