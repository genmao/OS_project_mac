
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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
#define ContFileSize 512  //MB

static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi)<<32);
}

void CreateFiles(){
    // Create files for FileCacheSize measurement: file1-7 1-7GB
    system("dd if=/dev/urandom of=file1 bs=1048576 count=1024");
    system("dd if=/dev/urandom of=file2 bs=1048576 count=2048");
    system("dd if=/dev/urandom of=file3 bs=1048576 count=3072");
    system("dd if=/dev/urandom of=file4 bs=1048576 count=4096");
    system("dd if=/dev/urandom of=file5 bs=1048576 count=5120");
    system("dd if=/dev/urandom of=file6 bs=1048576 count=6144");
    system("dd if=/dev/urandom of=file7 bs=1048576 count=7168");
    
    // Create files for Seq/Rand reading: read1-10 1-512MB
    system("dd if=/dev/urandom of=read1 bs=1048576 count=1");
    system("dd if=/dev/urandom of=read2 bs=1048576 count=2");
    system("dd if=/dev/urandom of=read3 bs=1048576 count=4");
    system("dd if=/dev/urandom of=read4 bs=1048576 count=8");
    system("dd if=/dev/urandom of=read5 bs=1048576 count=16");
    system("dd if=/dev/urandom of=read6 bs=1048576 count=32");
    system("dd if=/dev/urandom of=read7 bs=1048576 count=64");
    system("dd if=/dev/urandom of=read8 bs=1048576 count=128");
    system("dd if=/dev/urandom of=read9 bs=1048576 count=256");
    system("dd if=/dev/urandom of=read10 bs=1048576 count=512");
    
    // Create files for contention test
    system("dd if=/dev/urandom of=cont1 bs=1048576 count=512");
    system("dd if=/dev/urandom of=cont2 bs=1048576 count=512");
    system("dd if=/dev/urandom of=cont3 bs=1048576 count=512");
    system("dd if=/dev/urandom of=cont4 bs=1048576 count=512");
    system("dd if=/dev/urandom of=cont5 bs=1048576 count=512");
    system("dd if=/dev/urandom of=cont6 bs=1048576 count=512");
    system("dd if=/dev/urandom of=cont7 bs=1048576 count=512");
    system("dd if=/dev/urandom of=cont8 bs=1048576 count=512");
    
}

void FileCacheSize(){
    uint64_t start = 0, end = 0, total_time = 0, mean_time = 0;
    int fd;
    char buffer[BLOCKSIZE];

    for(int file_num = 1; file_num < 8; file_num++){
        char filename[128];
        int loops = GB*file_num/BLOCKSIZE;
        sprintf(filename,"file%d",file_num);
        // warm-up read
        fd = open(filename,O_RDONLY);
        for(int i=0;i<loops;i++){
            read(fd,buffer,BLOCKSIZE);
        }
        close(fd);
        
        // measure reading time
        for(int iter=0;iter<ITERATION;iter++){
            fd = open(filename,O_RDONLY);
            start = rdtsc();
            for(int i=0;i<loops;i++){
                read(fd,buffer,BLOCKSIZE);
            }
            end = rdtsc();
            total_time += (end-start);
            close(fd);
        }
        
        mean_time = total_time/ITERATION;
        printf("Reading %s takes %llu cycles!\n",filename,mean_time);
    }
}

void SequentialAccess(){
    uint64_t start = 0, end = 0, total_time = 0, ave_per_block_time = 0;
    int fd;
    char buffer[BLOCKSIZE];

    for(int file_num = 1; file_num < 10; file_num++){

        char filename[128];
        int loops = MB*pow(2,file_num-1)/BLOCKSIZE;
        sprintf(filename,"/Users/genmaoshi/Downloads/filetest/read%d",file_num);
        // measure reading time
        for(int iter=0;iter<ITERATION;iter++){
            fd = open(filename,O_RDONLY);
            // use raw device interface by marking F_NOCACHE flag
            fcntl(fd, F_NOCACHE);
            start = rdtsc();
            for(int i=0;i<loops;i++){
                read(fd,buffer,BLOCKSIZE);
            }
            end = rdtsc();
            total_time += (end-start);
            close(fd);
        }
        
        ave_per_block_time = total_time/ITERATION/loops;
        printf("Sequential Reading %s takes average per block time %llu cycles, log %f!\n",filename,ave_per_block_time,log2(ave_per_block_time));
    }
}

void RandomAccess(){
    uint64_t start = 0, end = 0, total_time = 0, ave_per_block_time = 0;
    int fd;
    char buffer[BLOCKSIZE];
    
    system("sudo purge");
    
    for(int file_num = 1; file_num < 10; file_num++){
        char filename[128];
        int loops = MB*pow(2,file_num-1)/BLOCKSIZE;
        sprintf(filename,"/Users/genmaoshi/Downloads/filetest/read%d",file_num);
        
        // measure reading time
        for(int iter=0;iter<ITERATION;iter++){
            fd = open(filename,O_RDONLY);
            // use raw device interface by marking F_NOCACHE flag
            fcntl(fd, F_NOCACHE);
            for(int i=0;i<loops;i++){
                int offset = rand()%loops*BLOCKSIZE;  // random access
                start = rdtsc();
                pread(fd,buffer,BLOCKSIZE,offset);  // pread allows us can read any location in the file
                end = rdtsc();
                total_time += (end-start);
            }
            close(fd);
        }
        
        ave_per_block_time = total_time/ITERATION/loops;
        printf("Random Reading %s takes average per block time %llu cycles, log %f!\n",filename,ave_per_block_time,log2(ave_per_block_time));
    }
}

void Contention(int process_nums, int filesize){
    uint64_t start = 0, end = 0, total_time = 0, ave_per_block_time = 0;
    char buffer[BLOCKSIZE];
    int fd;
    int self_id = 0;
    
    for(int id = 0; id < process_nums; id++){
        pid_t pid = fork();
        if (pid != 0){
            self_id = id;
            break;
        }
        else if (id+1 == process_nums)
            return;
    }
    
    // Different processes read differerent files
    char filename[128];
    int loops = filesize/BLOCKSIZE;
    sprintf(filename,"/Users/genmaoshi/Downloads/filetest/cont%d",self_id+1);
    fd = open(filename,O_RDONLY);
    // use raw device interface by marking F_NOCACHE flag
    fcntl(fd, F_NOCACHE);
    start = rdtsc();
    for(int i=0;i<loops;i++){
        read(fd,buffer,BLOCKSIZE);
    }
    end = rdtsc();
    total_time += (end-start);
    close(fd);
    
    ave_per_block_time = total_time/loops;
    printf("Process %d takes average per block time %llu cycles\n",self_id,ave_per_block_time);
    printf("Process number = %d\n", process_nums);
    if (self_id!=0)
        exit(0);
    
}


int main() {
//    CreateFiles();    //Comments after creating files
    printf("Clearing caches...\n");
    system("sudo purge"); // clear memory and SSD cache
    printf("Cleared!\n");
    //FileCacheSize();
    SequentialAccess();
    RandomAccess();
    
    
//    for (int process_nums=1; process_nums<=8; process_nums++){
//        system("sudo purge"); // clear memory and SSD cache
//        Contention(process_nums, ContFileSize*MB);
//    }

    return 0;
}
