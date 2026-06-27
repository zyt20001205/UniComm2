cmake_minimum_required(VERSION 3.31)
project(UniComm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/MyFileAudit/software/msys64/ucrt64")

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Core5Compat
        Graphs
        Gui
        Multimedia
        Network
        Pdf
        PrintSupport
        Qml
        Quick
        QuickControls2
        QuickWidgets
        SerialPort
        Svg
        TextToSpeech
        WebView
        Widgets
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GUI_TYPE WIN32)
else ()
    set(GUI_TYPE "")
endif ()

file(GLOB_RECURSE UNICOMM_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/include/*.h"
)

qt_add_executable(UniComm ${GUI_TYPE}
        resources/icon/app.rc
        ${UNICOMM_SOURCES}
)

file(GLOB_RECURSE UNICOMM_QML CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/qml/*.qml"
)

qt_add_resources(UniComm "qml_resources"
        PREFIX "/qml"
        BASE "${CMAKE_SOURCE_DIR}/qml"
        FILES
        ${UNICOMM_QML}
)

file(GLOB_RECURSE UNICOMM_ICON CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/resources/*.ico"
        "${CMAKE_SOURCE_DIR}/resources/*.svg"
)

qt_add_resources(UniComm "icon_resources"
        PREFIX "/icon"
        BASE "${CMAKE_SOURCE_DIR}/resources/icon"
        FILES
        ${UNICOMM_ICON}
)

qt_add_resources(UniComm "config_resources"
        PREFIX "/config"
        BASE "resources/config"
        FILES
        "resources/config/.luarc.json"
        "resources/config/config.json"
)

qt_add_resources(UniComm "lib_resources"
        PREFIX "/lib"
        BASE "resources/lib"
        FILES
        "resources/lib/alias.d.lua"
        "resources/lib/database.d.lua"
        "resources/lib/datatable.d.lua"
        "resources/lib/file.d.lua"
        "resources/lib/http.d.lua"
        "resources/lib/imap.d.lua"
        "resources/lib/io.d.lua"
        "resources/lib/key.d.lua"
        "resources/lib/modbusAscii.d.lua"
        "resources/lib/modbusRtu.d.lua"
        "resources/lib/modbusTcp.d.lua"
        "resources/lib/mouse.d.lua"
        "resources/lib/mqtt.d.lua"
        "resources/lib/port.d.lua"
        "resources/lib/smtp.d.lua"
        "resources/lib/string.d.lua"
        "resources/lib/thread.d.lua"
)

qt_add_resources(UniComm "demo_resources"
        PREFIX "/demo"
        BASE "resources/demo"
        FILES
        "resources/demo/modbusRtu.lua"
)

set(UNICOMM_INCLUDE "${CMAKE_SOURCE_DIR}/include")

set(QT_DIR "${CMAKE_SOURCE_DIR}/3rd/qt")
set(QT_BIN "${QT_DIR}/bin")
set(QT_QML "${QT_DIR}/qml")
set(QT_SHARE "${QT_DIR}/share")

set(MISC_DIR "${CMAKE_SOURCE_DIR}/3rd/misc")
set(MISC_BIN "${MISC_DIR}/bin")

#3rd alphabetical order
set(CMARK_DIR "${CMAKE_SOURCE_DIR}/3rd/cmark")
set(CMARK_INCLUDE "${CMARK_DIR}/include")
file(GLOB CMARK_LIB "${CMARK_DIR}/lib/*.a")
set(CMARK_BIN "${CMARK_DIR}/bin")

set(DRMINGW_DIR "${CMAKE_SOURCE_DIR}/3rd/drmingw")
set(DRMINGW_INCLUDE "${DRMINGW_DIR}/include")
file(GLOB DRMINGW_LIB "${DRMINGW_DIR}/lib/*.a")
set(DRMINGW_BIN "${DRMINGW_DIR}/bin")

set(ICU_DIR "${CMAKE_SOURCE_DIR}/3rd/icu")
set(ICU_INCLUDE "${ICU_DIR}/include")
file(GLOB ICU_LIB "${ICU_DIR}/lib/*.a")
set(ICU_BIN "${ICU_DIR}/bin")

set(KDDW_DIR "${CMAKE_SOURCE_DIR}/3rd/kddw")
set(KDDW_INCLUDE "${KDDW_DIR}/include")
file(GLOB KDDW_LIB "${KDDW_DIR}/lib/*.a")
set(KDDW_BIN "${KDDW_DIR}/bin")

set(LEPTONICA_DIR "${CMAKE_SOURCE_DIR}/3rd/leptonica")
set(LEPTONICA_INCLUDE "${LEPTONICA_DIR}/include")
file(GLOB LEPTONICA_LIB "${LEPTONICA_DIR}/lib/*.a")
set(LEPTONICA_BIN "${LEPTONICA_DIR}/bin")

set(LUA_DIR "${CMAKE_SOURCE_DIR}/3rd/lua")
set(LUA_INCLUDE "${LUA_DIR}/include")
file(GLOB LUA_LIB "${LUA_DIR}/lib/*.a")
set(LUA_BIN "${LUA_DIR}/bin")

set(LUALS_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-language-server")

set(NUSPELL_DIR "${CMAKE_SOURCE_DIR}/3rd/nuspell")
set(NUSPELL_INCLUDE "${NUSPELL_DIR}/include")
file(GLOB NUSPELL_LIB "${NUSPELL_DIR}/lib/*.a")
set(NUSPELL_BIN "${NUSPELL_DIR}/bin")
set(NUSPELL_DICT "${NUSPELL_DIR}/dict")

set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/3rd/opencv")
set(OPENCV_INCLUDE "${OPENCV_DIR}/include")
file(GLOB OPENCV_LIB "${OPENCV_DIR}/lib/*.a")
set(OPENCV_BIN "${OPENCV_DIR}/bin")

set(QTKEYCHAIN_DIR "${CMAKE_SOURCE_DIR}/3rd/qtkeychain")
set(QTKEYCHAIN_INCLUDE "${QTKEYCHAIN_DIR}/include")
file(GLOB QTKEYCHAIN_LIB "${QTKEYCHAIN_DIR}/lib/*.a")
set(QTKEYCHAIN_BIN "${QTKEYCHAIN_DIR}/bin")

set(RIPGREP_DIR "${CMAKE_SOURCE_DIR}/3rd/ripgrep")

set(SCINTILLA_DIR "${CMAKE_SOURCE_DIR}/3rd/scintilla")
set(SCINTILLA_INCLUDE "${SCINTILLA_DIR}/include")
file(GLOB SCINTILLA_LIB "${SCINTILLA_DIR}/lib/*.a")
set(SCINTILLA_BIN "${SCINTILLA_DIR}/bin")

set(SOL2_DIR "${CMAKE_SOURCE_DIR}/3rd/sol2")
set(SOL2_INCLUDE "${SOL2_DIR}/include")

set(TESSERACT_DIR "${CMAKE_SOURCE_DIR}/3rd/tesseract")
set(TESSERACT_INCLUDE "${TESSERACT_DIR}/include")
file(GLOB TESSERACT_LIB "${TESSERACT_DIR}/lib/*.a")
set(TESSERACT_BIN "${TESSERACT_DIR}/bin")
set(TESSERACT_LANG "${TESSERACT_DIR}/lang")

set(VISA_DIR "${CMAKE_SOURCE_DIR}/3rd/visa")
set(VISA_INCLUDE "${VISA_DIR}/include")
file(GLOB VISA_LIB "${VISA_DIR}/bin/*.dll")
set(VISA_BIN "${VISA_DIR}/bin")

set(VTERM_DIR "${CMAKE_SOURCE_DIR}/3rd/vterm")
set(VTERM_INCLUDE "${VTERM_DIR}/include")
file(GLOB VTERM_LIB "${VTERM_DIR}/lib/*.a")
set(VTERM_BIN "${VTERM_DIR}/bin")

target_include_directories(UniComm PRIVATE
        ${UNICOMM_INCLUDE}
        ${CMARK_INCLUDE}
        ${DRMINGW_INCLUDE}
        ${ICU_INCLUDE}
        ${KDDW_INCLUDE}
        ${LEPTONICA_INCLUDE}
        ${LUA_INCLUDE}
        ${NUSPELL_INCLUDE}
        ${OPENCV_INCLUDE}
        ${QTKEYCHAIN_INCLUDE}
        ${SCINTILLA_INCLUDE}
        ${SOL2_INCLUDE}
        ${TESSERACT_INCLUDE}
        ${VISA_INCLUDE}
        ${VTERM_INCLUDE}
)

target_link_libraries(UniComm PRIVATE
        Qt6::Core
        Qt6::Core5Compat
        Qt6::Gui
        Qt6::Graphs
        Qt6::Multimedia
        Qt6::Network
        Qt6::Pdf
        Qt6::PrintSupport
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickWidgets
        Qt6::SerialPort
        Qt6::Svg
        Qt6::TextToSpeech
        Qt6::WebView
        Qt6::Widgets
        ${CMARK_LIB}
        ${DRMINGW_LIB}
        ${ICU_LIB}
        ${KDDW_LIB}
        ${LEPTONICA_LIB}
        ${LUA_LIB}
        ${NUSPELL_LIB}
        ${QTKEYCHAIN_LIB}
        ${SCINTILLA_LIB}
        ${TESSERACT_LIB}
        ${VISA_LIB}
        ${VTERM_LIB}
        -Wl,--start-group
        ${OPENCV_LIB}
        -Wl,--end-group
)

target_compile_definitions(UniComm PRIVATE
        KDDW_FRONTEND_QT
        KDDW_FRONTEND_QTWIDGETS
)

target_compile_options(UniComm PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Release>:-g1>
)

if(FALSE)
    #auto deploy(do not use)
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND Qt6::windeployqt
            --qmldir "${CMAKE_SOURCE_DIR}/qml"
            "$<TARGET_FILE:UniComm>")
else ()
    #theme
    set(THEME_DIR "${CMAKE_SOURCE_DIR}/resources/theme")
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${THEME_DIR}"
            "${CMAKE_BINARY_DIR}/theme"
            COMMENT "THEME DIR COPIED")
    #lua-language-server-3.18.2
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LUALS_DIR}"
            "${CMAKE_BINARY_DIR}/lua-language-server"
            COMMENT "LUALS DIR COPIED")
    #nuspell-5.1.7-1
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${NUSPELL_DICT}"
            "${CMAKE_BINARY_DIR}/dict"
            COMMENT "NUSPELL DICT COPIED")
    #ripgrep-15.1.0
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RIPGREP_DIR}"
            "${CMAKE_BINARY_DIR}/ripgrep"
            COMMENT "RIPGREP DIR COPIED")
    #tesseract-5.5.2-1
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TESSERACT_LANG}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "TESSERACT LANG COPIED")

    #qt-6.11.1
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "QT BIN COPIED")
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_SHARE}"
            "${CMAKE_BINARY_DIR}/share"
            COMMENT "QT SHARE COPIED")
    #misc
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MISC_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "MISC BIN COPIED")
    #cmark-0.31.2
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMARK_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "CMARK BIN COPIED")
    #drmingw-0.9.11-2
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${DRMINGW_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "DRMINGW BIN COPIED")
    #icu-78.3-2
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ICU_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "ICU BIN COPIED")
    #kddw-2.4.0
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${KDDW_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "KDDW BIN COPIED")
    #leptonica-1.87.0-1
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LEPTONICA_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "LEPTONICA BIN COPIED")
    #lua-5.5.0-1
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LUA_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "LUA BIN COPIED")
    #nuspell-5.1.7-1
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${NUSPELL_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "NUSPELL BIN COPIED")
    #opencv-4.13.0-4
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${OPENCV_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "OPENCV BIN COPIED")
    #qtkeychain-0.15.0-2
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QTKEYCHAIN_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "QTKEYCHAIN BIN COPIED")
    #scintilla-5.6.2
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SCINTILLA_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "SCINTILLA BIN COPIED")
    #tesseract-5.5.2-1
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TESSERACT_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "TESSERACT BIN COPIED")
    #visa
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VISA_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "VISA BIN COPIED")
    #vterm-0.3.3-1
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VTERM_BIN}"
            "${CMAKE_BINARY_DIR}"
            COMMENT "VTERM BIN COPIED")
endif()
