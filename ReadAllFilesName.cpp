#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 255 

void printFiles(char** filenames, int fileCount)
{
	int fileIndex;
	for (fileIndex = 0; fileIndex < fileCount; fileIndex++) 
	{
        printf("%s ", filenames[i]);  
    }
}


int main(int argc, char* argv[]) 
{
	int numFiles;
	char** filenames;

	if (argc<2)
	{
		 printf("Please provide the files to read.\n");
         return 1;
	}
	numFiles=argc-1;
	
	filenames = (char**)malloc(numFiles * sizeof(char*));
	
	int i;
	for (i = 0; i < numFiles; i++) 
	{
		filenames[i] = (char*)malloc(BUFFER_SIZE * sizeof(char));
		strncpy(filenames[i], argv[i+1], strlen(argv[i+1]));
		filenames[strlen(argv[i+1])]='\0';
	}
	printFiles(filenames,numFiles);
	
	for (i = 0; i < numFiles; i++)
	{
     free(filenames[i]);  
    }
    free(filenames);  
	
return 0;
}
