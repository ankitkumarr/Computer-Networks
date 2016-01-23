#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>


int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);
int main (int agrc, char **argv) 
{
	int sock; char* port;
	char *host;
	int rtt;
	//CHECKIng the arguments for rtt
	if(agrc < 3)
	{
		printf("Need more arguments");
		return 1;
	}
	
	if(agrc == 4) {
		host = argv[2];
		port = argv[3];
		rtt = 1;
	}

	if(agrc == 3) {
		host = argv[1];
		port = argv[2];
		rtt = 0;
	}
	int j =0;
	int hostlength = strlen(host);
	int flagdot=0; int flagslash = 0;
	char *page = (char*)malloc(sizeof host);
	int pageindex = 0; int temphostindex = 0;
	char* temphost = (char*) malloc(sizeof host);
	//extracting the filename and the hostname asked by the user
	for(j = 0; j < hostlength; j++){
		if(host[j] == '.') {
			flagdot = 1;
		}

		if((host[j] =='/') && (flagdot == 1)) { flagslash =1; }

		if(flagslash == 1) {
			page[pageindex] = host[j];
			pageindex++;
		}

		if(flagslash == 0) {
			temphost[temphostindex] = host[j];
			temphostindex++;
		}

	}
	strcpy(host, temphost);
	host[temphostindex] = '\0';
	page[pageindex] = '\0';
	if(flagslash == 0) {
		page[0]='/';
		page[1]='\0';
	}
	printf("page = %s \n host = %s \n",page, host);
	
	struct sockaddr_in *client;

////////////////////////////////////////////////CREATE THE SOCKET//////////////////////////////////////////////////////////////////////////////////////////////////
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(host, port, &hints, &res); 
	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);    // create a socket of the type specified by getaddrinfo
	if(sock < 0) {
		perror("TCP socket creation failed!");
		exit(1);
	}
	printf("Socket created");
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////RESOLVE THE IP OF THE HOST ENTERED BY THE USER/////////////////////////////////////////////////////////////////////

	char *ip;
	struct hostent *hostdetails;
	int iplength = 15;	// the length of IPv4 
	ip = (char*)malloc(16);	// allocating memory to store ip in the IPv4 format
	memset(ip, 0, 16);	// storing all zeros in ip in the IPv$ format
	hostdetails = gethostbyname(host);
	if(hostdetails == NULL) {
		herror("Problem with ip");
		exit(1);
	}

	if(!inet_ntop(AF_INET, (void *) &hostdetails->h_addr_list, ip, iplength))    //inet_ntop will convert the numeric address in hostdetails and store it in a pointer ip sent to it
												//if it is NULL, resolving failed
	{
		perror("Cannot resolve host");
		exit(1);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// CONNECTING TO THE SERVER /////////////////////////////////////////////////////////////////////////////////////////////
	struct timeval starttime;
	struct timeval endtime;
	client = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in *));	// allocating the memory for the client socket
	client->sin_family = AF_INET;		// specifying IPv4
	if(inet_pton(AF_INET, ip, (void*) (&(client->sin_addr.s_addr))) <= 0) {			//setting the ip address in the client in binary form inside the sin_addr structure (in s_addr)
		perror("error in the ip / cannot set sin_addr");
		exit(1);
	}

	client->sin_port = htons(port);    //argv[3] gives the port number by the user. htons will convert the host byte order to network byte order and store in the sin_port
	int counter = 1;
	int sum = 0;
	if(rtt == 1) {

	printf("\n RTT Values \n");
	counter = 10; }
	int c = 1; int newskd;
	for(c = 1; c <= counter; c++) {

	if(rtt == 1) {
	gettimeofday(&starttime, NULL);
	newskd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);  }  // create a socket of the type specified by getaddrinfo
	if(rtt == 1) {
	     if(connect(newskd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("Connection to the server failed");
		exit(1);
	     }
        }
	if(rtt == 1) {
	gettimeofday(&endtime, NULL); }
	int timed;	
	if(rtt == 1) {
	timed = ((0.001*(endtime.tv_usec)) + (1000*(endtime.tv_sec))) - ((0.001*(starttime.tv_usec)) + (1000*(starttime.tv_sec)));
	sum = sum + timed;
	printf("\nThe time taken is %d\n", timed);
	close(newskd); }
	}
	     if(connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
		perror("Connection to the server failed");
		exit(1);
	     }
	if(rtt == 1) { 
		printf("\n The average is %d\n", (sum/10)); }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// MAKING A GET REQUEST ////////////////////////////////////////////////////////////////////////////////////////////

	char *gettemp = (char *) malloc(1000);
	  /// need to figure out the page thingy
	char *get = (char *) malloc(1000);
	strcpy(gettemp, "GET %s HTTP/1.1\r\nHOST:%s \r\nUser-Agent: Mozilla/5.0\r\nConnection: close\r\n\r\n");
	sprintf(get, gettemp, page, host);
	//strcpy(get, strcat(strcat(strcat(strcat("GET ", page), "HTTP/1.1\r\nHOST: "), host), "\r\nUser-Agent: Mozilla/5.0\r\nConnection: close\r\n\r\n"));
	printf("Query created is: \n %s \n ", get);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// SENDING THE QUERY TO THE SERVER //////////////////////////////////////////////////////////////////////////////////

	int sent = 0; // to store the number of bytes sent
	while(sent < strlen(get)) {

		sent = sent + send(sock, get+sent, strlen(get)-sent, 0);
		// sock is the socket to be sent to
		// get + sent will send the get by moving it to the number of bytes that has been already sent to. So, if x bytes has been sent out already, get + sent will move get x bytes further
		// and then send it. strlen(get) - sent will give it the remaining number of bytes to be sent to the server.

	}
	printf("test");

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// RECEIVING THE PAGE FROM THE SERVER ///////////////////////////////////////////////////////////////////////////////

	char buf[1025]; // last is NULL
	memset(buf, 0, sizeof(buf));	//setting the buffer to all 0s
	int start = 0;
	char *content;
	while(recv(sock, buf, 1024, 0) > 0) {

		if(start == 0) {
			content = strstr ( buf, "\r\n\r\n");
			if(content != NULL) {
				start = 1;
				content += 4;
			}
		}
		else {
			content =buf;
		}
		if (start) {
			printf("Content \n %s", content);
		}

		memset(buf, 0, sizeof(buf));
	}
	close(sock);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////Thanks to http://stackoverflow.com/questions/15846762/timeval-subtract-explanation
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
  struct timeval xx = *x;
  struct timeval yy = *y;
  x = &xx; y = &yy;

  if (x->tv_usec > 999999)
  {
    x->tv_sec += x->tv_usec / 1000000;
    x->tv_usec %= 1000000;
  }

  if (y->tv_usec > 999999)
  {
    y->tv_sec += y->tv_usec / 1000000;
    y->tv_usec %= 1000000;
  }

  result->tv_sec = x->tv_sec - y->tv_sec;

  if ((result->tv_usec = x->tv_usec - y->tv_usec) < 0)
  {
    result->tv_usec += 1000000;
    result->tv_sec--; // borrow
  }

  return result->tv_sec < 0;
}









