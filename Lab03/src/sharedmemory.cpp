#include <iostream>
#include <sys/shm.h>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <fstream>
#include <semaphore.h>
#include <algorithm>
#include<cstring>
#include<fcntl.h>
#include <chrono>

#define LINE_SIZE 130000
#define SIZE 1200
#define NUM_THREADS 4

typedef struct {
    char data[LINE_SIZE][SIZE];
    int l;
} SharedData;

typedef struct{
    char * now;
    char * search_word;
    int len;
} param;

typedef struct {
    char ans[LINE_SIZE][SIZE];
    int index;
} r;

r result;
pthread_mutex_t mutex;

void reduce(SharedData *shared_data);
void * map(void *arg);

int main(int argc, char *argv[]) {
    auto start = std::chrono::high_resolution_clock::now();

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <file_path> <word_to_search> <file_out>" << std::endl;
        return 1;
    }


    char *file_path = argv[1];
    char *word_to_search = argv[2];
    char *file_out = argv[3];

    sem_t* a = sem_open("/a",O_CREAT|O_RDWR,0666,0);
    sem_t* b = sem_open("/b",O_CREAT|O_RDWR,0666,0);

    // 创建共享内存
    key_t key = ftok("./shmfile", 66);
    int shm_id = shmget(key, sizeof(SharedData), 0666 | IPC_CREAT);
    SharedData *shared_data = (SharedData *)shmat(shm_id, (void *)0, 0);

    pthread_mutex_init(&mutex, NULL);

    pid_t child_pid = fork();
    if(child_pid == -1){
        perror("fork");
        return 1;
    } else if(child_pid == 0 ){
        pthread_t threads[NUM_THREADS];
//        std::ofstream fileout(file_out, std::ios::out | std::ios::binary);

        sem_wait(a);
//            for(int k = 0; k < shared_data->l; k++){
//                fileout<<shared_data->data[k];
//                fileout.flush();
//            }

        param  nowparam[NUM_THREADS];
        for (int i = 0; i < NUM_THREADS; i++) {
            int len = shared_data->l / NUM_THREADS;

            if(i != NUM_THREADS - 1){
                nowparam[i].len = len;
            }else nowparam[i].len = shared_data->l - (NUM_THREADS - 1)*len;

            nowparam[i].now = shared_data->data[i * len];

            nowparam[i].search_word = word_to_search;

            if (pthread_create(&threads[i], NULL, map, &nowparam[i]) != 0) {
                perror("pthread_create");
                return 1;
            }
        }
        for (int i = 0; i < NUM_THREADS; ++i) {
            if (pthread_join(threads[i], NULL) != 0) {
                perror("pthread_join");
                return 1;
            }
        }


        memset(shared_data->data, 0, sizeof(shared_data->data));

//        shared_data->l = result.index;
//        for (int i = 0; i < result.index && i < LINE_SIZE; ++i) {
//            strncpy(shared_data->data[i], result.ans[i], SIZE - 1);
//        }

        reduce(shared_data);
        sem_post(b);
//        fileout.close();
        shmdt(shared_data);
        exit(0);
    } else if(child_pid > 0){
        std::ofstream fileout(file_out, std::ios::out | std::ios::binary);
        FILE *file = fopen(file_path, "r");

        int cnt = 0;
        char line[SIZE];
        while(std::fgets(line, sizeof(line), file)){
            strncpy(shared_data->data[cnt++], line, SIZE - 1);
        }
        shared_data->l = cnt;
        sem_post(a);

        sem_wait(b);
        for (int i = 0; i < shared_data->l; ++i) {
            fileout << shared_data->data[i];
            fileout.flush();
        }

        shmdt(shared_data);
        shmctl(shm_id, IPC_RMID, NULL); // 标记共享内存可以被删除
        sem_close(a); // 关闭信号量
        sem_unlink("/a"); // 移除信号量
        sem_close(b); // 关闭信号量
        sem_unlink("/b"); // 移除信号量
        pthread_mutex_destroy(&mutex);
        fileout.close();
        
        // 记录结束时间点
    auto end = std::chrono::high_resolution_clock::now();

    // 计算执行时间并转换为毫秒
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 输出执行时间
    std::cout << "程序执行时间: " << duration.count() << " 毫秒" << std::endl;
    
    
        exit(0);
    }

    return 0;
}



void *map(void *arg) {
    param *param_data = (param *)arg;

    char *current_line = param_data->now;
    char *search_word = param_data->search_word;
    int length = param_data->len;

    for(int k = 0; k < length; k++){
        bool flag = false;
        std::string ll(current_line);
        size_t pos = ll.find(search_word);
        if (pos != std::string::npos) {
            flag = true;
            if(isalpha(ll[pos - 1]) || isalpha(ll[pos + strlen(search_word)])){
                flag = false;
            }
        }
        if(flag){
            pthread_mutex_lock(&mutex);
            strncpy(result.ans[result.index], ll.c_str(), SIZE - 1);
            ++result.index;
            pthread_mutex_unlock(&mutex);
        }
        current_line += SIZE;
    }

    return NULL;
}
void swap(char* a, char* b, int size) {
    char temp[SIZE];
    strncpy(temp, a, size - 1);
    strncpy(a, b, size - 1);
    strncpy(b, temp, size - 1);
}

int partition(char arr[LINE_SIZE][SIZE], int low, int high) {
    char* pivot = arr[high];
    int i = low - 1;

    for (int j = low; j <= high - 1; ++j) {
        if (strcmp(arr[j], pivot) < 0) {
            ++i;
            swap(arr[i], arr[j], SIZE);
        }
    }
    swap(arr[i + 1], arr[high], SIZE);
    return i + 1;
}

void quickSort(char arr[LINE_SIZE][SIZE], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}


void reduce(SharedData* shared_data) {
    pthread_mutex_lock(&mutex); // Lock mutex

//    for (int i = 0; i < result.index; ++i) {
//        for (int j = 0; j < result.index - i; ++j) {
//            if (strcmp(result.ans[j], result.ans[j + 1]) > 0) {
//                char temp[SIZE];
//                strncpy(temp, result.ans[j], SIZE - 1);
//                strncpy(result.ans[j], result.ans[j + 1], SIZE - 1);
//                strncpy(result.ans[j + 1], temp, SIZE - 1);
//            }
//        }
//    }

    quickSort(result.ans, 0, result.index - 1);

    // Copy sorted strings back to shared_data
    for (int i = 0; i < result.index && i < LINE_SIZE; ++i) {
        strncpy(shared_data->data[i], result.ans[i], SIZE - 1);
    }

    shared_data->l = result.index;

    pthread_mutex_unlock(&mutex);
}

