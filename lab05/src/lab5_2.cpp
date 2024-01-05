#include <iostream>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <fstream>

using namespace std;
// 用于存储tlb和page_table的数组
int tlbindex = 0;
unsigned char tlb[16][2];
unsigned char page_table[128][2];   // logical page ---> physical page
signed char main_memory[256 * 256]; // physical memory


// 使用FIFO算法，每次替换最先进入的页
int page_table_time[128];
int pagefault_num = 0, tlbhit_num = 0;

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Usage: ./lab5_1 <address file>" << endl;
        return 0;
    }

    // 读取地址文件
    FILE *address_file = fopen(argv[1], "r");
    FILE *output = fopen("myans.txt", "w");

    // 初始化page_table
    for(int i = 0; i < 128; i++) {
        for(int j = 0; j < 2; j++) {
            page_table[i][j] = -1;
        }
        page_table_time[i] = 0; // 初始化页面时间
    }

    char buf[10];
    int init = 0; // 初始化物理页号
    // 读取地址文件中的地址
    while(fgets(buf, 10, address_file)) {
        // 更新page_table_time
        for(int i = 0; i < 128; i++){
            if(page_table[i][0] != -1){
                page_table_time[i]++;
            }
        }

        int logiadd = atoi(buf);
        int offset = logiadd & 0xFF;
        int logipage = (logiadd >> 8) & 0xFF;
        int physpage = -1;

        bool tlbhit = false;
        for (int i = 0; i < 16; i++) {
            if (tlb[i][0] == logipage) {
                physpage = tlb[i][1];
                tlbhit = true;
                tlbhit_num++;
                fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n", logiadd, (physpage << 8)|offset, main_memory[physpage * 256 + offset]);
                break;
            }
        }
        if (tlbhit){
            continue;
        }

        for (int i = 0; i < 128; i++) {
            if (page_table[i][0] == logipage) {
                physpage = page_table[i][1];
                tlb[tlbindex % 16][0] = logipage; // 更新tlb
                tlb[tlbindex % 16][1] = physpage;
                tlbindex++;
                break;
            }
        }
        if (physpage == -1) {
            pagefault_num++ ;
            physpage = init++;
            // 从backing store中读取，并存入page_table和tlb以及main_memory
            FILE *backing_store = fopen("BACKING_STORE.bin", "rb");
            fseek(backing_store, logipage * 256, SEEK_SET);
            fread(main_memory + physpage * 256, 1, 256, backing_store);
            fclose(backing_store);

            fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n", logiadd, (physpage << 8)|offset, main_memory[physpage * 256 + offset]);

            // 更新page_table
            int update_f = 0;
            for(int i = 0; i < 128; i++){
                if(page_table[i][0] == -1){
                    page_table[i][0] = logipage;
                    page_table[i][1] = physpage;
                    page_table_time[i] = 0;
                    update_f = 1;
                    break;
                }
            }
            if(update_f == 0){
                int max_time = 0;
                int max_index = 0;
                for(int i = 0; i < 128; i++){
                    if(page_table_time[i] > max_time){
                        max_time = page_table_time[i];
                        max_index = i;
                    }
                }
                page_table[max_index][0] = logipage;
                page_table[max_index][1] = physpage;
                page_table_time[max_index] = 0;
            }

            // 更新tlb
            tlb[tlbindex % 16][0] = logipage;
            tlb[tlbindex % 16][1] = physpage;
            tlbindex++;
        } else {
            fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n", logiadd, (physpage << 8)|offset, main_memory[physpage * 256 + offset]);
        }
    }
    fclose(address_file);
    fclose(output);

    cout << "Page-fault rate:   " << pagefault_num / 10.0 << "%\n";
    cout << "TLB hit rate:  " << tlbhit_num / 10.0 << "%" << endl;

    return 0;
}
