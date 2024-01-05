#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;

// 内存对象
class MemoryAllocator {
private:
    struct MemoryBlock {
        string process_name;
        int start_address;
        int size;
        MemoryBlock* next;
    };

    MemoryBlock* head;   // 内存分配空闲队列头指针
    int max_memory_size; // 内存总大小

public:
    MemoryAllocator(int max_size) {
        /* 初始化内存分配空闲链表*/
        max_memory_size = max_size;
        head = new MemoryBlock(); // 创建第一个空闲块
        head->start_address = 0;
        head->size = max_size;
        head->process_name = "Free";
        head->next = nullptr;
    }

    void allocateMemory(string process, int size, char strategy) {
        MemoryBlock* current = head;
        MemoryBlock* prev = nullptr;
        MemoryBlock* new_block = nullptr;
        bool allocated = false;

        if (strategy == 'F') { // 首次适应算法
            while (current != nullptr) {
                if (current->process_name == "Free" && current->size >= size) {
                    new_block = new MemoryBlock();
                    new_block->process_name = process;
                    new_block->size = size;
                    new_block->start_address = current->start_address;

                    if (prev == nullptr) {
                        head = new_block;
                    } else {
                        prev->next = new_block;
                    }

                    if(current->size == size) {
                        new_block->next = current->next;
                        delete current;
                    } else {
                        new_block->next = current;
                        current->start_address += size;
                        current->size -= size;
                    }

                    allocated = true;
                    break;
                }
                prev = current;  // 记录前一个节点
                current = current->next;  // 移动到下一个节点
            }
        } else if (strategy == 'B') { // 最佳适应算法
            MemoryBlock* best_fit_block = nullptr;
            MemoryBlock* best_fit_prev = nullptr;
            int min_size = max_memory_size + 1;
            current = head;
            prev = nullptr;

            while (current != nullptr) {
                if (current->process_name == "Free" && current->size >= size && current->size < min_size) {
                    best_fit_block = current;
                    best_fit_prev = prev;
                    min_size = current->size;
                }
                prev = current;
                current = current->next;

            }

            if (best_fit_block != nullptr) {
                new_block = new MemoryBlock();
                new_block->process_name = process;
                new_block->size = size;
                new_block->start_address = best_fit_block->start_address;

                if (best_fit_prev == nullptr) {
                    head = new_block;
                } else {
                    best_fit_prev->next = new_block;
                }

                if(best_fit_block->size == size){
                    new_block->next = best_fit_block->next;
                    delete best_fit_block;
                } else {
                    new_block->next = best_fit_block;
                    best_fit_block->start_address += size;
                    best_fit_block->size -= size;
                }

                allocated = true;
            }
        } else if (strategy == 'W') { // 最差适应算法
            MemoryBlock* worst_fit_block = nullptr;
            MemoryBlock* worst_fit_prev = nullptr;
            int max_size = -1;
            current = head;
            prev = nullptr;

            while (current != nullptr) {
                if (current->process_name == "Free" && current->size >= size && current->size > max_size) {
                    worst_fit_block = current;
                    worst_fit_prev = prev;
                    max_size = current->size;
                }
                prev = current;
                current = current->next;
            }

            if (worst_fit_block != nullptr) {
                new_block = new MemoryBlock();
                new_block->process_name = process;
                new_block->size = size;
                new_block->start_address = worst_fit_block->start_address;

                if (worst_fit_prev == nullptr) {
                    head = new_block;
                } else {
                    worst_fit_prev->next = new_block;
                }

                if(worst_fit_block->size == size){
                    new_block->next = worst_fit_block->next;
                    delete worst_fit_block;
                } else {
                    new_block->next = worst_fit_block;
                    worst_fit_block->start_address += size;
                    worst_fit_block->size -= size;
                }

                allocated = true;
            }
        } else {
            cout << "Invalid strategy. Please enter F, B, or W." << endl;
            return;
        }

        if (!allocated) {
            cout << "Error: Insufficient memory to allocate for process " << process << endl;
        }
    }


    void releaseMemory(string process) {
        MemoryBlock* current = head;
        MemoryBlock* prev = nullptr;

        while (current != nullptr) {
            if (current->process_name == process) {
                current->process_name = "Free";

                // 合并相邻空闲块
//                if (prev != nullptr && prev->process_name == "Free") {
//                    prev->size += current->size;
//                    prev->next = current->next;
//                    delete current;
//                    current = prev;
//                }
//
//                if (current->next != nullptr && current->next->process_name == "Free") {
//                    current->size += current->next->size;
//                    MemoryBlock* temp = current->next;
//                    current->next = current->next->next;
//                    delete temp;
//                }
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    void compactMemory() {
        bool f = false;

        while (true) {
            MemoryBlock* current = head;
            f = false;

            while (current != nullptr && current->next != nullptr) {
                if (current->process_name == "Free" && current->next->process_name == "Free") {
                    f = true;
                    current->size += current->next->size;
                    MemoryBlock* temp = current->next;
                    current->next = current->next->next;
                    delete temp;
                }
                current = current->next;
            }

            if (!f) {
                break;
            }
        }
    }


    void reportStatus() {
        MemoryBlock* current = head;
        while (current != nullptr) {
            int end_address = current->start_address + current->size - 1;
            cout << "Addresses [" << current->start_address << ":" << end_address << "] ";

            if (current->process_name == "Free") {
                cout << "Unused" << endl;
            } else {
                cout << "Process " << current->process_name << endl;
            }
            current = current->next;
        }
    }

    void start() {
        string line;
        while (true) {
            cout << "allocator> ";
            getline(cin, line); // 逐行读取输入

            stringstream ss(line);
            string command;
            ss >> command;

            if (command == "RQ") {
                // 处理请求命令
                string process_name;
                int memory_size;
                char strategy;
                ss >> process_name >> memory_size >> strategy;
                allocateMemory(process_name, memory_size, strategy);
            } else if (command == "RL") {
                // 处理释放命令
                string process_name;
                ss >> process_name;
                releaseMemory(process_name);
            } else if (command == "C") {
                // 处理压缩命令
                compactMemory();
            } else if (command == "STAT") {
                // 处理状态报告命令
                reportStatus();
            } else if (command == "X") {
                // 退出程序
                break;
            } else {
                cout << "Invalid command. Please enter RQ, RL, C, STAT, or X." << endl;
            }
        }
    }


};



int main(int argc, char *argv[]) {
    if(argc != 2) {
        cerr << "Usage: "<< argv[0] << " <memory_size>" << endl;
        return 1;
    }

    MemoryAllocator allocator(atoi(argv[1]));

    allocator.start();

    return 0;
}
