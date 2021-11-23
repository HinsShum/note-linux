# @file CMakeLists.txt
# @author HinsShum
# @date 2020-06-22
# @brief Here's the first line of every CMakeLists.txt,
#        which is the required name of the file CMake looks for:
cmake_minimum_required(VERSION 3.1)

# check the board name is right
if(NOT DEFINED BOARD_NAME)
    message(FATAL_ERROR "Please input the board name, eg: -DBOARD_NAME=XXXXX")
else()
    execute_process(COMMAND cat "support_board.cfg" OUTPUT_VARIABLE ALL_BOARD)
    string(REPLACE "\n" ";" ALL_BOARD "${ALL_BOARD}")
    foreach(BOARD ${ALL_BOARD})
        if("${BOARD_NAME}" STREQUAL "${BOARD}")
            set(BOARD_OK 1)
            break()
        endif()
    endforeach(BOARD)
    if(NOT DEFINED BOARD_OK)
        message(FATAL_ERROR "Can not find the board(${BOARD_NAME}), please check the input parameters")
    endif()
endif()

# check program name
if("${PROGRAM_NAME}" STREQUAL "")
    set(PROGRAM_NAME "demo")
endif()

# set project describe information
project(${PROGRAM_NAME} VERSION 1.0.0 DESCRIPTION "A demo CMake project" LANGUAGES C)

# display project describe information
message("Project name: ${PROJECT_NAME}")
message("Project board: ${BOARD_NAME}")
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

# set global include directory
set(G_SOURCE_INCLUDE_DIRS ${G_SOURCE_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})

#set target c file pathname
set(C_SOURCE_FILES)
# set project lib
set(C_SOURCE_LIBS)

add_definitions(-DCONFIG_OPTIONS_FILE="config/${BOARD_NAME}/options.h")

include("tools/${BOARD_NAME}.cmake")
add_subdirectory(src)
add_subdirectory(driver)
add_subdirectory(common)
add_subdirectory(utils)
add_subdirectory(components)

# tell cmake asm file also is c file
set_property(SOURCE ${C_SOURCE_FILES} PROPERTY LANGUAGE C)

add_executable(${PROJECT_NAME}.elf ${C_SOURCE_FILES})
target_include_directories(${PROJECT_NAME}.elf PRIVATE "${G_SOURCE_INCLUDE_DIRS}")

target_link_libraries(${PROJECT_NAME}.elf PUBLIC ${C_SOURCE_LIBS})

if(NOT "${BOARD_NAME}" STREQUAL "linux" AND NOT "${BOARD_NAME}" STREQUAL "win32")
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
else()
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Map,${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref")
endif()