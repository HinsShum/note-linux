# @file /CMakeLists.txt
# @author HinsShum
# @date 2020-06-22
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

# set project describe information
project(ATLib VERSION 1.0.0 DESCRIPTION "project for atlib" LANGUAGES C)

# display project describe information
message("Project name: ${PROJECT_NAME}")
message("Project version: ${PROJECT_VERSION}")
message("Project description: ${PROJECT_DESCRIPTION}")
message("Project source path: ${PROJECT_SOURCE_DIR}")
message("Project binary path: ${PROJECT_BINARY_DIR}")

function(FILTER_OUT FILTEROUT_VARIABLE INPUT_VARIABLE OUTPUT_VARIABLE)
    set(OUTPUT_TMP)
    foreach(INPUT ${${INPUT_VARIABLE}})
        set(RESERVE 1)
        foreach(FILTEROUT ${${FILTEROUT_VARIABLE}})
            if("${INPUT}" STREQUAL "${FILTEROUT}")
                set(RESERVE 0)
                break()
            endif()
        endforeach(FILTEROUT)
        if(${RESERVE} EQUAL 1)
            set(OUTPUT_TMP ${OUTPUT_TMP} ${INPUT})
        endif()
    endforeach(INPUT)
    set(${OUTPUT_VARIABLE} ${OUTPUT_TMP} PARENT_SCOPE)
endfunction(FILTER_OUT)

function(DIRNAME INPUT OUTPUT)
    execute_process(COMMAND dirname ${${INPUT}} COMMAND uniq OUTPUT_VARIABLE DIR)
    string(REPLACE "\n" ";" DIR "${DIR}")
    set(${OUTPUT} ${DIR} PARENT_SCOPE)
endfunction(DIRNAME)

# add source files to compile
set(C_SOURCE_FILES "main.c")

# add libirary
set(C_SOURCE_LIBS)

# add os directory
add_subdirectory(lib/rs232)
add_subdirectory(atlib)

# compile executable
add_executable(main ${C_SOURCE_FILES})

# link library
target_link_libraries(main PUBLIC ${C_SOURCE_LIBS})
