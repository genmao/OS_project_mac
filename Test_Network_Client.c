#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define PORT 49152
//loopback
#define HOST_ADDR "127.0.0.1"
//remote
//#define HOST_ADDR "140.112.30.38"
#define BUFFER_SIZE 4096
#define ITERATION 100
#define CYCLES_PER_SECOND 2.9e9
static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (uint64_t)lo)|( ((uint64_t)hi)<<32);
}

double client(char* message){
    uint64_t start = 0, end = 0, elapsed = 0;
    struct sockaddr_in server_addr;
    struct hostent *server;

    /* Create a TCP socket in server */
    int sockid = socket(PF_INET, SOCK_STREAM, 0);
    if (sockid < 0){
        printf("Failed to create socket!\n");
        return sockid;
    }

    //Specify attributes of server_addr
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    //Specify host address
    server = gethostbyname(HOST_ADDR);
    memcpy(server->h_addr, &server_addr.sin_addr.s_addr, server->h_length);
    //Establise connection to server
    if (connect(sockid, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        printf("Failed to connect to server!\n");
        return -1;
    }

    //Initialize the message buffer
    //char* write_buffer = (char*)malloc(BUFFER_SIZE);
    char* read_buffer = (char *) malloc(BUFFER_SIZE);
    bzero(read_buffer, BUFFER_SIZE);
    //bzero(write_buffer, BUFFER_SIZE);
    //strncpy(write_buffer, msg, sizeof(char)*strlen(msg));
    int len = 0;

    start = rdtsc();
    //Write to server
    len = write(sockid, message, strlen(message)); //Supposed to be blocking write
    // if (len < 0)
    //    perror("ERROR writing to socket");
    //Read from server
    len = read(sockid, read_buffer, BUFFER_SIZE-1);
    // if (len < 0)
    //    perror("ERROR reading from socket");
    end = rdtsc();
    //printf("Message receive from server: %s\n", read_buffer);

    close(sockid);
    return (end - start);
}

double RoundTripTime() {

    double total_time = 0.0;
    char *message = "s";

    for(int i = 0; i < ITERATION; i++) {
        total_time += client(message);
    }
    return total_time / ITERATION ;
}
char* rand_char_arr(int log_array_size) {
    srand(time(NULL));
    uint64_t array_size = (pow((double)2, log_array_size));
    char* char_arr = (char*)malloc(sizeof(char) * array_size);
    uint64_t i = 0;
    for (i = 0; i < array_size; i++) {
        *(char_arr + i) = rand() % 26 + 'a';
    }
    return char_arr;
}
double peak_bandwidth() {
    double nano_sec;
    double bandwidth;
    double max = 0;
    //array size = 64KB
    //char *array = rand_char_arr(16);
    //printf("%lu\n", strlen(array));
    //1KB to 1GB
    for (int i = 10; i <130; i++) {
        char *array = rand_char_arr(i);
        nano_sec = client(array)/2.0;
        //byte/sec
        bandwidth = pow(2,i-20)*2.9 / (nano_sec * pow(10, -9));
        max = bandwidth > max ? bandwidth:max;
        //printf("Bandwidth: %f MB/sec\n", bandwidth);
    }
    return max;
}

double SetUpTime(){

    uint64_t start = 0, end = 0, total_cycle = 0;
    for(int i = 0; i<ITERATION; i++){
        int sockid;
        struct sockaddr_in server_addr;
        struct hostent *server;

        sockid = socket(PF_INET, SOCK_STREAM, 0);
        if (sockid < 0)
            perror("ERROR opening socket");

        server = gethostbyname(HOST_ADDR);

        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        bcopy(server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(PORT);

        //Measure the time of establishing connection
        int connection;
        start = rdtsc();
        connection = connect(sockid,(struct sockaddr*)&server_addr,sizeof(server_addr));
//        if (connect(sockid,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
//            perror("ERROR connecting to server");
        end = rdtsc();
        total_cycle += end - start;

        if (connection < 0)
            printf("Failed to connect to server!\n");
        close(sockid);

    }
    return total_cycle / ITERATION;
}

double TearDownTime(){
    uint64_t start = 0, end = 0, total_cycle = 0;
    for(int i = 0; i<ITERATION; i++){
        int sockid;
        struct sockaddr_in server_addr;
        struct hostent *server;
        if ((sockid = socket(PF_INET, SOCK_STREAM, 0))< 0)
            perror("ERROR opening socket");

        server = gethostbyname(HOST_ADDR);
        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(PORT);

        if (connect(sockid,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
            perror("ERROR connecting to server");

        //Measure the time of closing socket
        start = rdtsc();
        close(sockid);
        end = rdtsc();

        total_cycle += end-start;

    }
    return total_cycle / ITERATION;
}

int main(int argc, const char* argv[]) {

    double setup_time = SetUpTime();
    printf("Connection Setup Time = %f\n", setup_time/2.9);

    double teardown_time = TearDownTime();
    printf("Connection Teardown Time = %f\n", teardown_time/2.9);

    double rtt = RoundTripTime();
    //printf("Round Trip Time = %f\n", rtt-setup_time-teardown_time);
    printf("Round Trip Time = %f\n", rtt/2.9);

    double bandwidth = peak_bandwidth();
    printf("Max bandwidth: %f MB/sec\n", bandwidth);


    return 0;
}