| 符号 | 含义                         |
| ---- | ---------------------------- |
| $    | 扩展输出路径                 |
| #    | 扩展为输出文件绝对路径       |
| @    | 扩展为输出文件名称（无后缀） |
| L    | 输出文件                     |
| K    | MDK安装路径                  |

**样例：**

Name of Executable：a-debug

Folder for Objects: C:/home

`fromelf --bin #L --output bin/@L.bin`

* #L: C:/home/a-debug.axf
* @L: a-debug

`cmd.exe /c copy /y $L@L.hex bin\@L.hex`

* $L: C:/home/