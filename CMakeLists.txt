cmake_minimum_required(VERSION 3.31)
project(UniComm)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(LUA_DIR "${CMAKE_SOURCE_DIR}/lib/lua-5.4.8")
set(LUA_INCLUDE_DIR "${LUA_DIR}/include")
set(LUA_LIBRARY "${LUA_DIR}/lua54.dll")

set(TESSERACT_DIR "${CMAKE_SOURCE_DIR}/lib/tesseract-5.5.1")
set(TESSERACT_INCLUDE_DIR "${TESSERACT_DIR}/include")
set(TESSERACT_DLL "${TESSERACT_DIR}/bin/libtesseract55.dll")

set(LEPTONICA_DIR "${CMAKE_SOURCE_DIR}/lib/leptonica-1.85.0")
set(LEPTONICA_INCLUDE_DIR "${LEPTONICA_DIR}/include")
set(LEPTONICA_DLL "${LEPTONICA_DIR}/bin/libleptonica.dll")

add_library(tesseract SHARED IMPORTED)
set_target_properties(tesseract PROPERTIES IMPORTED_LOCATION "${TESSERACT_DLL}")
set_target_properties(tesseract PROPERTIES IMPORTED_IMPLIB "${TESSERACT_DLL}")

add_library(leptonica SHARED IMPORTED)
set_target_properties(leptonica PROPERTIES IMPORTED_LOCATION "${LEPTONICA_DLL}")
set_target_properties(leptonica PROPERTIES IMPORTED_IMPLIB "${LEPTONICA_DLL}")

find_package(Qt6 COMPONENTS Core Gui Widgets SerialPort Svg REQUIRED)

qt_add_executable(UniComm src/main.cpp
        include/mainWindow.h
        include/config.h
        include/explorer.h
        include/log.h
        include/port.h
        include/send.h
        include/script.h
        include/suffix.h
        src/mainWindow.cpp
        src/config.cpp
        src/explorer.cpp
        src/log.cpp
        src/port.cpp
        src/send.cpp
        src/script.cpp
        src/suffix.cpp

)

qt_add_resources(UniComm "doc_resources"
        PREFIX "/doc"
        BASE "resources/doc"
        FILES
        "resources/doc/close.md"
        "resources/doc/open.md"
        "resources/doc/read.md"
        "resources/doc/write.md"
)

qt_add_resources(UniComm "icon_resources"
        PREFIX "/icon"
        BASE "resources/icon"
        FILES
        "resources/icon/add.svg"
        "resources/icon/send.svg"
        "resources/icon/stop.svg"
)

target_link_libraries(UniComm PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::SerialPort
        Qt::Svg
        ${LUA_LIBRARY}
        tesseract
        leptonica
)

target_include_directories(UniComm PRIVATE
        ${LUA_INCLUDE_DIR}
        ${TESSERACT_INCLUDE_DIR}
        ${LEPTONICA_INCLUDE_DIR}
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets SerialPort Svg)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
    if (EXISTS "${LUA_DIR}/lua54.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LUA_DIR}/lua54.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif ()

    file(GLOB TESSERACT_DLLS "${TESSERACT_DIR}/bin/*.dll")
    foreach (DLL ${TESSERACT_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach ()

    file(GLOB TESSDATA_FILES "${TESSERACT_DIR}/lang/*.traineddata")
    foreach (TESSDATA ${TESSDATA_FILES})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${TESSDATA}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach ()

    if (EXISTS "${LEPTONICA_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LEPTONICA_DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif ()

    if (EXISTS "${QT_INSTALL_PATH}/plugins/imageformats/qsvg${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/imageformats/qsvg${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
    endif ()
endif ()
