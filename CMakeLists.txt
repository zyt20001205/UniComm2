cmake_minimum_required(VERSION 3.31)
project(UniComm)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(UNICOMM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

set(MISC_DIR "${CMAKE_SOURCE_DIR}/3rd/misc")
file(GLOB MISC_DLLS "${MISC_DIR}/bin/*.dll")

set(DRMINGW_DIR "${CMAKE_SOURCE_DIR}/3rd/drmingw-0.9.11")
set(DRMINGW_INCLUDE_DIR "${DRMINGW_DIR}/include")
file(GLOB DRMINGW_DLLS "${DRMINGW_DIR}/bin/*.dll")

set(LUA_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-5.4.8")
set(LUA_INCLUDE_DIR "${LUA_DIR}/include")
set(LUA_DLL "${LUA_DIR}/bin/lua54.dll")

set(LUA_LANGUAGE_SERVER_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-language-server-3.15.0")

set(CMARK_DIR "${CMAKE_SOURCE_DIR}/3rd/cmark-0.31.1")
set(CMARK_INCLUDE_DIR "${CMARK_DIR}/include")
set(CMARK_DLL "${CMARK_DIR}/bin/libcmark.dll")

set(ICU4C_DIR "${CMAKE_SOURCE_DIR}/3rd/icu4c-77.1")
set(ICU4C_INCLUDE_DIR "${ICU4C_DIR}/include")
file(GLOB ICU4C_DLLS "${ICU4C_DIR}/bin/*.dll")

set(NUSPELL_DIR "${CMAKE_SOURCE_DIR}/3rd/nuspell-5.1.6")
set(NUSPELL_INCLUDE_DIR "${NUSPELL_DIR}/include")
set(NUSPELL_DLL "${NUSPELL_DIR}/bin/libnuspell.dll")

set(KDDOCKWIDGETS_DIR "${CMAKE_SOURCE_DIR}/3rd/kddockwidgets-2.3.0")
set(KDDOCKWIDGETS_INCLUDE_DIR "${KDDOCKWIDGETS_DIR}/include")
set(KDDOCKWIDGETS_DLL "${KDDOCKWIDGETS_DIR}/bin/libkddockwidgets-qt62.dll")

set(QSCINTILLA_DIR "${CMAKE_SOURCE_DIR}/3rd/qscintilla-2.14.1")
set(QSCINTILLA_INCLUDE_DIR "${QSCINTILLA_DIR}/include")
set(QSCINTILLA_DLL "${QSCINTILLA_DIR}/bin/qscintilla2_qt6.dll")

set(QCUSTOMPLOT_DIR "${CMAKE_SOURCE_DIR}/3rd/qcustomplot-2.1.1")
set(QCUSTOMPLOT_INCLUDE_DIR "${QCUSTOMPLOT_DIR}/include")
set(QCUSTOMPLOT_DLL "${QCUSTOMPLOT_DIR}/bin/libqcustomplot.dll")

set(VISA_DIR "${CMAKE_SOURCE_DIR}/3rd/visa")
set(VISA_INCLUDE_DIR "${VISA_DIR}/include")
file(GLOB VISA_DLLS "${VISA_DIR}/bin/*.dll")

set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/3rd/opencv-4.12.0")
set(OPENCV_INCLUDE_DIR "${OPENCV_DIR}/include")
file(GLOB OPENCV_DLLS "${OPENCV_DIR}/bin/libopencv_*.dll")

set(TESSERACT_DIR "${CMAKE_SOURCE_DIR}/3rd/tesseract-5.5.1")
set(TESSERACT_INCLUDE_DIR "${TESSERACT_DIR}/include")
set(TESSERACT_DLL "${TESSERACT_DIR}/bin/libtesseract-5.5.dll")

set(LEPTONICA_DIR "${CMAKE_SOURCE_DIR}/3rd/leptonica-1.86.0")
set(LEPTONICA_INCLUDE_DIR "${LEPTONICA_DIR}/include")
set(LEPTONICA_DLL "${LEPTONICA_DIR}/bin/libleptonica-6.dll")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport SerialPort Network Multimedia TextToSpeech)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GUI_TYPE WIN32)
else ()
    set(GUI_TYPE "")
endif ()

qt_add_executable(UniComm ${GUI_TYPE}
        src/main.cpp
        include/mainWindow.h
        include/configModule.h
        include/crashHandler.h
        include/dataModule/databaseModule.h
        include/dataModule/dataplotModule.h
        include/dataModule/datatableModule.h
        include/globals.h
        include/logModule.h
        include/luaModule/luaControl.h
        include/luaModule/luaDataProcess.h
        include/luaModule/luaInterpreter.h
        include/luaModule/luaLanguageServer.h
        include/luaModule/luaPort.h
        include/luaModule/luaMiscellaneous.h
        include/luaModule/luaModbus.h
        include/portModule/areaSelection.h
        include/portModule/basePort.h
        include/portModule/camera.h
        include/portModule/pixmapPreview.h
        include/portModule/portModule.h
        include/portModule/portPage.h
        include/portModule/portSetting.h
        include/portModule/screen.h
        include/portModule/sendModule.h
        include/portModule/serialPort.h
        include/portModule/tcpClient.h
        include/portModule/tcpServer.h
        include/portModule/udpSocket.h
        include/portModule/visa.h
        include/scriptModule/completionTooltip.h
        include/scriptModule/debugModule.h
        include/scriptModule/diagnosticsModule.h
        include/scriptModule/explorerModule.h
        include/scriptModule/gotoPopup.h
        include/scriptModule/nuspellModule.h
        include/scriptModule/positionTooltip.h
        include/scriptModule/scriptEditor.h
        include/scriptModule/scriptModule.h
        include/scriptModule/scriptPage.h
        include/scriptModule/signatureHelpTooltip.h
        include/scriptModule/structureModule.h
        include/scriptModule/threadpoolModule.h
        include/scriptModule/welcomePage.h
        include/scriptModule/codeAssistant/codeAssistant.h
        include/scriptModule/codeAssistant/dwellWidget.h
        include/settingModule/fontSettingLog.h
        include/settingModule/fontSettingScript.h
        include/settingModule/indicatorSettingScript.h
        include/settingModule/markerSettingScript.h
        include/settingModule/settingModule.h
        include/suffix.h
        include/undoModule.h
        include/utils/cvUtils.h
        include/utils/luaUtils.h
        include/utils/qtUtils.h
        src/mainWindow.cpp
        src/configModule.cpp
        src/crashHandler.cpp
        src/dataModule/databaseModule.cpp
        src/dataModule/dataplotModule.cpp
        src/dataModule/datatableModule.cpp
        src/globals.cpp
        src/logModule.cpp
        src/luaModule/luaControl.cpp
        src/luaModule/luaDataProcess.cpp
        src/luaModule/luaInterpreter.cpp
        src/luaModule/luaLanguageServer.cpp
        src/luaModule/luaPort.cpp
        src/luaModule/luaMiscellaneous.cpp
        src/luaModule/luaModbus.cpp
        src/portModule/areaSelection.cpp
        src/portModule/basePort.cpp
        src/portModule/camera.cpp
        src/portModule/pixmapPreview.cpp
        src/portModule/portModule.cpp
        src/portModule/portPage.cpp
        src/portModule/portSetting.cpp
        src/portModule/screen.cpp
        src/portModule/sendModule.cpp
        src/portModule/serialPort.cpp
        src/portModule/tcpClient.cpp
        src/portModule/tcpServer.cpp
        src/portModule/udpSocket.cpp
        src/portModule/visa.cpp
        src/scriptModule/completionTooltip.cpp
        src/scriptModule/debugModule.cpp
        src/scriptModule/diagnosticsModule.cpp
        src/scriptModule/explorerModule.cpp
        src/scriptModule/gotoPopup.cpp
        src/scriptModule/nuspellModule.cpp
        src/scriptModule/positionTooltip.cpp
        src/scriptModule/scriptEditor.cpp
        src/scriptModule/scriptModule.cpp
        src/scriptModule/scriptPage.cpp
        src/scriptModule/signatureHelpTooltip.cpp
        src/scriptModule/structureModule.cpp
        src/scriptModule/threadpoolModule.cpp
        src/scriptModule/welcomePage.cpp
        src/scriptModule/codeAssistant/codeAssistant.cpp
        src/scriptModule/codeAssistant/dwellWidget.cpp
        src/settingModule/fontSettingLog.cpp
        src/settingModule/fontSettingScript.cpp
        src/settingModule/indicatorSettingScript.cpp
        src/settingModule/markerSettingScript.cpp
        src/settingModule/settingModule.cpp
        src/suffix.cpp
        src/undoModule.cpp
        src/utils/cvUtils.cpp
        src/utils/luaUtils.cpp
        src/utils/qtUtils.cpp
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
        "resources/icon/apps.svg"
        "resources/icon/arrowClockwise.svg"
        "resources/icon/arrowDown.svg"
        "resources/icon/arrowRight.svg"
        "resources/icon/arrowUp.svg"
        "resources/icon/autoFitHeight.svg"
        "resources/icon/bug.svg"
        "resources/icon/clock.svg"
        "resources/icon/crop.svg"
        "resources/icon/dataBarVertical.svg"
        "resources/icon/debugContinue.svg"
        "resources/icon/debugStepInto.svg"
        "resources/icon/debugStepOut.svg"
        "resources/icon/debugStepOver.svg"
        "resources/icon/definition.svg"
        "resources/icon/delete.svg"
        "resources/icon/dock.svg"
        "resources/icon/folder.svg"
        "resources/icon/function.svg"
        "resources/icon/implementation.svg"
        "resources/icon/lockClosed.svg"
        "resources/icon/matchCase.svg"
        "resources/icon/open.svg"
        "resources/icon/pause.svg"
        "resources/icon/play.svg"
        "resources/icon/reference.svg"
        "resources/icon/regExp.svg"
        "resources/icon/reset.svg"
        "resources/icon/save.svg"
        "resources/icon/send.svg"
        "resources/icon/share.svg"
        "resources/icon/splitDown.svg"
        "resources/icon/splitLeft.svg"
        "resources/icon/splitRight.svg"
        "resources/icon/splitUp.svg"
        "resources/icon/stop.svg"
        "resources/icon/symbolEnum.svg"
        "resources/icon/symbolEnumMember.svg"
        "resources/icon/symbolField.svg"
        "resources/icon/symbolKeyword.svg"
        "resources/icon/symbolMethod.svg"
        "resources/icon/symbolString.svg"
        "resources/icon/symbolVariable.svg"
        "resources/icon/textCollapse.svg"
        "resources/icon/textExpand.svg"
        "resources/icon/typeDefinition.svg"
        "resources/icon/wholeWord.svg"
        "resources/icon/wordStart.svg"
)

qt_add_resources(UniComm "config_resources"
        PREFIX "/config"
        BASE "resources/config"
        FILES
        "resources/config/.luarc.json"
        "resources/config/config.json"
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
        ${DRMINGW_DLLS}
        ${LUA_DLL}
        ${CMARK_DLL}
        ${ICU4C_DLLS}
        ${NUSPELL_DLL}
        ${KDDOCKWIDGETS_DLL}
        ${QSCINTILLA_DLL}
        ${QCUSTOMPLOT_DLL}
        ${VISA_DLLS}
        ${OPENCV_DLLS}
        ${TESSERACT_DLL}
        ${LEPTONICA_DLL}
)

target_include_directories(UniComm PRIVATE
        ${UNICOMM_INCLUDE_DIR}
        ${DRMINGW_INCLUDE_DIR}
        ${LUA_INCLUDE_DIR}
        ${CMARK_INCLUDE_DIR}
        ${ICU4C_INCLUDE_DIR}
        ${NUSPELL_INCLUDE_DIR}
        ${KDDOCKWIDGETS_INCLUDE_DIR}
        ${QSCINTILLA_INCLUDE_DIR}
        ${QCUSTOMPLOT_INCLUDE_DIR}
        ${VISA_INCLUDE_DIR}
        ${OPENCV_INCLUDE_DIR}
        ${TESSERACT_INCLUDE_DIR}
        ${LEPTONICA_INCLUDE_DIR}
)

target_compile_definitions(UniComm PRIVATE
        KDDW_FRONTEND_QT
        KDDW_FRONTEND_QTWIDGETS
)

target_compile_options(UniComm PRIVATE
        -g
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

    file(GLOB MISC_DLLS "${MISC_DIR}/bin/*.dll")
    foreach (DLL ${MISC_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach ()

    file(GLOB DRMINGW_DLLS "${DRMINGW_DIR}/bin/*.dll")
    foreach (DLL ${DRMINGW_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach ()
    
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

    if (EXISTS "${CMARK_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMARK_DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endif ()

    file(GLOB ICU4C_DLLS "${ICU4C_DIR}/bin/*.dll")
    foreach (DLL ${ICU4C_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach ()

    if (EXISTS "${NUSPELL_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${NUSPELL_DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${NUSPELL_DIR}/dict"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/dict")
    endif ()

    if (EXISTS "${KDDOCKWIDGETS_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${KDDOCKWIDGETS_DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
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

    file(GLOB VISA_DLLS "${VISA_DIR}/bin/*.dll")
    foreach (DLL ${VISA_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach ()

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
