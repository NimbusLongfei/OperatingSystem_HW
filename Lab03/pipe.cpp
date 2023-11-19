#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <locale>  // 用于 UTF-8 编码支持
#include <istream>
using namespace std;


int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <file_path> <find_word>" << endl;
        exit(1);
    }else{
        cout << "Success opening the file" << endl;
    }
    string file_path = argv[1];
    const char *word_to_search = argv[2];
    string file_out_path = argv[3];

    // 父写子读
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    } else {
        cout << "Success opening the pipe 1st 父写子读" << endl;
    }
    // 子写父读
    int pipe_fd2[2];
    if (pipe(pipe_fd2) == -1) {
        perror("pipe");
        return 1;
    }else{
        cout << "Success opening the pipe 2nd 子写父读" << endl;
    }

    // 子进程创建
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        return 1;



    }else if (child_pid == 0) { // Child
        close(pipe_fd[1]); // Close write end
        close(pipe_fd2[0]); // Close read end

        FILE* pipe_read = fdopen(pipe_fd[0], "r");

        if (pipe_read != nullptr) {

            vector<string> stline;
            char line[1024];

            while (fgets(line, sizeof(line), pipe_read)) {
                size_t len = strlen(line);

                // 去除末尾的换行符
                if (len > 0 && line[len - 1] == '\n') {
                    line[len - 1] = '\0';
                    len--;
                }

                string ll(line);
                size_t pos = ll.find(word_to_search);
                if (pos != string::npos) {
                    stline.push_back(ll);
                    if(isalpha(ll[pos - 1]) || isalpha(ll[pos + strlen(argv[2])])){
                       stline.pop_back();
                    }
                }
            }

            // 按字母表排序
            sort(stline.begin(), stline.end());

            // 将排序后的字符串写回到管道
            for (auto& now : stline) {
                now +="\n";
                write(pipe_fd2[1], now.c_str(), now.size());
            }

            cout << "成功" << endl;

            close(pipe_fd[0]);
            close(pipe_fd2[1]);
            exit(0);
        }


    } else if(child_pid > 0){ // Parent
        close(pipe_fd[0]); // Close read end
        close(pipe_fd2[1]); // write end

        ifstream file(file_path);

        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                line +="\n";
                write(pipe_fd[1], line.c_str(), line.size());
            }
        }
        file.close();
        close(pipe_fd[1]);

        FILE  * pipe_readagain = fdopen(pipe_fd2[0], "r");
        if(pipe_readagain != nullptr){
            char ans[1024];
            ofstream fileout(file_out_path, ios::out | ios::binary);
            while (fgets(ans, sizeof(ans), pipe_readagain)) {
                size_t len = strlen(ans);

                // 去除末尾的换行符
                if (len > 0 && ans[len - 1] == '\n') {
                    ans[len - 1] = '\0';
                    len--;
                }
                fileout << ans << endl;
            }

            fileout.close();
        }
        close(pipe_fd2[0]);

        int status;
        wait(&status);
        exit(0);
    }

    return 0;
}