#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>


#define ITERATION 100
static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

//1: Measurement overhead
//1-1: Overhead of reading time
static double ReadTimeOverhead() {
    uint64_t start;
    uint64_t end;
    double sum = 0;
    for (int i = 0; i < 1000; i++) {
        start = rdtsc();
        end = rdtsc();
        sum += end - start;
    }
    sum /= 1000.0;
    return sum;
}

//1-2: Overhead of using a loop to measure many iterations of an operation
static double LoopOverhead() {
    uint64_t start;
    uint64_t end;
    long double sum = 0;
    int i,j;
    for (int i = 0; i < 1000; i++) {
        start = rdtsc();
        for (int j = 0; j < 10000; j++) {
            //Empty loop, the only operation is inc(j)
        }
        end = rdtsc();
        sum += (end - start) / 10000.0;
    }
    sum /= 1000.0;
    return sum;
}

//2: Procedure call overhead
static __attribute__ ((noinline)) void TestArgument_0() {}
static __attribute__ ((noinline)) void TestArgument_1(int a) {}
static __attribute__ ((noinline)) void TestArgument_2(int a, int b) {}
static __attribute__ ((noinline)) void TestArgument_3(int a, int b, int c) {}
static __attribute__ ((noinline)) void TestArgument_4(int a, int b, int c, int d) {}
static __attribute__ ((noinline)) void TestArgument_5(int a, int b, int c, int d, int e) {}
static __attribute__ ((noinline)) void TestArgument_6(int a, int b, int c, int d, int e, int f) {}
static __attribute__ ((noinline)) void TestArgument_7(int a, int b, int c, int d, int e, int f, int g) {}

static double ProcedureOverhead(int num) {
    uint64_t start, end;
    double sum = 0;
    for (int i = 0; i < 1000; i++) {
        switch (num) {
            case 0:
                start = rdtsc();
                TestArgument_0();
                TestArgument_0();
                TestArgument_0();
                TestArgument_0();
                TestArgument_0();
                end = rdtsc();
                break;
            case 1:
                start = rdtsc();
                TestArgument_1(1);
                TestArgument_1(1);
                TestArgument_1(1);
                TestArgument_1(1);
                TestArgument_1(1);
                end = rdtsc();
                break;
            case 2:
                start = rdtsc();
                TestArgument_2(1, 1);
                TestArgument_2(1, 1);
                TestArgument_2(1, 1);
                TestArgument_2(1, 1);
                TestArgument_2(1, 1);
                end = rdtsc();
                break;
            case 3:
                start = rdtsc();
                TestArgument_3(1, 1, 1);
                TestArgument_3(1, 1, 1);
                TestArgument_3(1, 1, 1);
                TestArgument_3(1, 1, 1);
                TestArgument_3(1, 1, 1);
                end = rdtsc();
                break;
            case 4:
                start = rdtsc();
                TestArgument_4(1, 1, 1, 1);
                TestArgument_4(1, 1, 1, 1);
                TestArgument_4(1, 1, 1, 1);
                TestArgument_4(1, 1, 1, 1);
                TestArgument_4(1, 1, 1, 1);
                end = rdtsc();
                break;
            case 5:
                start = rdtsc();
                TestArgument_5(1, 1, 1, 1, 1);
                TestArgument_5(1, 1, 1, 1, 1);
                TestArgument_5(1, 1, 1, 1, 1);
                TestArgument_5(1, 1, 1, 1, 1);
                TestArgument_5(1, 1, 1, 1, 1);
                end = rdtsc();
                break;
            case 6:
                start = rdtsc();
                TestArgument_6(1, 1, 1, 1, 1, 1);
                TestArgument_6(1, 1, 1, 1, 1, 1);
                TestArgument_6(1, 1, 1, 1, 1, 1);
                TestArgument_6(1, 1, 1, 1, 1, 1);
                TestArgument_6(1, 1, 1, 1, 1, 1);
                end = rdtsc();
                break;
            case 7:
                start = rdtsc();
                TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                end = rdtsc();
                break;
            default:
                start = rdtsc();
                end = rdtsc();
        }
        sum += (end - start) / 5.0;
    }
    sum /= 1000.0;
    return sum;
}

//3: System call overhead
static double SystemOverhead() {
    uint64_t start;
    uint64_t end;
    double sum = 0;
    for (int i = 0; i < 1000; i++) {
        start = rdtsc();
        syscall(SYS_getpid);
        end = rdtsc();
        sum += end - start;
    }
    sum /= 1000.0;
    return sum;
}

//4: Task creation time
static double TaskCreationTime() {
    uint64_t start, end;
    double sum = 0;
    for(int i = 0; i < 100; i++){
        start = rdtsc();
        pid_t pid = fork();
        end = rdtsc();
        if (pid == 0){
            exit(0);
        }
        else{
            sum += end - start;
        }
    }
    return sum / 100.0;
}

//5: Context switch time
uint64_t SwitchTime(int *fd) {
    uint64_t start, end;
    uint64_t sum = 0;
    pid_t pid = fork();
    if (pid == 0){
        end = rdtsc();
        write(fd[1], (void*)&end, sizeof(end));
        exit(0);
    }
    else{
        start = rdtsc();
        wait(NULL);
        read(fd[0], (void*)&end, sizeof(end));
    }
    if(end > start){
        sum = end - start;
    }
    return sum;
}

static double ContextSwitchOverhead() {
    int fd[2];
    pipe(fd);
    uint64_t sum = 0;
    int i = 0;
    while(i < 100) {
        uint64_t number = SwitchTime(fd);
        if (number > 0) {
            sum += number;
            i++;
        }
    }
    return (double)sum/ 100.0;
}


static double PipeOverhead() {
    uint64_t sum = 0;
    for (int i = 0; i < 100; i++){
        uint64_t start, end;
        int fd[2];
        pipe(fd);
        start = rdtsc();
        write(fd[1], &start, sizeof(start));
        read(fd[0], &start, sizeof(start));
        end = rdtsc();
        sum += end - start;
    }
    return (double)sum / 100.0;
}


//Create kernel thread overhead
void *Task() {
    pthread_exit(0);
}

//1-5-2 Kernel Switch time
static double KernelOverhead() {
    pthread_t thread;
    uint64_t start, end;
    double sum;
    start = rdtsc();
    pthread_create(&thread, NULL, Task, NULL);
    end = rdtsc();
    sum = end - start;
    return sum;
}

//Context switch kernel thread overhead
static uint64_t thread_end;
static uint64_t thread_start;
static uint64_t sum;
void *Thread2() {
    thread_end = rdtsc();
    pthread_exit(0);
}

void *Thread1() {
    pthread_t t2;
    pthread_create(&t2, NULL, &Thread2, NULL);
    thread_start = rdtsc();
    pthread_join(t2, NULL);
    pthread_exit(0);
}
static double ContextSwitchTimeKernel() {
    pthread_t t1;
    pthread_create(&t1, NULL, &Thread1, NULL);
    pthread_join(t1, NULL);
    sum = thread_end - thread_start;
    return sum;
}

int main(int argc, const char * argv[]) {
    double overhead;

    //Read time overhead
    overhead = ReadTimeOverhead();
    printf("Read Time overhead = %lf cycles\n", overhead);

    //Read time overhead
    overhead = LoopOverhead();
    printf("Loop overhead = %lf cycles\n", overhead);

    //Procedure call overhead
    for (int i = 0; i < 10; i++) {
        overhead = ProcedureOverhead(i);
        printf("Procedure call overhead with %d arguments = %lf cycles\n", i, overhead);
    }
    //System call overhead
    overhead = SystemOverhead();
    printf("System call overhead = %lf cycles\n", overhead);

    //Task creation time
    overhead = TaskCreationTime();
    printf("Process creation overhead = %lf cycles\n", overhead);

    //Context switch time between process
    //printf("%lf\n", PipeOverhead());
    overhead = ContextSwitchOverhead()-PipeOverhead();
    printf("Process context switch overhead = %lf cycles\n", overhead);

    //kernel thread
    overhead = KernelOverhead();
    printf("kernel thread overhead = %lf cycles\n", overhead);

    //Context switch time between kernel
    overhead = ContextSwitchTimeKernel();
    printf("kernel thread context switch overhead = %lf cycles\n", overhead);
    return 0;
}