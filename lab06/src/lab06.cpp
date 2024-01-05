#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
using namespace std;


#define MAX_FILES 100   // 假设最多 100 个文件
#define BLOCK_SIZE 512  // 每个块大小为 512 字节
#define MAX_BLOCKS 1024 // 磁盘块数为 1024

char ldisk[MAX_BLOCKS][BLOCK_SIZE]; // 模拟磁盘的数组
char mem_area[BLOCK_SIZE * 3];           // 内存区域
char buffer[BLOCK_SIZE];            // 缓冲区

// 文件描述符
struct file_descriptor {
    char filename[20];
    int length;
    int block_numbers[3]; // 假设每个文件最多分配3个块
};
/*fds 包含文件的名字、长度和分配的磁盘块号。文件名最多 20 个字符, */
struct file_descriptor fds[MAX_FILES]; 


// 打开文件表条目结构
// 每一条对应一个打开文件，包含文件描述符索引，读写指针
struct open_file {
    int fd_index;    // 文件描述符索引
    int rw_pointer;  // 读写指针
} open_files[MAX_FILES];


// 磁盘块位视图
int disk_bitmap[MAX_BLOCKS];
void init_disk_bitmap();
int allocate_block();
void display_disk_bitmap();

// I/O 系统函数
int read_block(int i, char *p);
int write_block(int i, char *p);
int save_ldisk_to_file(const char* filename);
int restore_ldisk_from_file(const char* filename);
int save_file_system_to_file(const char* filename);
int restore_file_system_from_file(const char* filename);

// 文件系统函数
int create(const char *filename);
int destroy(const char *filename);
int open(const char *filename);
int close(int index);
int read(int index, char *mem_area, int count);
int write(int index, char *mem_area, int count);
int lseek(int index, int pos);
void directory();




int main() {
    // 初始化磁盘，打开文件表
    memset(ldisk, 0, sizeof(ldisk));
    memset(open_files, -1, sizeof(open_files));

    // 保留区设置 
    // 前k个块中，2^10 bit用来存储位视图，共2^10/2^12 = 1/4个块
    // 剩下的块用于存储文件描述符
    // 从第k+1个块开始，存储文件数据
    // 这里进行一定简化，不将保留区的块显示在ldisk数组中，而是将位视图和文件描述符单独分别存储

    // 位视图
    init_disk_bitmap();
    // 文件描述符
    memset(fds, 0, sizeof(fds));

    // 目录
    // 初始化目录项,其中文件描述符索引为0的文件为目录文件
    strcpy(fds[0].filename, "directory");
    fds[0].length = 0; // 目录文件的长度代表文件的个数
    memset(fds[0].block_numbers, -1, sizeof(fds[0].block_numbers));

    // 随机初始化内存区域
    for (int i = 0; i < BLOCK_SIZE * 3; i++) {
        // 生成一个随机字符，这里以ASCII码可打印字符为例（范围32到126）
        mem_area[i] = (char)(rand() % (126 - 32 + 1)) + 32;
    }
    // 恢复磁盘内容
    restore_ldisk_from_file("ldisk.bin");
    restore_file_system_from_file("file_system.bin");

    // 需要循环读入命令，直到EOF退出
    char command[20];
    char filename[20];
    int index, count, pos, status;
    char mem[100];

    while (1) {
        printf("请输入命令：");
        fgets(command, 20, stdin);  // 读取用户输入的命令

        // 解析并执行命令
        if (sscanf(command, "create %s", filename) == 1) {
            status = create(filename);
            if(status == -1){
                printf("无空闲文件描述符\n");
            }
            else if(status == 0){
                printf("创建文件 %s\n", filename);
            }
        } else if (sscanf(command, "destroy %s", filename) == 1) {
            status = destroy(filename);
            if(status == -1){
                printf("文件未成功删除\n");
            }
            else if(status == 0){
                printf("删除文件 %s\n", filename);
            }
        } else if (sscanf(command, "open %s", filename) == 1) {
            index = open(filename);
            if(index != -1) printf("打开文件 %s，索引为 %d\n", filename, index);
            else printf("无空闲打开文件表条目\n");
        } else if (sscanf(command, "close %d", &index) == 1) {
            close(index);
            printf("关闭文件，索引为 %d\n", index);
        } else if (sscanf(command, "read %d %d", &index, &count) == 2) {
                if(open_files[index].fd_index != -1){
                    printf("从文件索引 %d 读取 %d 字节\n", index, count);
                    read(index, mem_area, count);
                }else{
                    printf("文件未打开\n");
                }
        } else if (sscanf(command, "write %d %s %d", &index, mem, &count) == 3) {
                if(open_files[index].fd_index != -1){
                    printf("向文件索引 %d 写入 %d 字节\n", index, count);
                    int cnt = write(index, mem_area, count);
                    printf("实际写入 %d 字节\n", cnt);
                }else{
                    printf("文件未打开\n");
                }
        } else if (sscanf(command, "lseek %d %d", &index, &pos) == 2) {
            lseek(index, pos);
            printf("移动文件索引 %d 的读写指针到位置 %d\n", index, pos);
        } else if (strcmp(command, "directory\n") == 0) {
            directory();
            display_disk_bitmap();
        } else if (strcmp(command, "exit\n") == 0) {
            // 保存到磁盘
            save_ldisk_to_file("ldisk.bin");
            save_file_system_to_file("file_system.bin");
            break;  // 退出循环
        } else {
            printf("无效命令\n");
        }
    }

}


// I/O 系统函数
int read_block(int i, char *p) {
    memcpy(p, ldisk[i], BLOCK_SIZE);
    // 返回实际读的字数
    return strlen(p);
}
int write_block(int i, char *p) {
    memcpy(ldisk[i], p, BLOCK_SIZE);
    // 返回实际写的字数
    return strlen(p);
}
// 还需要实现另外两个函数:一个用来把数组 ldisk 存储到文件; 另一个用来把文件内容恢复到数组
int save_ldisk_to_file(const char* filename) {
    ofstream file(filename, ios::binary);
    if (!file) {
        return -1; // 文件打开失败
    }
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if(ldisk[i] != NULL)
        file.write(ldisk[i], BLOCK_SIZE);
    }

    file.close();
    return 0; // 存储成功
}
int restore_ldisk_from_file(const char* filename) {
    ifstream file(filename, std::ios::binary);
    if (!file)  return -1; // 文件打开失败

    for (int i = 0; i < MAX_BLOCKS; i++) {
        file.read(ldisk[i], BLOCK_SIZE);
    }

    file.close();
    return 0; // 恢复成功
}
// 保存文件系统到文件
int save_file_system_to_file(const char* filename) {
    ofstream file(filename, ios::binary);
    if (!file)  return -1; // 文件打开失败

    // 保存位视图
    file.write((char*)disk_bitmap, sizeof(disk_bitmap));
    // 保存文件描述符
    file.write((char*)fds, sizeof(fds));

    file.close();
    return 0; // 存储成功
}
// 恢复文件系统
int restore_file_system_from_file(const char* filename) {
    ifstream file(filename, std::ios::binary);
    if (!file)  return -1; // 文件打开失败

    // 恢复位视图
    file.read((char*)disk_bitmap, sizeof(disk_bitmap));
    // 恢复文件描述符
    file.read((char*)fds, sizeof(fds));

    file.close();
    return 0; // 恢复成功
}  

// 初始化磁盘块位视图
void init_disk_bitmap() {
    memset(disk_bitmap, 0, sizeof(disk_bitmap));
}
// 分配一个空闲磁盘块
int allocate_block() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (!disk_bitmap[i]) {
            disk_bitmap[i] = 1;
            return i;
        }
    }
    return -1; // 没有空闲磁盘块可用
}
// 释放一个已分配的磁盘块
void deallocate_block(int block_number) {
    if (block_number >= 0 && block_number < MAX_BLOCKS) {
        disk_bitmap[block_number] = 0;
    }
}
// 输出显示磁盘块位视图
void display_disk_bitmap() {
    printf("磁盘块位视图：\n");
    for (int i = 0; i < MAX_BLOCKS; i++) {
        printf("%d", disk_bitmap[i]);
        if ((i + 1) % 64 == 0) {
            printf("\n");
        }
    }
}



// 文件系统函数
int create(const char *filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strlen(fds[i].filename) == 0) {
            strcpy(fds[i].filename, filename);
            fds[i].length = 0;
            memset(fds[i].block_numbers, -1, sizeof(fds[i].block_numbers));

            //添加目录项
            fds[0].length++;
            return 0; // 创建成功
        }
    }
    return -1; // 无空闲文件描述符
}
int destroy(const char *filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strcmp(fds[i].filename, filename) == 0) {
            for(int j = 0; j < 3; j++){
                if(fds[i].block_numbers[j] != -1){
                    deallocate_block(fds[i].block_numbers[j]);
                }
            }
            memset(&fds[i], 0, sizeof(struct file_descriptor));
            memset(&open_files[i], -1, sizeof(struct open_file));
            fds[0].length--;
            return 0; // 删除成功
        }
    }
    return -1; // 文件未找到
}
int open(const char *filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (strcmp(fds[i].filename, filename) == 0) {
            for (int j = 0; j < MAX_FILES; j++) {
                if (open_files[j].fd_index == -1) {
                    open_files[j].fd_index = i;
                    open_files[j].rw_pointer = 0;
                    // 读入一块到缓冲区
                    // if(fds[i].block_numbers[0] != -1){
                    //     read_block(fds[i].block_numbers[0], buffer);
                    // }   
                    return j; // 返回打开文件表索引
                }
            }
            return -1; // 无空闲打开文件表条目
        }
    }
    return -1; // 文件未找到
}
int close(int index) {
    if (index >= 0 && index < MAX_FILES && open_files[index].fd_index != -1) {
        open_files[index].fd_index = -1;
        open_files[index].rw_pointer = 0;
        return 0; // 关闭成功
    }
    return -1; // 无效的索引
}
int read(int index, char *mem_area, int count) {
    if (index >= 0 && index < MAX_FILES && open_files[index].fd_index != -1) {
        int fd_index = open_files[index].fd_index;
        int rw_pointer = open_files[index].rw_pointer;
        int remaining_length = fds[fd_index].length - rw_pointer; // 剩余可读取的字节数
        int bytes_to_read = (count < remaining_length) ? count : remaining_length;

        cout << "实际读取的字节数：" << bytes_to_read << endl;
        int block_index = rw_pointer / BLOCK_SIZE;
        int offset = rw_pointer % BLOCK_SIZE;
        int bytes_read = 0;

        while (bytes_read < bytes_to_read) {
            if (block_index >= 3 || fds[fd_index].block_numbers[block_index] == -1) {
                break; // 读取到文件末尾或无效的块号
            }
            
            char*block_data = ldisk[fds[fd_index].block_numbers[block_index]];
            int remaining_block_bytes = BLOCK_SIZE - offset;
            int bytes_to_copy = (bytes_to_read - bytes_read < remaining_block_bytes) ? bytes_to_read - bytes_read : remaining_block_bytes;
            
            // memcpy(mem_area + bytes_read, block_data + offset, bytes_to_copy);
            for(int i = offset; i < offset + bytes_to_copy; i++){
                cout << block_data[i];
            }
            cout << endl;

            bytes_read += bytes_to_copy;
            offset = 0;
            block_index++;
        }
        
        open_files[index].rw_pointer += bytes_read;
        return bytes_read;
        }
    
    return -1; // 无效的索引或无法读取数据
}
int write(int index, char *mem_area, int count) {
    if (index >= 0 && index < MAX_FILES && open_files[index].fd_index != -1) {
        int fd_index = open_files[index].fd_index;
        int rw_pointer = open_files[index].rw_pointer;
        
        int block_index = rw_pointer / BLOCK_SIZE; // 开始写的当前块是该文件的第几块
        int offset = rw_pointer % BLOCK_SIZE; // 块内偏移
        int bytes_written = 0;

        int remaining_length = 512 * 3 - fds[fd_index].length; // 剩余可写入的字节数
        int bytes_to_write = (count < remaining_length) ? count : remaining_length; // 实际写入的字节数

        cout << "实际写入的字节数：" << bytes_to_write << endl;

        if(count > remaining_length) printf("文件已满，无法继续写入\n");
        
        while (bytes_written < bytes_to_write) {
            if (block_index > 2) {
                break; // 文件已满，无法继续写入
            }
            
            if (fds[fd_index].block_numbers[block_index] == -1) {
                // 该文件需要分配新的块
                int block_number = allocate_block();
                if (block_number == -1) {
                    break; // 没有可用的块，无法继续写入
                }
                fds[fd_index].block_numbers[block_index] = block_number;
                offset = 0; // 更新块内偏移
            }
            
            char *block_data = ldisk[fds[fd_index].block_numbers[block_index]];
            int remaining_block_bytes = BLOCK_SIZE - offset;
            int bytes_to_copy = (bytes_to_write - bytes_written < remaining_block_bytes) ? bytes_to_write - bytes_written : remaining_block_bytes;
            
            memcpy(block_data + offset, mem_area + bytes_written, bytes_to_copy);
            
            bytes_written += bytes_to_copy;
            offset = 0;
            block_index++;

        }
        // 遍历该文件的所有块，并输出每个块的内容
        for (int i = 0; i < 3; i++) {
            if (fds[fd_index].block_numbers[i] != -1) {
                cout << "块号：" << fds[fd_index].block_numbers[i] << endl;
                cout << "块内容：" << ldisk[fds[fd_index].block_numbers[i]] << endl;
            }
        }
        
        fds[fd_index].length += bytes_written;
        cout << "文件长度：" << fds[fd_index].length << endl;
        open_files[index].rw_pointer += bytes_written;
        return bytes_written;
    }
    
    return -1; // 无效的索引或无法写入数据
}
int lseek(int index, int pos) {
    if (index >= 0 && index < MAX_FILES && open_files[index].fd_index != -1) {
        open_files[index].rw_pointer = pos;
        return 0; // 移动成功
    }
    return -1; // 无效的索引
}
void directory() {
    printf("文件名\t\t长度\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (strlen(fds[i].filename) != 0) {
            if(strcmp(fds[i].filename, "directory") == 0){
                printf("%s\t%d\n", fds[i].filename, fds[i].length);
                continue;
            }
            printf("%s\t\t%d\n", fds[i].filename, fds[i].length);
        }
    }
}