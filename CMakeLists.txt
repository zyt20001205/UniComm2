cmake_minimum_required(VERSION 3.31)
project(UniComm)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Core5Compat
        Graphs
        Gui
        Multimedia
        Network
        PrintSupport
        Qml
        Quick
        QuickControls2
        QuickWidgets
        SerialPort
        Svg
        TextToSpeech
        Widgets
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GUI_TYPE WIN32)
else ()
    set(GUI_TYPE "")
endif ()

qt_add_executable(UniComm ${GUI_TYPE}
        src/main.cpp
        include/configManager.h
        include/crashHandler.h
        include/dataModule/databaseModule.h
        include/dataModule/dataplotModule.h
        include/dataModule/datatableModule.h
        include/globals.h
        include/systemModule.h
        include/logModule.h
        include/luaModule/luaControl.h
        include/luaModule/luaDataProcess.h
        include/luaModule/luaInterpreter.h
        include/luaModule/luaIO.h
        include/luaModule/luaLanguageServer.h
        include/luaModule/luaModbusAscii.h
        include/luaModule/luaModbusRtu.h
        include/luaModule/luaPort.h
        include/luaModule/luaSmtp.h
        include/luaModule/luaThread.h
        include/mainWindow/kddwCustom.h
        include/mainWindow/mainWindow.h
        include/mainWindow/menuModule.h
        include/mainWindow/statusModule.h
        include/portModule/basePort.h
        include/portModule/pixmapPreview.h
        include/portModule/portModule.h
        include/portModule/portSetting.h
        include/portModule/sendModule.h
        include/portModule/serialPort.h
        include/portModule/sslClient.h
        include/portModule/tcpClient.h
        include/portModule/tcpServer.h
        include/portModule/udpSocket.h
        include/portModule/videoStream.h
        include/portModule/visa.h
        include/scriptModule/nuspellModule.h
        include/scriptModule/scriptModule.h
        include/scriptModule/scriptPage.h
        include/scriptModule/welcomePage.h
        include/scriptModule/codeAnalysis/codeAssistant.h
        include/scriptModule/codeAnalysis/completionWidget.h
        include/scriptModule/codeAnalysis/diagnosticsModule.h
        include/scriptModule/codeAnalysis/dwellWidget.h
        include/scriptModule/codeAnalysis/navigationWidget.h
        include/scriptModule/codeAnalysis/positionWidget.h
        include/scriptModule/codeAnalysis/signatureWidget.h
        include/scriptModule/codeAnalysis/structureModule.h
        include/scriptModule/codeDebug/breakpointModule.h
        include/scriptModule/codeDebug/debugModule.h
        include/scriptModule/codeDebug/threadpoolModule.h
        include/scriptModule/codeDebug/watchModule.h
        include/scriptModule/codeEditor/editorWidget.h
        include/scriptModule/codeEditor/explorerModule.h
        include/scriptModule/codeEditor/searchWidget.h
        include/settingModule/fontSettingLog.h
        include/settingModule/fontSettingScript.h
        include/settingModule/indicatorSettingScript.h
        include/settingModule/markerSettingScript.h
        include/settingModule/settingModule.h
        include/undoModule.h
        include/utils/cmarkUtils.h
        include/utils/cvUtils.h
        include/utils/luaUtils.h
        include/utils/qtUtils.h
        include/utils/suffixUtils.h
        src/configManager.cpp
        src/crashHandler.cpp
        src/dataModule/databaseModule.cpp
        src/dataModule/dataplotModule.cpp
        src/dataModule/datatableModule.cpp
        src/globals.cpp
        src/systemModule.cpp
        src/logModule.cpp
        src/luaModule/luaControl.cpp
        src/luaModule/luaDataProcess.cpp
        src/luaModule/luaInterpreter.cpp
        src/luaModule/luaIO.cpp
        src/luaModule/luaLanguageServer.cpp
        src/luaModule/luaModbusAscii.cpp
        src/luaModule/luaModbusRtu.cpp
        src/luaModule/luaPort.cpp
        src/luaModule/luaSmtp.cpp
        src/luaModule/luaThread.cpp
        src/mainWindow/kddwCustom.cpp
        src/mainWindow/mainWindow.cpp
        src/mainWindow/menuModule.cpp
        src/mainWindow/statusModule.cpp
        src/portModule/basePort.cpp
        src/portModule/pixmapPreview.cpp
        src/portModule/portModule.cpp
        src/portModule/portSetting.cpp
        src/portModule/sendModule.cpp
        src/portModule/serialPort.cpp
        src/portModule/sslClient.cpp
        src/portModule/tcpClient.cpp
        src/portModule/tcpServer.cpp
        src/portModule/udpSocket.cpp
        src/portModule/videoStream.cpp
        src/portModule/visa.cpp
        src/scriptModule/nuspellModule.cpp
        src/scriptModule/scriptModule.cpp
        src/scriptModule/scriptPage.cpp
        src/scriptModule/welcomePage.cpp
        src/scriptModule/codeAnalysis/codeAssistant.cpp
        src/scriptModule/codeAnalysis/completionWidget.cpp
        src/scriptModule/codeAnalysis/diagnosticsModule.cpp
        src/scriptModule/codeAnalysis/dwellWidget.cpp
        src/scriptModule/codeAnalysis/navigationWidget.cpp
        src/scriptModule/codeAnalysis/positionWidget.cpp
        src/scriptModule/codeAnalysis/signatureWidget.cpp
        src/scriptModule/codeAnalysis/structureModule.cpp
        src/scriptModule/codeDebug/breakpointModule.cpp
        src/scriptModule/codeDebug/debugModule.cpp
        src/scriptModule/codeDebug/threadpoolModule.cpp
        src/scriptModule/codeDebug/watchModule.cpp
        src/scriptModule/codeEditor/editorWidget.cpp
        src/scriptModule/codeEditor/explorerModule.cpp
        src/scriptModule/codeEditor/searchWidget.cpp
        src/settingModule/fontSettingLog.cpp
        src/settingModule/fontSettingScript.cpp
        src/settingModule/indicatorSettingScript.cpp
        src/settingModule/markerSettingScript.cpp
        src/settingModule/settingModule.cpp
        src/undoModule.cpp
        src/utils/cmarkUtils.cpp
        src/utils/cvUtils.cpp
        src/utils/luaUtils.cpp
        src/utils/qtUtils.cpp
        src/utils/suffixUtils.cpp
)

qt_add_resources(UniComm "qml_resources"
        PREFIX "/qml"
        BASE "qml"
        FILES
        "qml/logModule.qml"
        "qml/dataModule/databaseModule.qml"
        "qml/dataModule/dataplotModule.qml"
        "qml/dataModule/datatableModule.qml"
        "qml/mainWindow/mainWindow.qml"
        "qml/mainWindow/menuModule.qml"
        "qml/mainWindow/statusModule.qml"
        "qml/portModule/portModule.qml"
        "qml/portModule/portSetting.qml"
        "qml/portModule/sendModule.qml"
        "qml/scriptModule/welcomePage.qml"
        "qml/scriptModule/codeAnalysis/diagnosticsModule.qml"
        "qml/scriptModule/codeAnalysis/structureModule.qml"
        "qml/scriptModule/codeDebug/breakpointModule.qml"
        "qml/scriptModule/codeDebug/debugModule.qml"
        "qml/scriptModule/codeDebug/threadpoolModule.qml"
        "qml/scriptModule/codeDebug/watchModule.qml"
        "qml/scriptModule/codeEditor/explorerModule.qml"
)

qt_add_resources(UniComm "icon_resources"
        PREFIX "/icon"
        BASE "resources/icon"
        FILES
        "resources/icon/icon.ico"
        "resources/icon/activateBreakpoints.svg"
        "resources/icon/add.svg"
        "resources/icon/apps.svg"
        "resources/icon/arrowClockwise.svg"
        "resources/icon/arrowCollapse.svg"
        "resources/icon/arrowDown.svg"
        "resources/icon/arrowExpand.svg"
        "resources/icon/arrowRight.svg"
        "resources/icon/arrowUp.svg"
        "resources/icon/autoFitHeight.svg"
        "resources/icon/autoResize.svg"
        "resources/icon/barGraph.svg"
        "resources/icon/brush.svg"
        "resources/icon/bug.svg"
        "resources/icon/checkmarkCircle.svg"
        "resources/icon/clock.svg"
        "resources/icon/close.svg"
        "resources/icon/collapse.svg"
        "resources/icon/contrast.svg"
        "resources/icon/copy.svg"
        "resources/icon/crop.svg"
        "resources/icon/database.svg"
        "resources/icon/debugContinue.svg"
        "resources/icon/debugStepInto.svg"
        "resources/icon/debugStepOut.svg"
        "resources/icon/debugStepOver.svg"
        "resources/icon/definition.svg"
        "resources/icon/delete.svg"
        "resources/icon/document.svg"
        "resources/icon/drag.svg"
        "resources/icon/edit.svg"
        "resources/icon/equalCircle.svg"
        "resources/icon/eraser.svg"
        "resources/icon/error.svg"
        "resources/icon/expand.svg"
        "resources/icon/eye.svg"
        "resources/icon/fileTypeCsv.svg"
        "resources/icon/fileTypeImage.svg"
        "resources/icon/fileTypeJson.svg"
        "resources/icon/fileTypeLua.svg"
        "resources/icon/fold.svg"
        "resources/icon/folder.svg"
        "resources/icon/function.svg"
        "resources/icon/github.svg"
        "resources/icon/hint.svg"
        "resources/icon/home.svg"
        "resources/icon/implementation.svg"
        "resources/icon/info.svg"
        "resources/icon/lineGraph.svg"
        "resources/icon/lockClosed.svg"
        "resources/icon/matchCase.svg"
        "resources/icon/more.svg"
        "resources/icon/null.svg"
        "resources/icon/open.svg"
        "resources/icon/pause.svg"
        "resources/icon/play.svg"
        "resources/icon/quadrilateral.svg"
        "resources/icon/rectangle.svg"
        "resources/icon/reference.svg"
        "resources/icon/refresh.svg"
        "resources/icon/regExp.svg"
        "resources/icon/rename.svg"
        "resources/icon/reset.svg"
        "resources/icon/resize.svg"
        "resources/icon/roi.svg"
        "resources/icon/save.svg"
        "resources/icon/saveAs.svg"
        "resources/icon/send.svg"
        "resources/icon/serialPort.svg"
        "resources/icon/settings.svg"
        "resources/icon/share.svg"
        "resources/icon/snooze.svg"
        "resources/icon/stop.svg"
        "resources/icon/symbolArray.svg"
        "resources/icon/symbolBoolean.svg"
        "resources/icon/symbolConstant.svg"
        "resources/icon/symbolEnum.svg"
        "resources/icon/symbolEnumMember.svg"
        "resources/icon/symbolEvent.svg"
        "resources/icon/symbolField.svg"
        "resources/icon/symbolFile.svg"
        "resources/icon/symbolKeyword.svg"
        "resources/icon/symbolMethod.svg"
        "resources/icon/symbolMisc.svg"
        "resources/icon/symbolNumeric.svg"
        "resources/icon/symbolPackage.svg"
        "resources/icon/symbolSnippet.svg"
        "resources/icon/symbolString.svg"
        "resources/icon/symbolVariable.svg"
        "resources/icon/table.svg"
        "resources/icon/tcpClient.svg"
        "resources/icon/tcpServer.svg"
        "resources/icon/typeDefinition.svg"
        "resources/icon/udpSocket.svg"
        "resources/icon/video.svg"
        "resources/icon/warning.svg"
        "resources/icon/wholeWord.svg"
        "resources/icon/wordStart.svg"
        "resources/icon/wrap.svg"
)

qt_add_resources(UniComm "config_resources"
        PREFIX "/config"
        BASE "resources/config"
        FILES
        "resources/config/.luarc.json"
        "resources/config/config.json"
        "resources/config/lib.d.lua"
)

set(UNICOMM_INCLUDE "${CMAKE_SOURCE_DIR}/include")

set(QT_DIR "${CMAKE_SOURCE_DIR}/3rd/qt-6.10.1")
set(QT_BIN "${QT_DIR}/bin")
set(QT_PLUGINS
        "generic"
        "iconengines"
        "imageformats"
        "multimedia"
        "networkinformation"
        "platforms"
        "qml"
        "qmltooling"
        "styles"
        "texttospeech"
        "tls"
        "translations")

set(MISC_DIR "${CMAKE_SOURCE_DIR}/3rd/misc")
set(MISC_BIN "${MISC_DIR}/bin")

#3rd alphabetical order
set(CMARK_DIR "${CMAKE_SOURCE_DIR}/3rd/cmark-0.31.1")
set(CMARK_INCLUDE "${CMARK_DIR}/include")
file(GLOB CMARK_LIB "${CMARK_DIR}/lib/*.a")
set(CMARK_BIN "${CMARK_DIR}/bin")

set(DRMINGW_DIR "${CMAKE_SOURCE_DIR}/3rd/drmingw-0.9.11")
set(DRMINGW_INCLUDE "${DRMINGW_DIR}/include")
file(GLOB DRMINGW_LIB "${DRMINGW_DIR}/lib/*.a")
set(DRMINGW_BIN "${DRMINGW_DIR}/bin")

set(ICU_DIR "${CMAKE_SOURCE_DIR}/3rd/icu-78.1")
set(ICU_INCLUDE "${ICU_DIR}/include")
file(GLOB ICU_LIB "${ICU_DIR}/lib/*.a")
set(ICU_BIN "${ICU_DIR}/bin")

set(KDDW_DIR "${CMAKE_SOURCE_DIR}/3rd/kddw-2.4.0")
set(KDDW_INCLUDE "${KDDW_DIR}/include")
file(GLOB KDDW_LIB "${KDDW_DIR}/lib/*.a")
set(KDDW_BIN "${KDDW_DIR}/bin")

set(LEPTONICA_DIR "${CMAKE_SOURCE_DIR}/3rd/leptonica-1.86.0")
set(LEPTONICA_INCLUDE "${LEPTONICA_DIR}/include")
file(GLOB LEPTONICA_LIB "${LEPTONICA_DIR}/lib/*.a")
set(LEPTONICA_BIN "${LEPTONICA_DIR}/bin")

set(LUA_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-5.4.8")
#set(LUA_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-5.5.0")
set(LUA_INCLUDE "${LUA_DIR}/include")
file(GLOB LUA_LIB "${LUA_DIR}/lib/*.a")
set(LUA_BIN "${LUA_DIR}/bin")

set(LUALS_DIR "${CMAKE_SOURCE_DIR}/3rd/lua-language-server-3.16.4")

set(NUSPELL_DIR "${CMAKE_SOURCE_DIR}/3rd/nuspell-5.1.6")
set(NUSPELL_INCLUDE "${NUSPELL_DIR}/include")
file(GLOB NUSPELL_LIB "${NUSPELL_DIR}/lib/*.a")
set(NUSPELL_BIN "${NUSPELL_DIR}/bin")
set(NUSPELL_DICT "${NUSPELL_DIR}/dict")

set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/3rd/opencv-4.12.0")
set(OPENCV_INCLUDE "${OPENCV_DIR}/include")
file(GLOB OPENCV_LIB "${OPENCV_DIR}/lib/*.a")
set(OPENCV_BIN "${OPENCV_DIR}/bin")

set(QSCINTILLA_DIR "${CMAKE_SOURCE_DIR}/3rd/qscintilla-2.14.1")
set(QSCINTILLA_INCLUDE "${QSCINTILLA_DIR}/include")
file(GLOB QSCINTILLA_LIB "${QSCINTILLA_DIR}/lib/*.a")
set(QSCINTILLA_BIN "${QSCINTILLA_DIR}/bin")

set(SCINTILLA_DIR "${CMAKE_SOURCE_DIR}/3rd/scintilla-5.5.9")
set(SCINTILLA_INCLUDE "${SCINTILLA_DIR}/include")
file(GLOB SCINTILLA_LIB "${SCINTILLA_DIR}/lib/*.a")
set(SCINTILLA_BIN "${SCINTILLA_DIR}/bin")

set(SOL2_DIR "${CMAKE_SOURCE_DIR}/3rd/sol2-3.5.0")
set(SOL2_INCLUDE "${SOL2_DIR}/include")

set(TESSERACT_DIR "${CMAKE_SOURCE_DIR}/3rd/tesseract-5.5.1")
set(TESSERACT_INCLUDE "${TESSERACT_DIR}/include")
file(GLOB TESSERACT_LIB "${TESSERACT_DIR}/lib/*.a")
set(TESSERACT_BIN "${TESSERACT_DIR}/bin")
set(TESSERACT_LANG "${TESSERACT_DIR}/lang")

#???
set(VISA_DIR "${CMAKE_SOURCE_DIR}/3rd/visa")
set(VISA_INCLUDE "${VISA_DIR}/include")
file(GLOB VISA_LIB "${VISA_DIR}/bin/*.dll")
set(VISA_BIN "${VISA_DIR}/bin")

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
        ${QSCINTILLA_INCLUDE}
        ${SOL2_INCLUDE}
        ${TESSERACT_INCLUDE}
        ${VISA_INCLUDE}
)

target_link_libraries(UniComm PRIVATE
        Qt6::Core
        Qt6::Core5Compat
        Qt6::Gui
        Qt6::Graphs
        Qt6::Multimedia
        Qt6::Network
        Qt6::PrintSupport
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::QuickWidgets
        Qt6::SerialPort
        Qt6::Svg
        Qt6::TextToSpeech
        Qt6::Widgets
        ${CMARK_LIB}
        ${DRMINGW_LIB}
        ${ICU_LIB}
        ${KDDW_LIB}
        ${LEPTONICA_LIB}
        ${LUA_LIB}
        ${NUSPELL_LIB}
        ${OPENCV_LIB}
        ${QSCINTILLA_LIB}
        ${TESSERACT_LIB}
        #???
        ${VISA_LIB}
)

target_compile_definitions(UniComm PRIVATE
        KDDW_FRONTEND_QT
        KDDW_FRONTEND_QTWIDGETS
)

target_compile_options(UniComm PRIVATE
        -g
)

#auto deploy(do not use)
#add_custom_command(TARGET UniComm POST_BUILD
#        COMMAND Qt6::windeployqt
#        --qmldir "${CMAKE_SOURCE_DIR}/qml"
#        "$<TARGET_FILE:UniComm>")
#qt-6.10.1
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "QT BIN COPIED")
foreach(QT_PLUGIN ${QT_PLUGINS})
    add_custom_command(TARGET UniComm POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_DIR}/${QT_PLUGIN}"
            "${CMAKE_BINARY_DIR}/${QT_PLUGIN}"
            COMMENT "${QT_PLUGIN} PLUGIN COPIED")
endforeach ()

#misc
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${MISC_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "MISC BIN COPIED")
#cmark-0.31.1
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMARK_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "CMARK BIN COPIED")
#drmingw-0.9.11
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${DRMINGW_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "DRMINGW BIN COPIED")
#icu-78.1
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
#leptonica-1.86.0
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${LEPTONICA_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "LEPTONICA BIN COPIED")
#lua-5.4.8
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${LUA_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "LUA BIN COPIED")
#lua-language-server-3.16.4
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${LUALS_DIR}"
        "${CMAKE_BINARY_DIR}/lua-language-server"
        COMMENT "LUALS DIR COPIED")
#nuspell-5.1.6
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${NUSPELL_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "NUSPELL BIN COPIED")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${NUSPELL_DICT}"
        "${CMAKE_BINARY_DIR}/dict"
        COMMENT "NUSPELL DICT COPIED")
#opencv-4.12.0
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${OPENCV_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "OPENCV BIN COPIED")
#qscintilla-2.14.1
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QSCINTILLA_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "QSCINTILLA BIN COPIED")
#scintilla-5.5.9
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SCINTILLA_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "SCINTILLA BIN COPIED")
#tesseract-5.5.1
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TESSERACT_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "TESSERACT BIN COPIED")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TESSERACT_LANG}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "TESSERACT LANG COPIED")
#visa
add_custom_command(TARGET UniComm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${VISA_BIN}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "VISA BIN COPIED")