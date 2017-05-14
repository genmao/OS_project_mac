#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define CacheSize 64
#define nanosecond pow(10.0, -9)
#define MB 1048576
#define CPU_cycle 2.9
#define ITERATION 1000
#define MIN_STRIDE 8
#define MAX_STRIDE 2048
#define MIN_POWER 8
#define MAX_POWER 26
#define PageSize 4096

static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)lo) | (((uint64_t)hi)<<32);
}


/**
 * GenerateRandomArray() will generate a random array of chars, of which the length is 2^(expand_times)
 * */
char* GenerateRandomArray(int expand_times) {
    time_t t;
    srand((unsigned) time(&t));
    uint64_t array_size = (uint64_t)(pow(2.0, expand_times * 1.0));
    char* random_array = (char*) malloc(sizeof(char) * array_size);
    for (uint64_t i = 0; i < array_size; i++) {
        *(random_array + i) = (char)('a' + rand() % 26);
    }
    return random_array;
}


/**
 * InitializeMemory() will initialize a piece of free memory
 */
void InitializeMemory() {
    char *temp = GenerateRandomArray(30);
    free(temp);
}


/**
 * MemoryAccessTime() will measure the latency for individual integer accesses to main memory
 */
void MemoryAccessTime(int interval, int expand_times){
    uint64_t array_size = (uint64_t)pow(2.0, expand_times * 1.0);
    uint64_t start, end;
    // Create jumping lookup table
    char ** head = malloc(sizeof(char*) * array_size);
    for (uint64_t i = 0; i < array_size ; i += interval) {
        uint64_t index = (i + interval) % array_size;
        head[i] = (char *) &head[index];
    }

    // Calculate memory access time, as number of cycles
    char ** p = head;
    uint64_t count = array_size / interval + 1;
    start = rdtsc();
    for (uint64_t i = 0; i < ITERATION; i++) {
        for (uint64_t j = 0; j < count; j++) {
            p = (char**) *p;
        }
    }
    end = rdtsc();
    double cycle_count = end - start;
    printf("Number of cycles for each access is %lf. \n", cycle_count / (ITERATION * count));
    free(head);
    return;

}

double ReadingBandwidth(int expand_times) {
    uint64_t start, end, count;
    char* random_array = GenerateRandomArray(expand_times);
    uint64_t array_size = (uint64_t)pow(2.0, expand_times * 1.0);
    start = rdtsc();
    char dump_char;
    for (uint64_t i = 0; i < array_size; i += CacheSize) {
        dump_char = random_array[i];
    }
    end = rdtsc();
    count = end - start;
    return (double)array_size / ((double)count);
}

double WritingBandwidth(int expand_times) {
    uint64_t start, end, count;
    char* random_array = GenerateRandomArray(expand_times);
    uint64_t array_size = (uint64_t)pow(2.0, expand_times * 1.0);
    start = rdtsc();
    for (uint64_t i = 0; i < array_size; i += CacheSize) {
        random_array[i] = 'a';
    }
    end = rdtsc();
    count = end - start;
    return (double)array_size / ((double)count);
}

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
            printf("%d:%lf cycles/s. \n", i, bandwidth);
        }
    }
    else if (io_flag == 'w') {
        printf("Writing RAM bandwidth: \n");
        for (int i = 22; i <= 30; i++) {
            bandwidth = WritingBandwidth(i);
            if (bandwidth > bandwidth_max) {
                bandwidth_max = bandwidth;
            }
            printf("%d:%lf cycles/s. \n", i, bandwidth);
        }
    }
    printf("The maximum width is %lf cycles/s. \n",bandwidth_max);
    return;
}

void PageFaultServiceTime() {
    uint64_t start, end, count;
    char *arr = GenerateRandomArray(35);
    uint64_t arr_size = pow(2,35);
    char dummy_char;
    printf("Start measuring Page Fault Service time: \n");
    for (uint64_t i = 0; i < ITERATION; i++) {
        start = rdtsc();
        dummy_char = arr[(i + 1) * PageSize];
        end = rdtsc();
        count += end - start;
        if (i % 100 == 0) {
            double avg = count * 1.0 / 100;
            printf("Num of cycles for page fault service: %lf\n", avg);
            count = 0;
        }
    }
    return;
}

int main(int argc, const char * argv[]) {

    int stride = 0, power = 0;
    for(stride = MIN_STRIDE; stride <= MAX_STRIDE; stride *= 2){
        printf("stride = %d\n", stride);
        for(power = MIN_POWER; power <= MAX_POWER; power++){
            MemoryAccessTime(stride, power);
        }
        printf("\n");
    }

    RamBandwidth('r');
    RamBandwidth('w');
    PageFaultServiceTime();
    return 0;
}