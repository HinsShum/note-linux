# 在当前文件夹下查找源文件
aux_source_directory(. DIR_RS232_SCRS)

# 生成链接库
add_library(rs232 ${DIR_RS232_SCRS})