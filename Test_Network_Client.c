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
//#define HOST_ADDR "127.0.0.1"
//remote
#define HOST_ADDR "192.168.0.16"
#define BUFFER_SIZE 131072
#define ITERATION 100
#define CYCLES_PER_SECOND 2.9e9
static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo)|( ((uint64_t)hi)<<32);
}
double client(char* message){
    uint64_t start = 0, end = 0;
    int sockid;
    struct sockaddr_in server_addr;
    struct hostent *server;
    /* Create a TCP socket in client */
    sockid = socket(PF_INET, SOCK_STREAM, 0);
    if (sockid < 0)
        perror("ERROR opening socket");
    //Assign address and port to socket
    server = gethostbyname(HOST_ADDR);
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(PORT);
    //Connect to server
    if (connect(sockid,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
        perror("ERROR connecting to server");
    //Create message
    char* read_buffer = (char*)malloc(BUFFER_SIZE);
    bzero(read_buffer,BUFFER_SIZE);
    start = rdtsc();
    //Write to and read from server
    printf("=====\n");
    write(sockid, message, strlen(message));
    printf("write.\n");
    read(sockid, read_buffer, BUFFER_SIZE - 1);
    printf("read.\n");
    end = rdtsc();
    close(sockid);
    return (end - start);
}


double RoundTripTime() {

    double total_time = 0.0;
    char *message = "m";
    for(int i = 0; i < ITERATION; i++) {
        total_time += client(message);
    }
    return total_time / ITERATION / CYCLES_PER_SECOND * 1000;
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


double PeakBandwidth() {
    double trans_time, bandwidth, max_bandwidth=0;
    int log_array_size = 17;
    for (int i = 0; i <ITERATION; i++) {
        char *array = rand_char_arr(log_array_size);
        trans_time = client(array);
        bandwidth = pow(2, log_array_size-20) / (trans_time / 2.0 / CYCLES_PER_SECOND);
        max_bandwidth = bandwidth > max_bandwidth ? bandwidth:max_bandwidth;
        printf("Max bandwidth is : %f MB/sec\n", bandwidth);
    }
    return max_bandwidth;
}


double SetUpTime(){
    uint64_t start = 0, end = 0, sum = 0;
    for(int i = 0; i < ITERATION; i++){
        int sockid;
        struct sockaddr_in server_addr;
        struct hostent *server;
        sockid = socket(PF_INET, SOCK_STREAM, 0);
        if (sockid < 0)
            perror("ERROR creating socket");
        server = gethostbyname(HOST_ADDR);
        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        bcopy(server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(PORT);
        start = rdtsc();
        int connection = connect(sockid,(struct sockaddr*) &server_addr,sizeof(server_addr));
        end = rdtsc();
        sum += end - start;
        if (connection < 0)
            perror("ERROR connecting to server");
        close(sockid);
    }
    return sum * 1.0 / ITERATION / CYCLES_PER_SECOND * 1000;
}

double TearDownTime(){
    uint64_t start = 0, end = 0, sum = 0;
    for(int i = 0; i < ITERATION; i++){
        int sockid;
        struct sockaddr_in server_addr;
        struct hostent * server;
        sockid = socket(PF_INET, SOCK_STREAM, 0);
        if (sockid < 0)
            perror("ERROR opening socket");
        server = gethostbyname(HOST_ADDR);
        bzero((char *) &server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        bcopy(server->h_addr, (char *) &server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(PORT);
        if (connect(sockid,(struct sockaddr*) &server_addr,sizeof(server_addr)) < 0)
            perror("ERROR connecting to server");
        start = rdtsc();
        close(sockid);
        end = rdtsc();
        sum += end - start;
    }
    return sum * 1.0 / ITERATION / CYCLES_PER_SECOND * 1000;
}

int main(int argc, const char* argv[]) {
    double setup_time = SetUpTime();
    printf("Connection Setup Time = %f ms\n", setup_time);
    double teardown_time = TearDownTime();
    printf("Connection Teardown Time = %f ms\n", teardown_time);
    double round_trip_time = RoundTripTime();
    printf("Round Trip Time = %f ms\n", round_trip_time);
    double bandwidth = PeakBandwidth();
    printf("Max bandwidth is: %f MB/s\n", bandwidth);
    return 0;
}