#include <iostream>
#include<cstring>
#include <pthread.h>
const int N = 9;
int puzzle[N][N] = {
        {5, 3, 4, 6, 7, 8, 9, 1, 2},
        {6, 7, 2, 1, 9, 5, 3, 4, 8},
        {1, 9, 8, 3, 4, 2, 5, 6, 7},
        {8, 5, 9, 7, 6, 1, 4, 2, 3},
        {4, 2, 6, 8, 5, 3, 7, 9, 1},
        {7, 1, 3, 9, 2, 4, 8, 5, 6},
        {9, 6, 1, 5, 3, 7, 2, 8, 4},
        {2, 8, 7, 4, 1, 9, 6, 3, 5},
        {3, 4, 5, 2, 8, 6, 1, 7, 9}
};
typedef struct {
    int index;
    int row;
    int column;
} parameters;
const int SUBGRID_SIZE = 3;
pthread_mutex_t mutex;
int result = 1; // 数独是否有效的标志，默认是1，如果有不符合的情况出现将其置为0
int results[11];

void * checkRowColumn(void *arg);
void * checkSubgrid(void *arg);

int main(void){
    pthread_t tid = pthread_self(); // 获取当前线程的线程ID
//    std::cout << "main Thread ID: " << tid << " is running." << std::endl;
    pthread_t threads[11];
    parameters paradata[11];

    pthread_mutex_init(&mutex, NULL);

    paradata[9].column = -1;
    paradata[9].index = 9;
    paradata[10].row =-1;
    paradata[10].index = 10;

    pthread_create(&threads[9], NULL, checkRowColumn, &paradata[9]);
    pthread_create(&threads[10], NULL, checkRowColumn, &paradata[10]);

    for (int i = 0; i < 3; ++i){
        for (int j = 0; j < 3; ++j) {
            paradata[i*3 + j%3].row = i * 3;
            paradata[i*3 + j%3].column = j * 3;
            paradata[i*3 + j%3].index = i*3 + j%3;
            pthread_create(&threads[i*3 + j%3], NULL, checkSubgrid, &paradata[i*3 + j%3]);
        }
    }
    for (int i = 0; i < 11; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);


    // 两种方式二选一即可
//    for(int i = 0; i < 11; i++){
//        if (results[i] == 0){
//            printf("Sudoku puzzle is not valid.");
//            return 0;
//        }
//    }
//    printf("Sudoku puzzle is valid.");

    if (result) printf("Sudoku puzzle is valid.");
    else    printf("Sudoku puzzle is not valid.");
    return 0;
}

void * checkRowColumn(void *arg) {
    pthread_t tid = pthread_self(); // 获取当前线程的线程ID
//    std::cout << "Thread ID: " << tid << " is running." << std::endl;

    parameters * data = (parameters *) arg;
    int id = data->index;
    int row = data->row;
    int col = data->column;
    int seen[N] = {0};

    if (col == -1){//  检查每一行
        for (int i = 0; i < N; ++i) {  // 遍历每一行
            memset(seen, 0, sizeof(seen));
            for (int j = 0; j < N; ++j) {
                int numInRow = puzzle[i][j];
                if (numInRow < 1 || numInRow > N || seen[numInRow - 1]) {
                    pthread_mutex_lock(&mutex);  // 锁定互斥锁
                    result = 0;  // 设定 result 为0
                    pthread_mutex_unlock(&mutex);  // 解锁互斥锁
                    pthread_exit(NULL);
                }
                seen[numInRow - 1] = 1;
            }

        }
    }else if (row == -1){
        for (int i = 0; i < N; ++i) {  // 遍历每一列
            memset(seen, 0, sizeof(seen));
            for (int j = 0; j < N; ++j) {
                int numInCol = puzzle[j][i];
                if (numInCol < 1 || numInCol > N || seen[numInCol - 1]) {
                    pthread_mutex_lock(&mutex);  // 锁定互斥锁
                    result = 0;  // 设定 result 为0
                    pthread_mutex_unlock(&mutex);  // 解锁互斥锁
                    pthread_exit(NULL);
                }
                seen[numInCol - 1] = 1;
            }
        }
    }
    pthread_mutex_lock(&mutex);
    results[id] = 1;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *checkSubgrid(void *arg) {
    pthread_t tid = pthread_self(); // 获取当前线程的线程ID
//    std::cout << "Thread ID: " << tid << " is running." << std::endl;

    parameters *data = (parameters *)arg;
    int row = data->row;
    int col = data->column;
    int id = data->index;
    int seen[N] = {0};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; ++j) {
            int nowRow = row + i, nowCol = col + j;
            int numInSubgrid = puzzle[nowRow][nowCol];
            if (numInSubgrid < 1 || numInSubgrid > N || seen[numInSubgrid - 1]) {
                pthread_mutex_lock(&mutex);  // 锁定互斥锁
                result = 0;  // 设定 result 为0
                pthread_mutex_unlock(&mutex);  // 解锁互斥锁
                pthread_exit(NULL);
            }
            seen[numInSubgrid - 1] = 1;
        }
    }
    pthread_mutex_lock(&mutex);
    results[id] = 1;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}