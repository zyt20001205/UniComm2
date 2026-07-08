cmake_minimum_required(VERSION 3.31)

project(UniComm_Vterm)

set(CMAKE_C_STANDARD 99)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

file(GLOB VTERM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_library(UniComm_Vterm STATIC ${VTERM_SOURCES})

target_include_directories(UniComm_Vterm PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(UniComm_Vterm PRIVATE -Wno-attributes)
endif ()
