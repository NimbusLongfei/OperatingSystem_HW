
# This is a piece of magic

    When this program is not output, please count five.

```
文件结构
├── remind.md
├── test.txt
├── sockpair.cpp
└── anssockpair.txt
```

test.txt为一个样例输入文件，anssockpair为一个样例输出文件。

## 使用方法

1. 编译程序：
   
   打开终端并切换到包含代码的目录
   ```bash
   g++ sockpair.cpp
   ```
    这将生成名为`a.out`的可执行文件
2. 运行程序：

    `file_path` 为要检查文件路径
    
    `word_to_search`为要搜索的词
    
    `file_out_path`为要输出结果的文件路径

   ```bash
   ./a.out file_path word_to_search file_out_path
   ```
