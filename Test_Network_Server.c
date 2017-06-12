#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#define PORT 49152
int main(int argc, char *argv[]) {
    /* Create a TCP socket in server */
    int sockid = socket(PF_INET, SOCK_STREAM, 0);
    if (sockid < 0){
        printf("Failed to create socket!\n");
        return sockid;
    }
    //Assign address and port to socket using bind()
    struct sockaddr_in server_addr, client_addr;
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);         //port
    server_addr.sin_addr.s_addr = INADDR_ANY;   //Internet address, set to INADDR_ANY
    if (bind(sockid, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        printf("Failed to assign address to socket!\n");
        return -1;
    }
    // Set socket to listen
    int queueLimit = 5;
    if (listen(sockid, queueLimit) < 0){
        printf("Failed to listen!\n");
        return -1;
    }

    /* Wait for client socket to connect */
    socklen_t client_length = sizeof(client_addr);
    char buffer[2097152];
    int status;
    for (;;) {
       // Accept new connection
       int s = accept(sockid, (struct sockaddr *) &client_addr, &client_length);//client_length must be set appropriately before call
       if (s < 0)
           printf("Failed to accept!\n");
       //Normal mode
       bzero(buffer, 256);
       if((read(s, buffer, 255)) < 0){
           printf("Read error!\n");
           return -1;
       }
       printf("Received: %s\n", buffer);
       //Reply a character
       if ((write(s, "s", 1)) < 0){
           printf("Write error!\n");
       }

//       //For bandwidth
//       bzero(buffer, pow(2,21));
//       if((read(s, buffer, pow(2,21))) < 0){
//           printf("Read error!\n");
//           return -1;
//       }
//       printf("Received: %s\n", buffer);
//       //Reply string
//       if ((write(s, buffer, pow(2,21)-1)) < 0){
//           printf("Write error!\n");
//           return -1;
//       }
//
       status = close(s);
   }
    return status;
}
