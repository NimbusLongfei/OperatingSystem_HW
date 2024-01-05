#include <iostream>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <fstream>

using namespace std;

int tlbindex = 0;
unsigned char tlb[16][2];
unsigned char page_table[256][2];   // logical page ---> physical page
signed char main_memory[256 * 256]; // physical memory

int pagefault_num = 0, tlbhit_num = 0;

bool compareFiles(const std::string& file1, const std::string& file2);

int main(int argc, char *argv[]){
    if(argc != 2){
        cout << "Usage: ./lab5_1 <address file>" << endl;
        return 0;
    }

    FILE *address_file = fopen(argv[1], "r");
    FILE *output = fopen("myans.txt", "w");


    char buf[10];
    int init = 0;
    while(fgets(buf, 10, address_file)) {
        int logiadd = atoi(buf);
        int offset = logiadd & 0xFF;
        int logipage = (logiadd >> 8) & 0xFF;
        int physpage = -1;

        bool tlbhit = false;
        for (int i = 0; i < 16; i++) {
            if (tlb[i][0] == logipage) {
                physpage = tlb[i][1];
                tlbhit = true;
                tlbhit_num+=1;

            	fprintf(output, "Virtual address: %d Physical address: %d Value: %d\n", logiadd, (physpage << 8)|offset, main_memory[physpage * 256 + offset]);
                break;
            }
        }
        if (tlbhit){
            continue;
        }

        for (int i = 0; i < 256; i++) {
            if (page_table[i][0] == logipage) {
                physpage = page_table[i][1];
                tlb[tlbindex % 16][0] = logipage;
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
            page_table[logipage][0] = logipage;
            page_table[logipage][1] = physpage;

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

    string file1 = "myans.txt";
    string file2 = "correct.txt";

    if (compareFiles(file1, file2)) {
        cout << "文件内容不同\n";
    } else {
        cout << "文件内容一致\n";
    }

    return 0;
}


bool compareFiles(const string& file1, const string& file2) {
    ifstream stream1(file1), stream2(file2);
    string line1, line2;
    int f = 0;
    // 逐行读取文件内容，并进行比较
    while (getline(stream1, line1) && getline(stream2, line2)) {
        if (line1 != line2) {
            f = 1;
        }
    }

    if(f == 1){
        return false;
    }

    return true;
}
