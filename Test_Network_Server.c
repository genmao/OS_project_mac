#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <unistd.h>
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

    //Wait for client socket to connect
    socklen_t client_length = sizeof(client_addr);
    char buffer[256];
    int n;
    int status;
    //Keep waiting for client to connect
   for (;;) {
       // Accept new connection
       int s = accept(sockid, (struct sockaddr *) &client_addr, &client_length);//client_length must be set appropriately before call
       if (s < 0)
           printf("Failed to accept!\n");
       //Read from client
       bzero(buffer, 256);
       n = read(s, buffer, 255);
       if(n < 0) printf("Read error!\n");
       printf("Received: %s\n", buffer);
       //Reply "G"
       n = write(s, "G",1);
       if (n < 0)
           printf("Write error!\n");
       status = close(s);
   }
    return status;
}
