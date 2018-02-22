#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define SIZE 16


union senum{ //Semaphore value
   int val;
   struct semid_ds *buf;
   ushort *array;
};

void lock(int id){//Locks semaphore
   struct sembuf sb;

   sb.sem_num = 0;
   sb.sem_op = -1;
   sb.sem_flg = 0;
   semop(id, &sb, 1);
}

void unlock(int id){//Unlocks semaphore
   struct sembuf sb;

   sb.sem_num = 0;
   sb.sem_op = 1;
   sb.sem_flg = 0;
   semop(id, &sb, 1);
}

int main (int argc, char** argv)
{ 
   union senum sem_val;
   int semId; 
   int status;
   long int i, loop, temp, *shmPtr;
   int shmId;
   pid_t pid;

   semId = semget(IPC_PRIVATE, 1, IPC_CREAT|0600);
   sem_val.val = 1;
   semctl(semId, 0, SETVAL, sem_val);
   
   loop = atoi(argv[1]);
      // get value of loop variable (from command-line argument)

   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n");
      exit (1);
   }
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) {
      perror ("can't attach\n");
      exit (1);
   }

   shmPtr[0] = 0;
   shmPtr[1] = 1;
   //semop(semId, SIGNAL1[0], 1);
   if (!(pid = fork())) {   
      
      for (i=0; i<loop; i++) {
	 lock(semId);	 
               // swap the contents of shmPtr[0] and shmPtr[1]
         temp = shmPtr[0];
	 shmPtr[0] = shmPtr[1];
	 shmPtr[1] = temp;
	 unlock(semId);
      }
      if (shmdt (shmPtr) < 0) {
         perror ("just can't let go\n");
         exit (1);
      }
      exit(0);
   }
   else   
      
      for (i=0; i<loop; i++) {
	 lock(semId);
         temp = shmPtr[0];
	 shmPtr[0] = shmPtr[1];
	 shmPtr[1] = temp;
	 unlock(semId);
               // swap the contents of shmPtr[1] and shmPtr[0]
      }

   wait (&status);
   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);

   if (shmdt (shmPtr) < 0) {
      perror ("just can't let go\n");
      exit (1);
   }
   if (shmctl (shmId, IPC_RMID, 0) < 0) {
      perror ("can't deallocate\n");
      exit(1);
   }
   semctl(semId, 0, IPC_RMID);//Destroy semaphore
   return 0;
}
