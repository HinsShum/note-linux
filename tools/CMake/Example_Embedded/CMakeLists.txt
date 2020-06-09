# @file /CMakeLists.txt
# @author HinsShum
# @date 2020-06-22
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

# set project describe information
project(Demo VERSION 1.0.0 DESCRIPTION "A demo CMake project" LANGUAGES C)

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

# set global include directory
set(G_SOURCE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}")
file(GLOB_RECURSE INCLUDES "*.h")
foreach(INCLUDE ${INCLUDES})
    execute_process(COMMAND dirname ${INCLUDE} OUTPUT_VARIABLE INCLUDE_DIR)
    # delete '\n'
    string(REGEX REPLACE "\n$" "" INCLUDE_DIR "${INCLUDE_DIR}")
    set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} ${INCLUDE_DIR})
endforeach(INCLUDE)

# set project lib
set(C_SOURCE_FILES)
set(C_SOURCE_LIBS)

# add os directory
add_subdirectory(freertos)
add_subdirectory(bsp/efm32lgxx)
add_subdirectory(lib/siliconlabs/efm32lgxx/emlib/src)
add_subdirectory(arch/efm32lgxx)
add_subdirectory(driver)
add_subdirectory(net/w5500)
add_subdirectory(src)

add_executable(${PROJECT_NAME}.elf ${C_SOURCE_FILES})
target_include_directories(${PROJECT_NAME}.elf PRIVATE "${G_SOURCE_INCLUDE_DIRS}")

target_link_libraries(${PROJECT_NAME}.elf PUBLIC ${C_SOURCE_LIBS})

# binary
set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
                   COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
                   COMMAND ${CMAKE_OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}
                   COMMENT "Building ${PROJECT_NAME}.bin and ${PROJECT_NAME}.hex"
                   COMMAND ${CMAKE_COMMAND} -E copy ${HEX_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
                   COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"
                   COMMAND ${CMAKE_SIZE} --format=berkeley ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
                   COMMENT "Invoking: Cross ARM GNU Print Size"
)