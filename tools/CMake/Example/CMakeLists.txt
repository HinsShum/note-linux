# 指定CMake的版本依赖
cmake_minimum_required(VERSION 2.8)

# 指定工程名
project(Demo)

# 指定子目录
add_subdirectory(lib/rs232)

# 在当前文件夹下查找源文件
aux_source_directory(. DIR_SRCS)

# 指定生成目标
add_executable(Demo ${DIR_SRCS})

# 添加链接库
target_link_libraries(Demo rs232)