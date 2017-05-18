#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>

#define NANOSECONDS_PER_SECOND 1e9
#define CYCLES_PER_SECOND 2.9e9
#define MB 1048576
#define ITERATION 1000
#define MIN_INTERVAL 8
#define MAX_INTERVAL 2048
#define MIN_EXPANDTIME 8
#define MAX_EXPANDTIME 26
#define RamBusSize 64

static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi)<<32);
}

/**
 * GenerateRandomArray() will generate a random array of chars, of which the length is 2^(expand_times)
 * */
int8_t * GenerateRandomArray(int expand_times) {
    time_t t;
    srand((unsigned) time(&t));
    uint64_t array_size = (uint64_t)(pow(2.0, expand_times * 1.0));
    int8_t * random_array = (int8_t*) malloc(sizeof(int) * array_size);
    for (uint64_t i = 0; i < array_size; i++) {
        *(random_array + i) = (int8_t)(rand() % 128);
    }
    return random_array;
}


/**
 * InitializeMemory() will initialize a piece of free memory
 */
void InitializeMemory() {
    int8_t *temp = GenerateRandomArray(30);
    free(temp);
}


/**
 * MemoryAccessTime() will measure the latency for individual integer accesses to main memory
 */
void MemoryAccessTime(int interval, int expand_times){
    uint64_t array_size = (uint64_t)pow(2.0, expand_times * 1.0);
    uint64_t start, end;
    // Create jumping lookup table
    int8_t ** head = malloc(sizeof(int8_t *) * array_size);
    for (uint64_t i = 0; i < array_size ; i += interval) {
        uint64_t index = (i + interval) % array_size;
        head[i] = (int8_t *) &head[index];
    }
    // Calculate memory access time, as number of cycles
    int8_t ** p = head;
    uint64_t count = array_size / interval + 1;
    start = rdtsc();
    for (uint64_t i = 0; i < ITERATION; i++) {
        for (uint64_t j = 0; j < count; j++) {
            p = (int8_t **) *p;
        }
    }
    end = rdtsc();
    double cycle_count = end - start;
    printf("Time for each access is %lf cycles. \n",
           cycle_count / (ITERATION * count));
    free(head);
    return;
}

/**
 * ReadingBandwidth() will return the bandwidth in terms of MB/s
 */
double ReadingBandwidth(int expand_times) {
    uint64_t start, end, count;
    int8_t * random_array = GenerateRandomArray(expand_times);
    uint64_t array_size = (uint64_t)pow(2.0, expand_times * 1.0);
    start = rdtsc();
    int8_t tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;
    // unroll loop for more accurate results
    for (uint64_t i = 0; i < array_size; i += (RamBusSize*8)) {
        tmp0 = random_array[i+RamBusSize*0];
        tmp1 = random_array[i+RamBusSize*1];
        tmp2 = random_array[i+RamBusSize*2];
        tmp3 = random_array[i+RamBusSize*3];
        tmp4 = random_array[i+RamBusSize*4];
        tmp5 = random_array[i+RamBusSize*5];
        tmp6 = random_array[i+RamBusSize*6];
        tmp7 = random_array[i+RamBusSize*7];
    }
    end = rdtsc();
    count = end - start;
    //MB per second
    return array_size * sizeof(int8_t) / count * CYCLES_PER_SECOND / MB;
}

/**
 * WritingBandwidth() will return the bandwidth in terms of MB/s
 */
double WritingBandwidth(int expand_times) {
    uint64_t start, end, count;
    int8_t * random_array = GenerateRandomArray(expand_times);
    uint64_t array_size = (uint64_t)pow(2.0, expand_times * 1.0);
    start = rdtsc();
    // unroll loop for more accurate results
    for (uint64_t i = 0; i < array_size; i += (RamBusSize*8)) {
        random_array[i+RamBusSize*0] = 1;
        random_array[i+RamBusSize*1] = 2;
        random_array[i+RamBusSize*2] = 3;
        random_array[i+RamBusSize*3] = 4;
        random_array[i+RamBusSize*4] = 5;
        random_array[i+RamBusSize*5] = 6;
        random_array[i+RamBusSize*6] = 7;
        random_array[i+RamBusSize*7] = 8;
    }
    end = rdtsc();
    count = end - start;
    return array_size * sizeof(int8_t) / count * CYCLES_PER_SECOND / MB;
}

/**
 * RAMBandwidth() will measure the maximum bandwidth in terms of MB/s
 */
void RamBandwidth(char io_flag) {
    double bandwidth = 0, bandwidth_max = -1;
    InitializeMemory();
    if (io_flag == 'r') {
        printf("Reading RAM bandwidth: \n");
        for (int i = 22; i <= 30; i++) {
            bandwidth = ReadingBandwidth(i);
            if (bandwidth > bandwidth_max) {
                bandwidth_max = bandwidth;
            }
            printf("%d: %lf MB/s. \n", i, bandwidth);
        }
        printf("The RAM reading bandwidth is %lf MB/s. \n",bandwidth_max);
    }
    else if (io_flag == 'w') {
        printf("Writing RAM bandwidth: \n");
        for (int i = 22; i <= 30; i++) {
            bandwidth = WritingBandwidth(i);
            if (bandwidth > bandwidth_max) {
                bandwidth_max = bandwidth;
            }
            printf("%d: %lf MB/s. \n", i, bandwidth);
        }
        printf("The RAM writing bandwidth is %lf MB/s. \n",bandwidth_max);
    }
    return;
}

/**
 * PageFaultServiceTime() will measure the page fault service time
 */
static void PageFaultServiceTime(){
    uint64_t start, end;
    int ITER = 100;
    double sum = 0, std = 0;
    // Uncomment the following code if running this code for the first time
    system("dd if=/dev/urandom of=file.txt bs=1048576 count=1024");
    int fd = open("file.txt",O_RDONLY);
    uint64_t stride = (uint64_t) pow(2,23);  // 8MB-stride, larger than any cache size
    char ch;
    int aggregate = 0;
    char* file_array;
    file_array = (char*) mmap(NULL, (uint64_t)(pow(2,30)), PROT_READ, MAP_SHARED, fd, 0);
    ch = file_array[0]; // read file_array into memory
    start = rdtsc();
    for (int i=0; i<ITER; i++){
	    ch = file_array[(i+1) * stride];
        aggregate = ch+1;
    }
    end = rdtsc();
    
    sum = (double) (end - start)/ITER;
    printf("Average Page Fault Service Time is: %f ns. \n",sum / CYCLES_PER_SECOND * NANOSECONDS_PER_SECOND);

}

int main(int argc, const char * argv[]) {
    printf("Clearing Memory and SSD caches...\n");
    system("sudo purge"); // clear memory and SSD cache
    printf("Cleared!\n");
    for(int interval = MIN_INTERVAL; interval <= MAX_INTERVAL; interval *= 2){
        printf("interval = %d\n", interval);
        for(int expand_times = MIN_EXPANDTIME; expand_times <= MAX_EXPANDTIME; expand_times++){
            MemoryAccessTime(interval, expand_times);
        }
        printf("\n");
    }
    RamBandwidth('r');
    RamBandwidth('w');

    PageFaultServiceTime();
    return 0;
}
