#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

void* process(void *);
int main(int agrc, char* argv[]) {
	//thread for multithreading
	pthread_t thread;
	struct sockaddr_in server, client;
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		perror("Error creating a socket");
		exit(-1);
	}

	int portno = atoi(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons(portno);  //changing from host byte ordering to network byte ordering
	server.sin_addr.s_addr = INADDR_ANY;  //bind to all available interfaces in the local machine

	bzero(&server.sin_zero, 8);

	if(bind (sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("Error binding the socket to the port");
		exit(1);
	}

	listen(sock, 15); //can listen 15 connections in a queue
	int clienlength = sizeof(client);

	while(1) { //infinite loop to run all the processes

	int newsocket = accept(sock, (struct sockaddr *) &client, &clienlength); // if the socket accepts a connection it will create a new socket which will no longer be in a listening state.

	if(newsocket < 0) {
		perror("Error, didn't accept properly");
		exit(1);
	}

	if(pthread_create(&thread, NULL, process, &newsocket) != 0)
	{
		perror("Failed to create a thread");
	}

}	
	close(sock);
	return 0;
}



void* process (void* ptr) {

	int newsocket = *((int*)ptr);
	char* bufferread="";  //new buffer to store the entire getfile until it ends
	char buffer[1025]; //Buffer to store the temporary data at every iteration
	bzero(buffer, 1025);
	char* tempforbuffer;
	//do an infinite while loop, exit when end of header is reached \r\n\r\n
	while(1) {

	
	int n = read (newsocket, buffer, 1024);
	// strcat(buffer, bufferread);
	int flag = 0;
	int i;
	tempforbuffer = (char*) malloc(sizeof bufferread); //a temporary variable to store the buffer
	strcpy(tempforbuffer, bufferread);	//copying  bufferread to it
	bufferread = (char*) malloc(strlen(bufferread) + n + 1);	//this is the buffer to store all the data grabbed
        strcpy(bufferread, tempforbuffer);	//storing the data
	strcat(bufferread, buffer);
	int lengthofbuffer = strlen(bufferread); 
	for(i=0; i < lengthofbuffer; i++) {	//exiting when the end is reached
		if(((strlen(bufferread))-i)>3){
			if((bufferread[i] == '\r')&&(bufferread[i+1] == '\n')&&(bufferread[i+2] == '\r') && (bufferread[i+3] == '\n')) {
				flag=1;	
				break;
			}
		}
	}

	if(flag==1) { break;}
	
				
		if(n < 0) {
		perror("Error reading data from the client");
		return NULL;
	}
	}

	int lengthofbuffer = strlen(bufferread);	
	int flag =0;
	char* filetoget = (char *)malloc(lengthofbuffer);
	int x=0;
	int i =0;

	for(i = 0; i < lengthofbuffer; i++) {
		if(bufferread[i] == '/') {
			flag=1;
		}
		if(((bufferread[i] == ' ') || (bufferread[i] == '?')) && (flag == 1)) { break; }
		if(flag == 1) {
			filetoget[x]=bufferread[i];	//storing the file to be extracted
			x++;
		}
	}
	filetoget[x] = '\0';	//to make it a valid string
	filetoget = filetoget + 1;

	printf("File to get is : %s \n", filetoget);		
		
	int fileexists = 0;	//check if the file exists
	struct stat bufferforfile;
	fileexists = (stat (filetoget, &bufferforfile) == 0);
	int n;
	printf("%d",newsocket);
	if(!fileexists) {
		char* successold = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n<h1>404- Not Found</h1>";
		char* success = (char*)malloc(1000);
		sprintf(success, successold, strlen("<h1>404- Not Found</h1>"));
		printf("%s",success);
		int successlength = strlen(success);
		n = write(newsocket, success, successlength);
		shutdown(newsocket, SHUT_WR);
		return NULL;
        }

	
	
	if(fileexists) {
		//if file exists, send all the data from the file
		struct stat st;
		stat(filetoget, &st);
		int size = st.st_size;
		char* successold = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n";
		char* success = (char*)malloc(1000);//sizeof(successold)+sizeof(size)+9);
		sprintf(success, successold, size);
		printf("%s",success);
		int successlength = strlen(success);
		n = write(newsocket, success, successlength);

		
		int inputfile = open(filetoget, O_RDONLY);
		if(inputfile == -1) { perror("failed to open file"); return NULL; }	

		char *buffertosend = (char*)calloc(1024, sizeof (char));
		int charsent;
		while ( (charsent = read(inputfile, buffertosend, 1024)) > 0) {
			write(newsocket, buffertosend, charsent);	//send the whole file
		}
		close(inputfile); //close the file
	}

		
	
	shutdown(newsocket, SHUT_WR); //shut the socket down
	}

	
