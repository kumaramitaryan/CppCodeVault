#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>  //gcc -pthread shm_server.c -o startServer
#include <sys/syscall.h>

pthread_mutex_t lock;

#define SHM_SIZE 4096   	// Size of shared memory segment
#define PORT 8080       	// Port number for server
#define FILE_MAX_SIZE 255 	// file size

//Struct to passe as argumnets in the thread.
struct thread_args
{
	char filename[FILE_MAX_SIZE];
	char* sharedMemory;
};

void *newProcessFiles(void *arg)
{
	// checking for thread id, for each threads - should be different for all.
	printf("Thread created with ID - [%d]\n",syscall(__NR_gettid));
	
	struct thread_args *args = (struct thread_args *)arg;
	char inputFileName[FILE_MAX_SIZE];
	memset(inputFileName,'\0',sizeof(inputFileName));
	strncpy(inputFileName,args->filename,sizeof(inputFileName));
	char* shared_memory      = args->sharedMemory;
	
	// debug line to check the address of shared memory for each thread, should be same as it is shared accross.
	printf("Address of the shared memory = %p\n",shared_memory);
			
	// Set the working directory to file location.
	if (chdir("/home/persistent.co.in/amit_kumar17/Amit") < 0) 
	{
		perror("Working directory change error");
		exit(1);
	}
	
	pid_t childPid = fork();
	//printf("New child process created with id = %d\n", getpid());
    
	if (childPid == 0) 
	{
		// Child process
        FILE* file = fopen(inputFileName, "r");
		if (file == NULL) 
		{
            perror("Error: fopen failed");
            exit(1);
        }

        char line[1024];
		// Find the end of the existing data
		size_t existing_length = strlen(shared_memory);
		if (existing_length > 0 && shared_memory[existing_length - 1] == '\n') 
		{
			shared_memory[existing_length - 1] = '\0';  // Remove the newline character
		}
		

		char line1[1024];
		while (fgets(line, sizeof(line), file) != NULL) 
		{	
			sprintf(line1,"%s : %s",inputFileName,line);
			// Acquire a lock on the shared memory
			pthread_mutex_lock(&lock);
			
			strncat(shared_memory, line1, SHM_SIZE - existing_length - 1);

			// Release the lock on the shared memory
			pthread_mutex_unlock(&lock);
            sleep(1);
        }
		shared_memory[SHM_SIZE - 1] = '\0';
        fclose(file);
		pthread_exit(NULL);
		
    } // read and write done.
	
	else if (childPid > 0) 
	{
           // Parent process
           wait(NULL);
    }  
	else // childpid < 0 case
	{
           perror("Error: fork failed");
           exit(1);
    }

}// newProcessFiles function end

// Main Function begins here.

int main() 
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
	socklen_t addrlen = sizeof(clientAddress);
    int opt = 1;
	char fileNames[FILE_MAX_SIZE];
	 
	// Settings for shared Memory segment  starts 
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		int shmid;
		key_t key;
		char* shared_memory;
		char buffer[SHM_SIZE];

		// Generate a unique key
		key = ftok(".", 'R');
		if (key == -1) 
		{
			perror("Error: ftok failed");
			exit(1);
		}
		
		// Create the  shared memory segment
		shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
		if (shmid == -1) {
			perror("Error: shmget failed");
			exit(1);
		}

		// Attach to the shared memory segment
		shared_memory = (char*)shmat(shmid, NULL, 0);
		if (shared_memory == (char*)-1) {
			perror("Error: shmat failed");
			exit(1);
		}

	// Settings for shared Memory segment ends 
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// socket -> bind-> listen -> accept in a infite loop.
	
    // 1. Create the server socket of TCP/IP -> SOCK_STREAM
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if ( serverSocket < 0 ) 
	{
        perror("Socket creation error"); 
        exit(1);
    }
	// 2. Bind the server socket to a specific address and port
	memset(&serverAddress, 0, sizeof(serverAddress));
	
	serverAddress.sin_family = AF_INET; // Internet
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // Take any available IPs
	serverAddress.sin_port = htons(PORT); // Macro defined at global space and casted to host to network system call menthos htons.

	//Bind -> IPAddress + SocketFD
    if (bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) 
	{
        perror("Socket binding error");
        exit(1);
    }

    // 3. Start listening for client connections
    if (listen(serverSocket, 5) == -1)  // 5-> Maximum 5 request can be handled at a time.
	{
        perror("Socket listen error or max value reached.");
        exit(1);
    }

	printf("Server started. Listening on port %d...\n", PORT);
	
	//4. Accept incoming client connection in a while loop and receive a command from clinet to process.

while (1) 
{
	
	clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addrlen);
    
	if (clientSocket < 0) 
	{
        error("Error accepting connection");
        exit(1);
	}
	
	//printf("connected: %s:%d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
	// Receive the command from the  client
	
	memset(fileNames, 0, sizeof(fileNames));
	
    if (recv(clientSocket, fileNames, sizeof(fileNames), 0) <= 0) {
        perror("Error: recv failed");
        exit(1);
    }
	
	// in case of exit command 
	if ( strncmp(fileNames, "exit" , 4) == 0 )
	{
		printf("exit command encountered, connection is being closed..\n");
		close(clientSocket);
		printf("Client disconnected. - Bye!!!\n");
		close(serverSocket);
		//stopServer();
		exit(0);
	}
	else
	{
		printf("Files received from the client = %s\n",fileNames); //sample1.txt sample2.txt 
		//------------------------------------------------------------------------	
		// Handle the client's command get <fileName1> <fileName2> ...
		// Segregating the streamed filename into an fileName Array to process individually.
		
		char delimiter[]=" ";
		char* inpfileName = strtok(fileNames,delimiter);
		int filecount=0;
		char *file_names_arr[10];// max ten files.
		int i =0;
		while (inpfileName != NULL)
		{
			file_names_arr[i++] = inpfileName;
			inpfileName = strtok(NULL,delimiter);
			filecount++;
		}
		// checks for the fileCount.
		printf("File count = %d\n",filecount);
		
		// Creating struct object array to initilaize its memeber in the loop each time.
		struct thread_args args[filecount];
		
		// Create the threads pool equals the no of files.
		pthread_t threads[filecount];
		
		i=0;
		
		// Here in the loop, each time a new stucture (Array) and  being initialized and pass to the thread.
		
		for (i; i < filecount; i++) 
		{
			// +++++++++++++ struct  Prepration +++++++++++++++++++//
			args[i].sharedMemory=shared_memory;
			sprintf(args[i].filename,"%s",file_names_arr[i]); 
			// +++++++++++++ struct  Prepration +++++++++++++++++++//
			
			// Thread creation in the loop for each file, and newProcessFiles will run accordingly each time.
			int r = pthread_create(&threads[i],NULL,newProcessFiles,&args[i]);
			if (r != 0)
			{
				printf("Error occured while creating thread\n");
				return 1;
			}

		}
		// Joining each thread in a loop.
		i=0;
		for (i; i < filecount; i++) 
		{
			pthread_join(threads[i], NULL);
			printf("Thread joined\n");
		}
		
		// Sending whole shared memory at once.
		send(clientSocket, shared_memory, strlen(shared_memory), 0);
		
		// Detach from the shared memory segment
		if (shmdt(shared_memory) == -1) 
		{
			perror("Error: shmdt failed");
			exit(1);
		}

		// Deallocate the shared memory segment
		if (shmctl(shmid, IPC_RMID, 0) == -1) 
		{
			perror("Error: shmctl failed");
			exit(1);
		}
		
		printf("Data written to shared Memory and sent back to client successfully.\n");
		printf("Connection closing...\n");
		close(clientSocket);
		printf("Client disconnected.\n");
	}
} // while loop end
	close(serverSocket);
	return 0;
}// end of main.



// ************************* END of the Document *******************************