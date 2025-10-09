cmake_minimum_required(VERSION 3.31)
project(UniComm)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(UNICOMM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

set(LUA_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-5.4.8")
set(LUA_INCLUDE_DIR "${LUA_DIR}/include")
set(LUA_DLL "${LUA_DIR}/bin/lua54.dll")

set(LUA_LANGUAGE_SERVER_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-language-server-3.15.0")

set(QSCINTILLA_DIR "${CMAKE_SOURCE_DIR}/3rd/qscintilla-2.14.1")
set(QSCINTILLA_INCLUDE_DIR "${QSCINTILLA_DIR}/include")
set(QSCINTILLA_DLL "${QSCINTILLA_DIR}/bin/qscintilla2_qt6.dll")

set(QCUSTOMPLOT_DIR "${CMAKE_SOURCE_DIR}/3rd/qcustomplot-2.1.1")
set(QCUSTOMPLOT_INCLUDE_DIR "${QCUSTOMPLOT_DIR}/include")
set(QCUSTOMPLOT_DLL "${QCUSTOMPLOT_DIR}/bin/libqcustomplot.dll")

set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/3rd/opencv-4.12.0")
set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
file(GLOB OPENCV_DLLS "${OPENCV_DIR}/bin/libopencv_*.dll")

set(TESSERACT_DIR "${CMAKE_SOURCE_DIR}/3rd/tesseract-5.5.1")
set(TESSERACT_INCLUDE_DIR "${TESSERACT_DIR}/include")
set(TESSERACT_DLL "${TESSERACT_DIR}/bin/libtesseract55.dll")

set(LEPTONICA_DIR "${CMAKE_SOURCE_DIR}/3rd/leptonica-1.85.0")
set(LEPTONICA_INCLUDE_DIR "${LEPTONICA_DIR}/include")
set(LEPTONICA_DLL "${LEPTONICA_DIR}/bin/libleptonica.dll")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport SerialPort Network Multimedia TextToSpeech)

qt_add_executable(UniComm src/main.cpp
        include/mainWindow.h
        include/config.h
        include/database.h
        include/dataplot.h
        include/datatable.h
        include/debug.h
        include/diagnostics.h
        include/explorer.h
        include/globals.h
        include/log.h
        include/luaControl.h
        include/luaDataProcess.h
        include/luaInterpreter.h
        include/luaLanguageServer.h
        include/luaPort.h
        include/luaMiscellaneous.h
        include/luaModbus.h
        include/port.h
        include/send.h
        include/script.h
        include/suffix.h
        include/threadpool.h
        include/utils.h
        src/mainWindow.cpp
        src/config.cpp
        src/database.cpp
        src/dataplot.cpp
        src/datatable.cpp
        src/debug.cpp
        src/diagnostics.cpp
        src/explorer.cpp
        src/globals.cpp
        src/log.cpp
        src/luaControl.cpp
        src/luaDataProcess.cpp
        src/luaInterpreter.cpp
        src/luaLanguageServer.cpp
        src/luaPort.cpp
        src/luaMiscellaneous.cpp
        src/luaModbus.cpp
        src/port.cpp
        src/send.cpp
        src/script.cpp
        src/suffix.cpp
        src/threadpool.cpp
        src/utils.cpp
)

qt_add_resources(UniComm "readme_resources"
        FILES
        "readme.md"
)

qt_add_resources(UniComm "icon_resources"
        PREFIX "/icon"
        BASE "resources/icon"
        FILES
        "resources/icon/icon.ico"
        "resources/icon/add.svg"
        "resources/icon/arrowClockwise.svg"
        "resources/icon/arrowRight.svg"
        "resources/icon/autoFitHeight.svg"
        "resources/icon/bug.svg"
        "resources/icon/clock.svg"
        "resources/icon/crop.svg"
        "resources/icon/debugContinue.svg"
        "resources/icon/debugStepInto.svg"
        "resources/icon/debugStepOut.svg"
        "resources/icon/debugStepOver.svg"
        "resources/icon/delete.svg"
        "resources/icon/function.svg"
        "resources/icon/pause.svg"
        "resources/icon/play.svg"
        "resources/icon/save.svg"
        "resources/icon/send.svg"
        "resources/icon/stop.svg"
)

qt_add_resources(UniComm "config_resources"
        PREFIX "/config"
        BASE "resources/config"
        FILES
        "resources/config/.luarc.json"
        "resources/config/lib.d.lua"
)

target_link_libraries(UniComm PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Svg
        Qt::PrintSupport
        Qt::SerialPort
        Qt::Network
        Qt::Multimedia
        Qt::TextToSpeech
        ${LUA_DLL}
        ${QSCINTILLA_DLL}
        ${QCUSTOMPLOT_DLL}
        ${OPENCV_DLLS}
        ${TESSERACT_DLL}
        ${LEPTONICA_DLL}
)

target_include_directories(UniComm PRIVATE
        ${UNICOMM_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
        ${QSCINTILLA_INCLUDE_DIR}
        ${QCUSTOMPLOT_INCLUDE_DIR}
        ${OPENCV_INCLUDE_DIR}
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
    foreach (QT_LIB Core Gui Widgets Svg PrintSupport SerialPort Network Multimedia TextToSpeech)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)

    if (EXISTS "${QT_INSTALL_PATH}/plugins/imageformats/qsvg${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/imageformats/qsvg${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
    endif ()

    if (EXISTS "${QT_INSTALL_PATH}/plugins/imageformats/qico${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/imageformats/qico${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/imageformats/")
    endif ()

    if (EXISTS "${QT_INSTALL_PATH}/plugins/multimedia")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/multimedia/")
        file(GLOB QT_MM_BACKENDS "${QT_INSTALL_PATH}/plugins/multimedia/*.dll")
        foreach (DLL ${QT_MM_BACKENDS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL}"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/multimedia/")
        endforeach ()
    endif ()

    if (EXISTS "${QT_INSTALL_PATH}/plugins/texttospeech")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/texttospeech/")
        file(GLOB QT_MM_BACKENDS "${QT_INSTALL_PATH}/plugins/texttospeech/*.dll")
        foreach (DLL ${QT_MM_BACKENDS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL}"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/texttospeech/")
        endforeach ()
    endif ()

    if (EXISTS "${LUA_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LUA_DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif ()

    if (EXISTS "${LUA_LANGUAGE_SERVER_DIR}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${LUA_LANGUAGE_SERVER_DIR}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lua-language-server")
    endif ()

    if (EXISTS "${QSCINTILLA_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QSCINTILLA_DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif ()

    if (EXISTS "${QCUSTOMPLOT_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QCUSTOMPLOT_DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif ()

    file(GLOB OPENCV_DLLS "${OPENCV_DIR}/bin/*.dll")
    foreach (DLL ${OPENCV_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach ()

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

endif ()
