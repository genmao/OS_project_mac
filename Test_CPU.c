#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>


#define ITERATION 100

static __inline__ uint64_t rdtsc(void) {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

// use assembly MACRO to read RDTSC to avoid procedure call overhead
#define read_start_time(hi,lo) \
asm volatile ("CPUID\n\t" \
"RDTSC\n\t" \
"mov %% edx, %0\n\t" \
"mov %% eax, %1\n\t": "=r" (hi), "=r" (lo):: "%rax","%rbx", "%rcx", "%rdx")

#define read_end_time(hi,lo) \
asm volatile ("RDTSC\n\t" \
"mov %% edx, %0\n\t" \
"mov %% eax, %1\n\t" \
"CPUID\n\t": "=r" (hi), "=r" (lo):: "%rax","%rbx", "%rcx", "%rdx")


//1: Measurement overhead
//1-1: Overhead of reading time
static double ReadTimeOverhead() {
    uint64_t start;
    uint64_t end;
    double val [1000] = {0};
    double sum = 0, std = 0;
    for (int i = 0; i < 1000; i++) {
        start = rdtsc();
        end = rdtsc();
        val[i] = end - start;
        sum += end - start;
    }
    sum /= 1000.0;

    // calculate std
    for (int i = 0; i<1000; i++){
        std += (sum-val[i])*(sum-val[i]);
    }
    std = sqrt(std/1000);
    printf("1000 iters: Read Time overhead std = %lf cycles\n", std);
    return sum;
}

//1-2: Overhead of using a loop to measure many iterations of an operation
static double LoopOverhead() {
    uint64_t start;
    uint64_t end;
    double val [1000] = {0};
    long double sum = 0, std = 0;
    int i,j;
    for (int i = 0; i < 1000; i++) {
        start = rdtsc();
        for (int j = 0; j < 10000; j++) {
            //Empty loop, the only operation is inc(j)
        }
        end = rdtsc();
        val[i] = end - start;
        sum += (end - start) / 10000.0;
    }
    sum /= 1000.0;

    // calculate std
    for (int i = 0; i<1000; i++){
        std += (sum-val[i])*(sum-val[i]);
    }
    std = sqrt(std/1000);
    printf("1000 iters: loop overhead std = %lf cycles\n", std);

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
    double sum = 0, std = 0;
    double val[1000] = {0};
    for (int i = 0; i < 1000; i++) {
        switch (num) {
            case 0:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                    TestArgument_0();
                }
                end = rdtsc();
                break;
            case 1:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                    TestArgument_1(1);
                }
                end = rdtsc();
                break;
            case 2:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                    TestArgument_2(1, 1);
                }
                end = rdtsc();
                break;
            case 3:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                    TestArgument_3(1, 1, 1);
                }
                end = rdtsc();
                break;
            case 4:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                    TestArgument_4(1, 1, 1, 1);
                }
                end = rdtsc();
                break;
            case 5:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                    TestArgument_5(1, 1, 1, 1, 1);
                }
                end = rdtsc();
                break;
            case 6:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                    TestArgument_6(1, 1, 1, 1, 1, 1);
                }
                end = rdtsc();
                break;
            case 7:
                start = rdtsc();
                for(int j=0;j<100;j++){
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                    TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                }
                end = rdtsc();
                break;
            default:
                start = rdtsc();
                end = rdtsc();
        }
        sum += (end - start) / 1000.0;
        val[i] = (end - start) / 1000.0;
    }
    sum /= 1000.0;

    // calculate std
    for (int i = 0; i<1000; i++){
        std += (sum-val[i])*(sum-val[i]);
    }
    std = sqrt(std/1000);
    printf("Procedure call overhead with %d paras std = %lf cycles\n", num, std);
    return sum;
}

//3: System call overhead
static double SystemOverhead() {
    uint64_t start;
    uint64_t end;
    double sum = 0, std = 0;
    double val[1000] = {0};
    for (int i = 0; i < 1000; i++) {
        start = rdtsc();
        syscall(SYS_getpid);
        end = rdtsc();
        sum += end - start;
        val[i] = end - start;
    }
    sum /= 1000.0;
    // calculate std
    for (int i = 0; i<1000; i++){
        std += (sum-val[i])*(sum-val[i]);
    }
    std = sqrt(std/1000);
    printf("System call overhead std = %lf cycles\n", std);
    return sum;
}

//4: Task creation time
static double TaskCreationTime() {
    uint64_t start, end;
    double sum = 0, std = 0;
    double val[1000] = {0};
    //start = rdtsc();
    for(int i = 0; i < 100; i++){
        start = rdtsc();
        pid_t pid = fork();
        end = rdtsc();
        if (pid == 0){
            exit(0);
        }
            // parent process
        else{
            sum += end - start;
            val[i] = end - start;
        }

    }
    //end = rdtsc();
    sum /= 100.0;
    for (int i = 0; i<100; i++){
        std += (sum-val[i])*(sum-val[i]);
    }
    std = sqrt(std/100);
    printf("Process creation overhead std = %lf cycles\n", std);
    return sum;

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
    uint64_t val[100] = {0};
    double std = 0;
    while(i < 100) {
        uint64_t switchtime = SwitchTime(fd);
        if (switchtime > 0) {
            sum += switchtime;
            val[i] = switchtime;
            i++;
        }
    }
    double mean = (double)sum/ 100.0;
    for(int i=0; i<100; i++){
        std += (mean-(double)val[i])*(mean-(double)val[i]);
    }
    std = sqrt(std/100);
    printf("Process context switch overhead std = %lf cycles\n", std);
    return mean;

}


static double PipeAndRWOverhead() {
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

//1-4-2 Kernel Switch time
static double KernelOverhead() {
    uint64_t start, end, sum = 0;
    double std = 0;
    double val[100] = {0};
    for(int i=0; i<100; i++){
        pthread_t thread;
        start = rdtsc();
        pthread_create(&thread, NULL, Task, NULL);
        pthread_join(thread,NULL);
        end = rdtsc();
        sum += end - start;
        val[i] = end - start;
    }

    double mean = (double)sum/ 100.0;
    for(int i=0; i<100; i++){
        std += (mean-val[i])*(mean-val[i]);
    }
    std = sqrt(std/100);
    printf("Kernel thread creation overhead std = %lf cycles\n", std);
    return mean;
}

//Context switch kernel thread overhead
static uint64_t thread_end;
static uint64_t thread_start;
static uint64_t sum;
static uint64_t val[100];
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
    for(int i=0;i<100;i++){
        pthread_t t1;
        pthread_create(&t1, NULL, &Thread1, NULL);
        pthread_join(t1, NULL);
        sum += thread_end - thread_start;
        val[i] = thread_end - thread_start;
    }

    double mean = (double)sum/ 100.0;
    double std = 0;
    for(int i=0; i<100; i++){
        std += (mean-(double)val[i])*(mean-(double)val[i]);
    }
    std = sqrt(std/100);
    printf("kernel thread context switch overhead std = %lf cycles\n", std);
    return mean;
}

int main(int argc, const char * argv[]) {
    double overhead;

    //Read time overhead
    overhead = ReadTimeOverhead();
    printf("Read Time overhead Ave. = %lf cycles\n", overhead);

    //Read time overhead
    overhead = LoopOverhead();
    printf("Loop overhead Ave. = %lf cycles\n", overhead);

    //Procedure call overhead
    for (int i = 0; i < 8; i++) {
        overhead = ProcedureOverhead(i);
        printf("Procedure call overhead with %d arguments Ave. = %lf cycles\n", i, overhead);
    }
    //System call overhead
    overhead = SystemOverhead();
    printf("System call overhead Ave. = %lf cycles\n", overhead);

    //Task creation time
    overhead = TaskCreationTime();
    printf("Process creation overhead Ave. = %lf cycles\n", overhead);

    //kernel thread
    overhead = KernelOverhead();
    printf("kernel thread creation overhead Ave. = %lf cycles\n", overhead);

    //Context switch time between process
    //printf("%lf\n", PipeOverhead());
    // Since we have 2 context switch, we need to devide by 2?
    overhead = (ContextSwitchOverhead()-PipeAndRWOverhead()) ;
    printf("Process context switch overhead Ave. = %lf cycles\n", overhead);

    //Context switch time between kernel
    overhead = ContextSwitchTimeKernel();
    printf("kernel thread context switch overhead Ave. = %lf cycles\n", overhead);
    return 0;
}
