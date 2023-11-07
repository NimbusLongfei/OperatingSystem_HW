#include <iostream>
#include <pthread.h>
using namespace std;

const int N = 10;

int unsortedArray[N] = {7,12,19,3,18,4,2,6, 15,8};
int sortedArray[N];

void quick_sort(int l, int r);
void *sortArray(void *arg);
void mergeArrays();

int main() {
    pthread_t tid = pthread_self(); // 获取当前线程的线程ID
//    std::cout << "main Thread ID: " << tid << " is running." << std::endl;

    pthread_t sortingThreads[2];
    int thread_ids[2] = {0, N/2};

    for (int i = 0; i < 2; i++)
        pthread_create(&sortingThreads[i], NULL, sortArray, &thread_ids[i]);

    for (int i = 0; i < 2; i++)
        pthread_join(sortingThreads[i], NULL);

    mergeArrays();

    cout << "Sorted Array: ";
    for (int i = 0; i < N; i++)
        cout << sortedArray[i] << " ";
    cout << endl;

    return 0;
}

void quick_sort(int l, int r) {
    int *q = unsortedArray;
    if (l >= r)
        return;
    int i = l - 1, j = r + 1, x = q[l + r >> 1];
    while (i < j) {
        do
            i++;
        while (q[i] < x);
        do
            j--;
        while (q[j] > x);
        if (i < j)
            swap(q[i], q[j]);
    }
    quick_sort(l, j);
    quick_sort(j + 1, r);
}

void *sortArray(void *arg) {
    pthread_t tid = pthread_self(); // 获取当前线程的线程ID
//    std::cout << "Thread ID: " << tid << " is running." << std::endl;

    int thread_id = *(int *)arg;
    int start = thread_id;
    int end = start + (N / 2);

    quick_sort(start, end - 1);

    pthread_exit(NULL);
}

void mergeArrays() {
    int i = 0, j = N / 2, k = 0;

    while (i < N / 2 && j < N) {
        if (unsortedArray[i] < unsortedArray[j]) {
            sortedArray[k++] = unsortedArray[i++];
        } else {
            sortedArray[k++] = unsortedArray[j++];
        }
    }

    while (i < N / 2) {
        sortedArray[k++] = unsortedArray[i++];
    }

    while (j < N) {
        sortedArray[k++] = unsortedArray[j++];
    }
}
