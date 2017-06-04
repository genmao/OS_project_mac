
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/syscall.h>
#include <fcntl.h>

#define KB 1024
#define MB 1024 * 1024
#define GB 1024 * 1024 * 1024
#define ITERATION 10
#define BLOCKSIZE 4096
#define FileSize  4194304

static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

double FileCacheSize(char* file_path, uint64_t file_size){
    uint64_t start = 0, end = 0, total_time = 0;

    int i = 0;
    for(i = 0; i < ITERATION; i++){
        int fd = open(file_path, O_RDONLY | O_SYNC);
        // Read a block a time
        char* read_buffer=(char*)malloc(BLOCKSIZE);
        ssize_t total_bytes = 0;

        while(total_bytes < file_size){
            start = rdtsc();
            // read a block a time
            ssize_t bytes_read = read(fd, read_buffer, BLOCKSIZE);
            //printf("%zd\n", bytes_read);
            end = rdtsc();

            if (bytes_read <= 0)
                perror("ERROR reading file");

            total_time += end - start;

            total_bytes += bytes_read;
        }
        //printf("%llu\n", total_time);

        // start = rdtsc();
        // ssize_t bytes_read = read(fd, read_buffer, BLOCKSIZE);
        // end = rdtsc();

        // total_time += end - start;

        free(read_buffer);
        close(fd);
    }
    printf("%f\n", total_time / (ITERATION * 2.7));

    return total_time/(ITERATION);
}

void SequentialRead(char* file_path, uint64_t file_size){
    uint64_t start = 0, end = 0, total_time = 0;

    unsigned long total_blocks = file_size / BLOCKSIZE;
    //printf("%s\n", file_path);
    int i = 0, j = 0;
    for(i = 0; i< ITERATION; i++){
        //printf("%d\n",i);
        int fd = open(file_path, O_RDONLY | O_SYNC);
        // dont use file cache
        // similar to O_DIRECT flag but mac doesn't have it
        fcntl(fd, F_NOCACHE);
        char* read_buffer=(char*)malloc(BLOCKSIZE);

        for (j = 0; j < total_blocks; j++){
            start = rdtsc();
            ssize_t bytes_read = read(fd, read_buffer, BLOCKSIZE);
            end = rdtsc();
            if (bytes_read <= 0)
                perror("ERROR reading file");

            // if read reach end of file, rewind
            // if (bytes_read <= BLOCKSIZE)
            // 	lseek(fd, 0, SEEK_SET);
            total_time += end - start;
        }
        //lseek(fd, 0, SEEK_SET);
        free(read_buffer);
        close(fd);
    }
    printf("%f\n", log2(total_time/(ITERATION * total_blocks * 2.7)));
}

void RandomRead(char* file_path, uint64_t file_size){
    uint64_t start = 0, end = 0, total_time = 0;

    unsigned long total_blocks = file_size/BLOCKSIZE;

    srand(time(NULL));

    int i = 0, j = 0;
    for(i = 0; i< ITERATION; i++){
        //printf("%d\n",i);
        int fd = open(file_path, O_RDONLY | O_SYNC);
        // dont use file cache
        // similar to O_DIRECT flag but mac doesn't have it
        fcntl(fd, F_NOCACHE);
        char* read_buffer=(char*)malloc(BLOCKSIZE);

        for (j = 0; j < total_blocks; j++){
            int rand_offset = rand()%total_blocks * BLOCKSIZE;
            //printf("%d\n", rand_offset);

            lseek(fd, rand_offset, SEEK_SET);
            start = rdtsc();
            ssize_t bytes_read = read(fd, read_buffer, BLOCKSIZE);
            end = rdtsc();
            if (bytes_read <= 0)
                perror("ERROR reading file");

            // if read reach end of file, rewind
            // if (bytes_read <= BLOCKSIZE)
            // 	lseek(fd, 0, SEEK_SET);
            total_time += end - start;
        }
        free(read_buffer);
        close(fd);
    }


    printf("%f\n", log2(total_time/(ITERATION * total_blocks * 2.7)));
}
void Contention(int proc_num){
    int i, j;
    pid_t parent_pid = getpid();
    //int proc_num = 20;
    /*
    for (i = 1; i <= proc_num; i++) {
        pid_t pid = fork();
        //printf("%d\n",pid);
        if (pid == 0) {
            char file_name[100];
            int n = sprintf(file_name, "../Ignore_File/%d", i);
            //printf("%s\n",file_name);
            int fd = open(file_name, O_RDONLY|F_NOCACHE);
            //printf("%d\n",fd);
            for (j = 0; j < 10000000; j++) {
                char *buf = (char*)malloc(BLOCKSIZE);
                ssize_t size = read(fd, buf, BLOCKSIZE);
                //set to start from the beginning
                lseek(fd, 0, SEEK_SET);
                if (j == 10) {
                    printf("Child, pid = %d, open %s\n", getpid(), file_name);
                }
                free(buf);
            }
            close(fd);
            printf("Child %d exit\n", getpid());
            exit(0);
        }
    }*/
    if (getpid() == parent_pid) {
        //usleep(100000000);
        uint64_t start;
        uint64_t end;
        double elapsed;
        int fd = open("../Ignore_File/0", O_RDONLY|F_NOCACHE);
        for (i = 0; i < 100; i++) {
            char *buf = (char*)malloc(BLOCKSIZE);
            start = rdtsc();
            ssize_t size = read(fd, buf, BLOCKSIZE);
            end = rdtsc();
            elapsed += end - start;
            //reset to beginning of file
            lseek(fd, 0, SEEK_SET);
            free(buf);
        }
        elapsed /= 100.0 * 2.7;
        printf("Process_num: %d, time: %f\n", proc_num, elapsed);
    }
    return;
}
int main(int argc, const char* argv[]) {
    //Contention(20);

    // FILE *fp;
    // 	fp=fopen("FileCacheSize.csv","w+");
    // 	fprintf(fp,"logFileSize, time\n");
    unsigned long all_file_size[6] = {1073741824, 2147483648, 3221225472, 4294967296, 5368709120, 6442450944};

    int i = 0;
    /*
    for(i = 1; i <= 7; i++){

        unsigned long file_size = all_file_size[i-1];

        char full_path[200]= {0};
        char dir_path[50] = "/Users/Silvia/Desktop/BigFiles/Temp";
        char file_name[50] = {'\0'};
        sprintf(file_name, "%d", i);
        strcat(full_path, dir_path);
        strcat(full_path, file_name);

        //double overhead = FileCacheSize(full_path, file_size);
    }

    for(i = 0; i <= 10; i++){
        int file_size = pow(2,i) * MB;
        //printf("%d\n", file_size);
        char full_path[200]= {0};
        char dir_path[50] = "/Users/Silvia/Desktop/TempFiles/Temp";
        char file_name[50] = {'\0'};
        sprintf(file_name, "%d", i);
        strcat(full_path, dir_path);

        strcat(full_path, file_name);
        //SequentialRead(full_path, file_size);
        RandomRead(full_path, file_size);

    }*/

    //remote access
    for(i = 8; i <= 8; i++){
        int file_size = pow(2,i) * MB;
        //printf("%d\n", file_size);
        char full_path[200]= {0};
        sprintf(full_path, "/Volumes/TempFiles/temp%d", i);
        printf("%s\n",full_path);
        //printf("Sequential:\n");
        //SequentialRead(full_path, file_size);
        printf("Random:\n");
        RandomRead(full_path, file_size);

    }

    //fclose(fp);
    return 0;
}