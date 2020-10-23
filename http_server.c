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

    printf("Listening for a client... \n");

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


  /////////////
  // Initialize
  /////////////

  // the buffer to move contents of the file over
  char buffer[256];

  //// what we are going to write back to the client, header + msg combined - NO LONGER NEEDED
  //char output[1400];

  // clients HTTP request info
  char request[128];

  // header for HTTP response
  char header[100];

  // client socket
  int sockfd;
  sockfd = *(int *)arg;

  // location of requested file
  char filepath[80];

  // file descriptor of file info to send
  int inFile;



  ////////////////////////
  // Open the correct file
  ////////////////////////


  // Open requested file or quit//
  if (read(sockfd, request, 80) > 0) {
    printf("Request found: '%s' \n",request);///// remove for final version

    ///////////////////
    // Parsing the file
    //

    // Also made sure to notify the client if they messed up on their end

    // parse the first argument as the command
    char* command = strtok(request," \n\r");
    if (strcmp(command, "GET") != 0){ // currently we only support GET
      write(sockfd, "Command not recognized. Use: GET {file}\n", 40);
      close(sockfd);
      return 0;
    }

    // parse next argument as the directory ///// \n\r to TRIM END OF LINE
    char* directory = strtok(NULL, " \n\r");
    if (directory == NULL){ // happens if there is nothing after GET command
      printf("Directory not found\n");
      write(sockfd, "File not specified. Use: GET {file}\n", 36);
      close(sockfd);
      return 0;
    }

    if (strlen(directory) == 1 && directory[0] == '/'){
      // arguemnt was 'GET / {rest of request}' and we will use default file index.html

      sprintf(filepath, "index.html", 10); // HARDCODED

    } else if (directory[0] != '/'){
      // the directory did not start with '/', incorrect!

      write(sockfd, "Invalid directory, please begin with '/'\n", 41);
      close(sockfd);
      return 0;

    } else {
      // otherwise, read the directory and skip that first /
      printf("Directory to use: '%s'\n", directory);  ///// remove for final commit
      int len = strlen(directory);
      for (int i=1 ; i < len ; i++){
        filepath[i-1] = directory[i];
      }
    }

    //
    ///////////////////

  // open the file
  inFile = open(filepath, O_RDONLY);  // HARD CODED, need to change this so that the client can open any file


  // bad request not read by server
  } else {
    printf("Error: Failed to read client request\n");
    return 0;
  }

  // print error and exit if file not found
  if (inFile < 0){
    printf("Error: file:  '%s' not found\n", filepath);
    printf("Error Number %d\n", errno);
    close(sockfd);
    return 0;
  } else
    printf("File '%s' succesfully found\n", filepath);  ///// remove for final commit


  //////////////////////////////////
  // Send contents of file to client
  //////////////////////////////////

  // To avoid making the server store the entire file, we can use chunked transfer encoding to avoid declaring the content-length
  sprintf(header, "HTTP/1.1 200 OK\nContent-Type: text/plain\nTransfer-Encoding: chunked\n\n");
  write(sockfd, header, strlen(header));
  printf("header sent to client \n");

  int charsRead;
  while (1) {
    charsRead = read(inFile, buffer, sizeof(buffer));
    if (charsRead == 0)
      break;

    write(sockfd, buffer, charsRead);

    printf("Wrote chars to client: \n -------------------- %s \n --------------------", buffer); ///// remove for final commit
  }

  close(sockfd);

}
