#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h> /* for wait */
#include <sys/time.h>
#include <string.h>
#include <wait.h>
#include <time.h>

int signal_app_arrived = 0;
pid_t pid_user;
pid_t pid_app;

void handler(int signumber,siginfo_t* info,void* nonused){
  // printf("Signal with number %i has arrived\n",signumber);
  switch (info->si_code){ 
    case SI_USER:  if(info->si_pid == pid_app) {
					   printf("Signal handler - App's signal handeled!\n");
				       signal_app_arrived = 1;
				   }
				   break;
	case SI_TIMER: printf("It was sent by a timer\n");
                   printf("Additional data: %i\n",info->si_value.sival_int);
                   break;
    case SI_QUEUE: printf("Queue signal arrived!\n");
                   break;
    default: printf("It was sent by something else \n");
  } 
}

int main( int argc, const char* argv[] )
{
	int pipe_app[2];
	int pipe_app_2[2];
	
	int customer_1_in_shop = 0;
	int shop_is_open = 1;
	
	
	// Shared memory stuff
	char *s;
	key_t key;
	int shared_mem_id;
	
	key           = ftok(argv[0], 1);
	shared_mem_id = shmget(key, 500, IPC_CREAT|S_IRUSR|S_IWUSR);
	s             = shmat(shared_mem_id, NULL, 0);
	
	// Set signal
	struct sigaction sigact;
	sigact.sa_sigaction = handler;    //instead of sa_handler, we use the 3 parameter version
	sigemptyset(&sigact.sa_mask); 
	sigact.sa_flags     = SA_SIGINFO; //we need to set the siginfo flag 
	sigaction(SIGUSR1, &sigact, NULL); 
	
	// Set MQ
	// struct sigaction act;
	// struct sigevent notify;
	// struct mq_attr attr;
	// struct mq_attr attr;
	// char *mqname = "/almafa"; // mqname must start with / !!!!!
	// char rcv_buf[MSGSIZE];
	// mqd_t mqdes1, mqdes2;
		
	if (pipe(pipe_app) == -1) {
	   perror("Error while opening pipe_app!");
	   exit(EXIT_FAILURE);
	}	
	
	if (pipe(pipe_app_2) == -1) {
	   perror("Error while opening pipe_app_2!");
	   exit(EXIT_FAILURE);
	}
	
	pid_user = fork();
	// Child User
	if(pid_user == 0) {
		char msg[100];
		
		printf("User started!\n");
		
		pid_app = fork();
		// Child App
		if(pid_app == 0) {
			char username[100];
			char password[100];
			
			printf("App started!\n");
			
			// Sending signal
			kill(getppid(), SIGUSR1);
			printf("App - Ready for communication signal sent!\n");

			close(pipe_app[1]);
			close(pipe_app_2[1]);
			read(pipe_app[0], username, sizeof(username));
			printf("App - User's username: [%s]\n", username);
			read(pipe_app_2[0], password, sizeof(password));
			printf("App - User's password: [%s]\n", password);
			close(pipe_app[0]);
			close(pipe_app_2[0]);
			
			// Write to shared memory
			char buffer[] = "Ez az óraállás képe!";
			strcpy(s, buffer);
			shmdt(s);
			
			printf("App finished!\n");
		} else {
			int child_status;
			
			if(!signal_app_arrived) {
				printf("User - Wating for App's signal...\n");
				pause();
			}	

			printf("User - App's signal arrived! It is ready for communication.\n");
			
			close(pipe_app[0]);
			close(pipe_app_2[0]);
			write(pipe_app[1], "Username", 9);
			printf("User - Username has been sent to the App.\n");
			write(pipe_app_2[1], "Password", 9);
			printf("User - Password has been sent to the App.\n");
			close(pipe_app[1]);
			close(pipe_app_2[1]);
			
			// Wait for app so the shared memory is surely written
			printf("User - Waiting for App to finish...\n");
			waitpid(pid_app, &child_status, 0);
			
			printf("User - Picture from memory: [%s]\n",s);
			shmdt(s);
			
			// Random picture tester
			srand ( time(NULL) );
			if(rand() % 2) {
				printf("User - The picture is clear and visible!\n",s);
			} else {
				printf("User - The picture is NOT clear and visible!\n",s);
			}
		
			printf("User finished!\n");
		}
	// Parent Provider
	} else {
		printf("Provider started!\n");
		int child_status;
		printf("Provider - Wating for User to finish...\n");
		waitpid(pid_user, &child_status, 0);
		printf("Provider - User has finished.\n");
		
	}
	
	return 0;
}