#!/bin/bash
# This script will use cmake to manage the project

function build()
{
    if [ "$board_name" == "linux" -o "$board_name" == "win32" ]; then
        cmake -B build -G "Unix Makefiles" -DBOARD_NAME=$board_name -DPROGRAM_NAME=$program_name -DCMAKE_BUILD_TYPE=$build_type
    else
        cmake -B build -G "Unix Makefiles" -DBOARD_NAME=$board_name -DPROGRAM_NAME=$program_name -DCMAKE_TOOLCHAIN_FILE=tools/${board_name}_toolchain.cmake -DCMAKE_BUILD_TYPE=$build_type
    fi
}

function compile()
{
    cmake --build build
}

function clear_build()
{
    echo "clear build"
    rm build -rf
    rm *.elf* -rf
}

function generate_flash_jlink()
{
    os=`uname -s`
    if [ $( expr $os : 'Linux' ) -gt 0 ]; then
        jlink=JLinkExe
    else
        jlink=JLink
    fi
    rm build/download.* -f
    rm build/erase.* -f
    echo -e "si SWD\nspeed 4000\nr\nh\nloadbin build/${board_name}.bin,${load_start_address}\nr\nexit" > build/download.jlink
    echo "${jlink} -Device ${device} -If SWD -Speed 4000 -JTAGConf -1,-1 -autoconnect 1 -CommanderScript build/download.jlink" > build/download.sh
    echo -e "si SWD\nspeed 4000\nr\nh\nerase ${load_start_address} ${load_end_address}\nexit" > build/erase.jlink
    echo "${jlink} -Device ${device} -If SWD -Speed 4000 -JTAGConf -1,-1 -autoconnect 1 -CommanderScript build/erase.jlink" > build/erase.sh
}

function download_bin()
{
    cmd=`cat build/download.sh`
    echo $cmd | sh
}

function erase_bin()
{
    cmd=`cat build/erase.sh`
    echo $cmd | sh
}

function help_info()
{
    echo "This shell script help user to build the easy_bootloader project easily"
    echo "Usage:"
    echo "./build.sh [options]"
    echo ""
    echo "options"
    echo "  -[B|b] --plat=<board name> --name=<program name> --[debug|release]  = build the project by board name"
    echo "  -[C|c]                                                              = compile the project"
    echo "  -[R|r]                                                              = clear the target and the project directory"
    echo "  --download                                                          = download bin file to the device"
    echo "  --erase                                                             = erase flash on the device"
}

if [ $# -eq 0 ]; then
    echo "Please input the board name that you want to compile"
    exit 0
fi

build=0
compile=0
clear=0
build_type=Debug
download=0
erase=0
support_board=("linux_0_0_null" "win32_0_0_null" "cy001_0x08000000_0x08004000_STM32F103ZE")
find_board=0

while [ $# -gt 0 ]; do
    if [ $( expr $1 : '-[B|b]' ) -gt 0 ]; then
        build=1
    elif [ $( expr $1 : '-[C|c]' ) -gt 0 ]; then
        compile=1
    elif [ $( expr $1 : '-[R|r]' ) -gt 0 ]; then
        clear=1
    elif [ $( expr $1 : '--plat=[a-z|A-Z|0-9]\{1,\}' ) -gt 0 ]; then
        board_name=$( expr $1 : '--plat=\([a-z|A-Z|0-9]\{1,\}\)' )
    elif [ $( expr $1 : '--name=[a-z|A-Z|0-9]\{1,\}' ) -gt 0 ]; then
        program_name=$( expr $1 : '--name=\([a-z|A-Z|0-9]\{1,\}\)' )
    elif [ $( expr $1 : '--release' ) -gt 0 ]; then
        build_type=Release
    elif [ $( expr $1 : '--debug' ) -gt 0 ]; then
        build_type=Debug
    elif [ $( expr $1 : '--download' ) -gt 0 ]; then
        download=1
    elif [ $( expr $1 : '--erase' ) -gt 0 ]; then
        erase=1
    elif [ $( expr $1 : '-[-help|h]' ) -gt 0 ]; then
        help_info
    fi
    shift
done

if [ $clear -gt 0 ]; then
    clear_build
fi

if [ -n "$board_name" ]; then
    for loop in ${support_board[*]}
    do
        # split by '_'
        support=(${loop//_/ })
        if [ $( expr $board_name : ${support[0]} ) -gt 0 ]; then
            find_board=1
            load_start_address=${support[1]}
            load_end_address=${support[2]}
            device=${support[3]}
        fi
    done
    if [ $find_board -eq 0 ]; then
        echo "Please check the board name that you input, can not find the board name:${board_name}"
        exit
    fi
fi

if [ "$board_name" == "linux" -o "$board_name" == "win32" ]; then
    download=0
    erase=0
fi

if [ $build -gt 0 ]; then
    if [ -n "$board_name" ]; then
        build
        if [ "$board_name" != "linux" -a "$board_name" != "win32" ]; then
            generate_flash_jlink
        fi
    else
        echo "Please input the board name that you want to compile"
        exit 0
    fi
fi

if [ $compile -gt 0 ]; then
    compile
    cp build/*.elf* ./
fi

if [ $download -gt 0 ]; then
    download_bin
fi

if [ $erase -gt 0 ]; then
    erase_bin
fi

exit 0