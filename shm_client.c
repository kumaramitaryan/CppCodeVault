#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080    // Port number of the server
#define FILE_MAX_SIZE 255

int main(int argc, char* argv[]) 
{
    if (argc < 2) 
	{
        printf("Please provide the file names to read.\n");
        return 1;
    }
	
	int clientSocket;
    struct sockaddr_in serverAddress;

    // Create the client socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error: socket creation failed");
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Error: connection failed");
        exit(1);
    }

	printf("Connected to server.\n");

     // Get the file names from the  console
		int numFiles;
		numFiles=argc-1;
		char fileNameBuffer[FILE_MAX_SIZE];
		memset(fileNameBuffer,'\0',sizeof(fileNameBuffer));
		
		printf("Total number of arguments received = %d\n",numFiles);
		//int i=0;
		char space[] =" ";
		strncpy(fileNameBuffer, argv[1],strlen(argv[1])); // first file Name 
		int i=1;
		for (i; i < numFiles; i++) // 2 ; argv[0], argv[1]
		{
			strcat (fileNameBuffer,space // "sample.txt sample2.txt sam2.txt"
			strcat(fileNameBuffer, argv[i+1] );
			
		}
		//fileNameBuffer[strlen(fileNameBuffer)]='\0';
		printf("Files or command to be sent to the server = %s\n",fileNameBuffer);

    // Send the command to the server
    if (send(clientSocket, fileNameBuffer, strlen(fileNameBuffer), 0) <= 0) {
        perror("Error: send failed from client\n");
        exit(1);
    }

    // Receive the data from the server
    char buffer[2048] = {0};
    if (recv(clientSocket, buffer, sizeof(buffer), 0) <= 0) {
        perror("Error: Server is closed!!!");
        exit(1);
    }
	
	/*
	char line[1024];
	while (1) {
    int read_result = read(clientSocket, line, sizeof(line));
    if (read_result == -1) 
	{
      printf("Error reading line: %d\n", read_result);
      exit(1);
    } 
	else if (read_result == 0) {
      // The server has closed the connection
      break;
    }

    // Print the line
    printf("%s\n", line);
  }*/
	
    printf("Received data:\n%s\n", buffer);
	close(clientSocket);

	return 0;
}
