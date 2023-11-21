#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <fcntl.h>
using namespace std;

int main(int argc, char *argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <file_path> <word_to_search>" << std::endl;
        return 1;
    }

    string file_path = argv[1];
    string word_to_search = argv[2];
    string file_out_path = argv[3];

    int sockfd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1) {
        perror("socketpair");
        return 1;
    }else   cout << "socketpair success\n";

    sem_t* p = sem_open("/test",O_CREAT|O_RDWR,0666,0);

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        return 1;
    }else if (child_pid == 0) { // Child process
        cout << "child create success\n";

//        fcntl(sockfd[1], F_SETFL, O_NONBLOCK);
//        ofstream  fileout(file_out_path, ios::out | ios::binary);
        FILE * sockread = fdopen(sockfd[1], "r+b");// 读写
        if (sockread != nullptr) {
            cout << "sockread success\n";
            vector<string> stline;
            char line[1024];
            int i =0;
            while (fgets(line, sizeof(line), sockread)) {
                size_t len = strlen(line);

                // 去除末尾的换行符
                if (len > 0 && line[len - 1] == '\n') {
                    line[len - 1] = '\0';
                    len--;
                }
                if(*((int*)line)==0XDEADBEEF){
                    cout << "--------------------------baibai================================\n" << endl;
                    break;
                }
//                cout << i++ << endl;
//                if(i >= 1) cout << (int)line[0] << "--" << line << "--"<<  endl;
//                fileout <<i++<< line << endl;
                string ll(line);

                size_t pos = ll.find(argv[2]);
                if (pos != string::npos) {
                    bool should_add = true;
                    if (isalpha(ll[pos - 1]) || isalpha(ll[pos + strlen(argv[2])])) {
                        should_add = false;
                    }
                    if (should_add) {
                        stline.push_back(ll);
                    }
                }
            }

            cout << "------------------------baibaiagain=========================="<<endl;

            sem_post(p);

            // 按字母表排序
            sort(stline.begin(), stline.end());

            for (auto& now : stline) {
                now +="\n";
                write(sockfd[1], now.c_str(), now.size());
            }

            int x = 0xDEADBEEF;
            write(sockfd[1], &x, 4);
            char  huanhang = '\n';
            write(sockfd[1], &huanhang, 1);

            cout << "---------------------------------成功================================" << endl;
            close(sockfd[1]);
            close(sockfd[0]);
//            fileout.close();
            sem_close(p);
            exit(0);
        }
        exit(1);
    } else if(child_pid > 0){ // Parent process
        cout << "parent create success\n";

        int buffer_size = 800000;  // 设置缓冲区大小，可以根据需要调整
        if (setsockopt(sockfd[0], SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size)) == -1) {
            perror("setsockopt");
            return 1;
        }

//        fcntl(sockfd[0], F_SETFL, O_NONBLOCK);
        ofstream fileout(file_out_path, ios::out | ios::binary);
        ifstream file(file_path);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                line +="\n";
                write(sockfd[0], line.c_str(), line.size());
            }
            int x = 0xDEADBEEF;
            write(sockfd[0], &x, 4);
            char  huanhang = '\n';
            write(sockfd[0], &huanhang, 1);
            file.close();
        }

//        sleep(5);
        sem_wait(p);

        FILE  * sock_readagain = fdopen(sockfd[0], "rb");
        if(sock_readagain != nullptr){
            char ans[1024];
            while (fgets(ans, sizeof(ans), sock_readagain)) {
                size_t len = strlen(ans);

                // 去除末尾的换行符
                if (len > 0 && ans[len - 1] == '\n') {
                    ans[len - 1] = '\0';
                    len--;
                }
                if(*((int*)ans)==0XDEADBEEF){
                    cout << "--------------------------baibaiparentprocess================================\n" << endl;
                    break;
                }
                fileout << ans << endl;
            }
            shutdown(sockfd[0], SHUT_WR);

            fileout.close();
        }
        close(sockfd[0]);
        close(sockfd[1]);
        sem_close(p);

        // 记录结束时间点
        auto end = std::chrono::high_resolution_clock::now();

        // 计算执行时间并转换为毫秒
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 输出执行时间
        std::cout << "程序执行时间: " << duration.count() << " ms" << std::endl;

        exit(0);
    }

    return 0;
}
