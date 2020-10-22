#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
extern int errno;

// When a client connects to server
void *client_handler(void *arg);


int main(int argc, char *argv[])
{

  /////////////
  // Initialize
  /////////////

  // max number of clients to queue when listening for connection
  const int backlog = 5;

  // socket addresses
  struct sockaddr_in  server_addr;
  struct sockaddr_in  client_addr;

  /*
  ///// FYI: sockaddr_in is defined in <netinet/in.h> as:
  /////
  /////   struct sockaddr_in {
  /////     short            sin_family;   // AF_INET
  /////     unsigned short   sin_port;     // port number as int
  /////     struct in_addr   sin_addr;     // address
  /////     char             sin_zero[8];  // probably will be zero
  /////   };
  */

  // separate thread to handle client
  pthread_t tid;

  // socket identifiers
  int sockfd, client_sockfd;
  int serverlen, clientlen;

  /* require correct program execution */
  if (argc != 3) {
    printf("Usage: %s <ip-address> <port> \n", argv[0]);
    return -1;
  }

  /* Create the server socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("Could not create socket");
    return -1;
  }

  /* Define the server socket */
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(argv[1]);
  server_addr.sin_port = htons(atoi(argv[2]));

  /* bind our program to server socket */
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
    perror("Could not bind to socket");
    close(sockfd);
    return -1;
  }



  /////////////////
  // Listening Loop
  /////////////////

  while (1) {

    /* wait for client to connect */
    listen(sockfd, backlog);

    /* Accept a connection */
    clientlen = sizeof(client_addr); // get ready to accept by knowing how many bytes we will read by
    client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clientlen);
    if (client_sockfd == -1) {
      perror("Unable to accept client connection request");
      continue;
    }

    // Handle client on a separate thread
    if (pthread_create(&tid, NULL, client_handler, (void *)&client_sockfd) < 0) {
      perror("Unable to create client thread");
      break;
    }
  }

  /* Done with loop, remove the temp socket */
  close(sockfd);

  /* end program */
  return 0;

}

// Handling the client
void *client_handler(void *arg)
{

  // the contents of the index file
  char msg[8096];

  // what we are going to write back to the client, header + msg combined
  char output[1400];

  // clients HTTP request info
  char request[80];

  // a simple  HTTP response header, when you make a client by going on your  broswer to connect to ip and port, it sends a HTTP request header and body to server
  // so server has to respond with appropriate HTTP header. without it, there would be an error. If you run date_server.c
  // that we did during class, and make a client on your browser, it works only because the server never does
  // the read command, it just accepts the socket connection and writes something back. In our case we need to read (i think) to get
  // name of the file, so we must also include a header
  char header[100];
  int sockfd;
  sockfd = *(int *)arg;

  // OPEN FILE //
  if (read(sockfd, request, 80) > 0) {
    int inFile = open("index.html", O_RDONLY);  // HARD CODED, need to change this so that the client can open any file

  // print error and exit if it fails
   if (inFile < 0){
     printf("Error: file:  %s not found\n", "index.html");
     printf("Error Number %d\n", errno);
     return 0;
   }
   else
     printf("File %s succesfully found\n", "index.html");

   // READ FILE //
   if(read(inFile, msg, 1300) > 0){ // HARD CODED, the index.html is about 1290 bytes long so I just made this 1300
                                    // so we might have to make a larger buffer size, or use a while loop like in hw 1
     msg[strlen(msg)] = '\0';
     //   printf("%s", msg);
   }

   // the content length is the length of the body, aka the file length
   sprintf(header, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %d\n\n", strlen(msg));

   // copy header to output
   memcpy(output, header, strlen(header));

   // concatenate the body (file data) to the header.
   // we cant just do output = header + msg  BTW, cuz output is a pointer so is header and msg. so we need to copy the data from them
   memcpy(&output[strlen(header)], msg, strlen(msg));
   //    output = hello + msg;
   output[strlen(output)] = '\0';


    write(sockfd, output, strlen(output));
	//	printf("DD");



    }

    close(sockfd);

}
