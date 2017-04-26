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
    uint64_t start = 0;
    uint64_t end = 0;
    double sum = 0;
    for (int i = 0; i < 1000; i++) {
        switch (num) {
            case 0:
                start = rdtsc();
                TestArgument_0();
                end = rdtsc();
                break;
            case 1:
                start = rdtsc();
                TestArgument_1(1);
                end = rdtsc();
                break;
            case 2:
                start = rdtsc();
                TestArgument_2(1, 1);
                end = rdtsc();
                break;
            case 3:
                start = rdtsc();
                TestArgument_3(1, 1, 1);
                end = rdtsc();
                break;
            case 4:
                start = rdtsc();
                TestArgument_4(1, 1, 1, 1);
                end = rdtsc();
                break;
            case 5:
                start = rdtsc();
                TestArgument_5(1, 1, 1, 1, 1);
                end = rdtsc();
                break;
            case 6:
                start = rdtsc();
                TestArgument_6(1, 1, 1, 1, 1, 1);
                end = rdtsc();
                break;
            case 7:
                start = rdtsc();
                TestArgument_7(1, 1, 1, 1, 1, 1, 1);
                end = rdtsc();
                break;
            default:
                start = rdtsc();
                end = rdtsc();
        }
        sum += (end - start);
    }
    sum /= 1000.0;
    return sum;
}

//3: System call overhead
static double SystemOverhead() {
    uint64_t start;
    uint64_t end;
    //long double sum = 0;
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
    double total_time = 0;

    for(int i = 0; i < ITERATION; i++){
        uint64_t start, end;
        start = rdtsc();
        pid_t pid = fork();
        end = rdtsc(); //parent's end

        if (pid == 0){ // child
            exit(0);
        }
        else{		//parent
            //wait(NULL);
            //end = rdtsc();
            total_time += end - start;
        }
    }
    return total_time / ITERATION;
}

//5: Context switch time
uint64_t OneSwitchTime(int *fd) {
    uint64_t start;
    uint64_t end;
    uint64_t elapsed = 0;

    pid_t pid = fork();

    if (pid == 0){ 	//child
        end = rdtsc();
        write(fd[1], (void*)&end, sizeof(end));
        exit(0);
    }
    else{		//parent
        start = rdtsc();
        wait(NULL);
        read(fd[0], (void*)&end, sizeof(end));
    }
    if(end > start){
        elapsed = end - start;
    }
    return elapsed;
}

static double ProcessContextSwitchOverhead() {

    int fd[2];
    pipe(fd);
    uint64_t total_time = 0;
    int i = 0;
    while(i < ITERATION) {
        uint64_t elapsed = OneSwitchTime(fd);
        //printf("%llu\n", elapsed);
        if (elapsed > 0) {
            total_time += elapsed;
            i += 1;
        }
    }
    return (double)total_time/ (double)ITERATION;
}


static double PipeOverhead() {
    uint64_t total_time = 0;

    int i = 0;
    for (i = 0; i< ITERATION; i++){
        uint64_t  start, end, elapsed;
        int fd[2];
        pipe(fd);

        start = rdtsc();
        write(fd[1], &start, sizeof(start));
        read(fd[0], &start, sizeof(start));
        end = rdtsc();
        elapsed = end - start;
        total_time += elapsed;
    }
    return (double)total_time / ITERATION;
}


//Create kernel thread overhead
void *Task() {
    pthread_exit(0);
}
static double KernelOverhead() {
    pthread_t thread;
    uint64_t start;
    uint64_t end;
    uint64_t elapsed;
    start = rdtsc();
    pthread_create(&thread, NULL, Task, NULL);
    end = rdtsc();
    elapsed = (end - start);
    return elapsed;
}

//Context switch kernel thread overhead
static uint64_t thread_end;
static uint64_t thread_start;
static uint64_t cs_time;
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
static double CSKernelOverhead() {
    pthread_t t1;
    pthread_create(&t1, NULL, &Thread1, NULL);
    pthread_join(t1, NULL);
    cs_time = thread_end - thread_start;
    return cs_time;
}
int main(int argc, const char * argv[]) {
    double overhead;
    int i = 0;

    //Read time overhead
    overhead = ReadTimeOverhead();
    printf("Read Time overhead = %lf circles\n", overhead);

    //Read time overhead
    overhead = LoopOverhead();
    printf("Loop overhead = %lf circles\n", overhead);

    //Procedure call overhead
    for (i = 0; i < 8; i++) {
        overhead = ProcedureOverhead(i);
        printf("Procedure call overhead with %d arguments = %lf circles\n", i, overhead);
    }
    //System call overhead
    overhead = SystemOverhead();
    printf("System call overhead = %lf circles\n", overhead);

    //Task creation time
    overhead = TaskCreationTime();
    printf("Process creation overhead = %lf circles\n", overhead);

    //Context switch time
    //printf("%lf\n", PipeOverhead());
    overhead = ProcessContextSwitchOverhead()-PipeOverhead();
    printf("Process context switch overhead = %lf circles\n", overhead);

    //kernel thread
    overhead = KernelOverhead();
    printf("kernel thread overhead = %lf circles\n", overhead);

    //kernel thread context switch time
    overhead = CSKernelOverhead();
    printf("kernel thread context switch overhead = %lf circles\n", overhead);
    return 0;
}