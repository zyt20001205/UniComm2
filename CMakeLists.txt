cmake_minimum_required(VERSION 3.31)
project(UniComm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH  "C:/MyFileAudit/software/msys64/ucrt64")

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
        Widgets
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(GUI_TYPE WIN32)
else ()
    set(GUI_TYPE "")
endif ()

qt_add_executable(UniComm ${GUI_TYPE}
        resources/icon/app.rc
        include/globals.h
        include/analysis/codeAssistant.h
        include/analysis/completionWidget.h
        include/analysis/diagnosticsModule.h
        include/analysis/dwellWidget.h
        include/analysis/navigationWidget.h
        include/analysis/nuspellModule.h
        include/analysis/positionWidget.h
        include/analysis/signatureWidget.h
        include/analysis/structureModule.h
        include/analysis/symbolWidget.h
        include/api/data.h
        include/api/file.h
        include/api/http.h
        include/api/imap.h
        include/api/io.h
        include/api/key.h
        include/api/modbusAscii.h
        include/api/modbusRtu.h
        include/api/modbusTcp.h
        include/api/mouse.h
        include/api/mqtt.h
        include/api/port.h
        include/api/smtp.h
        include/api/string.h
        include/api/thread.h
        include/core/configManager.h
        include/core/crashHandler.h
        include/core/explorerModule.h
        include/core/fileModule.h
        include/core/globalManager.h
        include/core/undoModule.h
        include/data/databaseModule.h
        include/data/dataplotModule.h
        include/data/datatableModule.h
        include/debug/breakpointModule.h
        include/debug/debugModule.h
        include/debug/watchModule.h
        include/document/documentModule.h
        include/document/module/codeWidget.h
        include/document/module/conflictWidget.h
        include/document/module/editorWidget.h
        include/document/module/resolveWidget.h
        include/document/module/scintillaWidget.h
        include/document/module/searchWidget.h
        include/document/page/basePage.h
        include/document/page/conflictPage.h
        include/document/page/imagePage.h
        include/document/page/luaPage.h
        include/document/page/pdfPage.h
        include/document/page/textPage.h
        include/document/page/welcomePage.h
        include/llm/llmModule.h
        include/llm/module/mcpModule.h
        include/llm/module/toolsModule.h
        include/llm/provider/baseProvider.h
        include/llm/provider/deepseekProvider.h
        include/mainWindow/kddwCustom.h
        include/mainWindow/mainWindow.h
        include/mainWindow/menuModule.h
        include/mainWindow/statusModule.h
        include/port/basePort.h
        include/port/pixmapPreview.h
        include/port/portModule.h
        include/port/portSetting.h
        include/port/sendModule.h
        include/port/serialPort.h
        include/port/sslClient.h
        include/port/tcpClient.h
        include/port/tcpServer.h
        include/port/udpSocket.h
        include/port/videoStream.h
        include/port/visa.h
        include/runtime/luaInterpreter.h
        include/runtime/threadpoolModule.h
        include/service/luaLanguageServer.h
        include/service/ripgrep.h
        include/service/git/gitModule.h
        include/terminal/logModule.h
        include/terminal/terminalModule.h
        include/terminal/page/cmdPage.h
        include/terminal/page/powershellPage.h
        include/terminal/page/terminalPage.h
        include/util/cmarkUtils.h
        include/util/cvUtils.h
        include/util/luaUtils.h
        include/util/qtUtils.h
        include/util/ringBuffer.h
        include/util/suffixUtils.h
        include/util/uniCast.h
        src/globals.cpp
        src/main.cpp
        src/analysis/codeAssistant.cpp
        src/analysis/completionWidget.cpp
        src/analysis/diagnosticsModule.cpp
        src/analysis/dwellWidget.cpp
        src/analysis/navigationWidget.cpp
        src/analysis/nuspellModule.cpp
        src/analysis/positionWidget.cpp
        src/analysis/signatureWidget.cpp
        src/analysis/structureModule.cpp
        src/analysis/symbolWidget.cpp
        src/api/data.cpp
        src/api/file.cpp
        src/api/http.cpp
        src/api/imap.cpp
        src/api/io.cpp
        src/api/key.cpp
        src/api/modbusAscii.cpp
        src/api/modbusRtu.cpp
        src/api/modbusTcp.cpp
        src/api/mouse.cpp
        src/api/mqtt.cpp
        src/api/port.cpp
        src/api/smtp.cpp
        src/api/string.cpp
        src/api/thread.cpp
        src/core/configManager.cpp
        src/core/crashHandler.cpp
        src/core/explorerModule.cpp
        src/core/fileModule.cpp
        src/core/globalManager.cpp
        src/core/undoModule.cpp
        src/data/databaseModule.cpp
        src/data/dataplotModule.cpp
        src/data/datatableModule.cpp
        src/debug/breakpointModule.cpp
        src/debug/debugModule.cpp
        src/debug/watchModule.cpp
        src/document/documentModule.cpp
        src/document/module/codeWidget.cpp
        src/document/module/conflictWidget.cpp
        src/document/module/editorWidget.cpp
        src/document/module/resolveWidget.cpp
        src/document/module/scintillaWidget.cpp
        src/document/module/searchWidget.cpp
        src/document/page/basePage.cpp
        src/document/page/conflictPage.cpp
        src/document/page/imagePage.cpp
        src/document/page/luaPage.cpp
        src/document/page/pdfPage.cpp
        src/document/page/textPage.cpp
        src/document/page/welcomePage.cpp
        src/llm/llmModule.cpp
        src/llm/module/mcpModule.cpp
        src/llm/module/toolsModule.cpp
        src/llm/provider/baseProvider.cpp
        src/llm/provider/deepseekProvider.cpp
        src/mainWindow/kddwCustom.cpp
        src/mainWindow/mainWindow.cpp
        src/mainWindow/menuModule.cpp
        src/mainWindow/statusModule.cpp
        src/port/basePort.cpp
        src/port/pixmapPreview.cpp
        src/port/portModule.cpp
        src/port/portSetting.cpp
        src/port/sendModule.cpp
        src/port/serialPort.cpp
        src/port/sslClient.cpp
        src/port/tcpClient.cpp
        src/port/tcpServer.cpp
        src/port/udpSocket.cpp
        src/port/videoStream.cpp
        src/port/visa.cpp
        src/runtime/luaInterpreter.cpp
        src/runtime/threadpoolModule.cpp
        src/service/luaLanguageServer.cpp
        src/service/ripgrep.cpp
        src/service/git/gitModule.cpp
        src/terminal/logModule.cpp
        src/terminal/terminalModule.cpp
        src/terminal/page/cmdPage.cpp
        src/terminal/page/powershellPage.cpp
        src/terminal/page/terminalPage.cpp
        src/util/cmarkUtils.cpp
        src/util/cvUtils.cpp
        src/util/luaUtils.cpp
        src/util/qtUtils.cpp
        src/util/ringBuffer.cpp
        src/util/suffixUtils.cpp
        src/util/uniCast.cpp
)

qt_add_resources(UniComm "qml_resources"
        PREFIX "/qml"
        BASE "qml"
        FILES
        "qml/analysis/diagnosticsModule.qml"
        "qml/analysis/structureModule.qml"
        "qml/analysis/symbolWidget.qml"
        "qml/core/explorerModule.qml"
        "qml/data/databaseModule.qml"
        "qml/data/dataplotModule.qml"
        "qml/data/datatableModule.qml"
        "qml/debug/breakpointModule.qml"
        "qml/debug/debugModule.qml"
        "qml/debug/watchModule.qml"
        "qml/document/module/imageWidget.qml"
        "qml/document/module/pdfWidget.qml"
        "qml/document/module/resolveWidget.qml"
        "qml/document/module/searchWidget.qml"
        "qml/document/page/welcomePage.qml"
        "qml/llm/llmModule.qml"
        "qml/mainWindow/mainWindow.qml"
        "qml/mainWindow/menuModule.qml"
        "qml/mainWindow/statusModule.qml"
        "qml/port/portModule.qml"
        "qml/port/portSetting.qml"
        "qml/port/sendModule.qml"
        "qml/runtime/threadpoolModule.qml"
        "qml/service/git/gitCommitWindow.qml"
        "qml/service/git/gitPushWindow.qml"
        "qml/service/git/gitModule.qml"
        "qml/terminal/logModule.qml"
        "qml/terminal/page/terminalPage.qml"
)

qt_add_resources(UniComm "icon_resources"
        PREFIX "/icon"
        BASE "resources/icon"
        FILES
        "resources/icon/icon.ico"
        "resources/icon/activateBreakpoints.svg"
        "resources/icon/add.svg"
        "resources/icon/apps.svg"
        "resources/icon/arrowCollapse.svg"
        "resources/icon/arrowDown.svg"
        "resources/icon/arrowExpand.svg"
        "resources/icon/arrowLeft.svg"
        "resources/icon/arrowRepeat.svg"
        "resources/icon/arrowRight.svg"
        "resources/icon/arrowUp.svg"
        "resources/icon/assembly.svg"
        "resources/icon/autoFitHeight.svg"
        "resources/icon/autoResize.svg"
        "resources/icon/breakpoint.svg"
        "resources/icon/breakpointConditional.svg"
        "resources/icon/breakpointDisabled.svg"
        "resources/icon/brush.svg"
        "resources/icon/bug.svg"
        "resources/icon/checkmark.svg"
        "resources/icon/checkmarkCircle.svg"
        "resources/icon/clock.svg"
        "resources/icon/collapse.svg"
        "resources/icon/completion.svg"
        "resources/icon/contrast.svg"
        "resources/icon/copy.svg"
        "resources/icon/crop.svg"
        "resources/icon/cut.svg"
        "resources/icon/database.svg"
        "resources/icon/debugContinue.svg"
        "resources/icon/debugStepInto.svg"
        "resources/icon/debugStepOut.svg"
        "resources/icon/debugStepOver.svg"
        "resources/icon/deepseek.svg"
        "resources/icon/delete.svg"
        "resources/icon/dismiss.svg"
        "resources/icon/dismissDark.svg"
        "resources/icon/dismissLight.svg"
        "resources/icon/document.svg"
        "resources/icon/drag.svg"
        "resources/icon/edit.svg"
        "resources/icon/eraser.svg"
        "resources/icon/error.svg"
        "resources/icon/expand.svg"
        "resources/icon/eye.svg"
        "resources/icon/fileTypeCsv.svg"
        "resources/icon/fileTypeDefault.svg"
        "resources/icon/fileTypeFolder.svg"
        "resources/icon/fileTypeFolderGit.svg"
        "resources/icon/fileTypeFolderIntellij.svg"
        "resources/icon/fileTypeFolderOpen.svg"
        "resources/icon/fileTypeFolderOpenGit.svg"
        "resources/icon/fileTypeFolderOpenIntellij.svg"
        "resources/icon/fileTypeGit.svg"
        "resources/icon/fileTypeImage.svg"
        "resources/icon/fileTypeJson.svg"
        "resources/icon/fileTypeLua.svg"
        "resources/icon/fileTypePdf.svg"
        "resources/icon/fileTypeTxt.svg"
        "resources/icon/fold.svg"
        "resources/icon/folder.svg"
        "resources/icon/gitBranch.svg"
        "resources/icon/gitBranchConflictDark.svg"
        "resources/icon/gitBranchConflictLight.svg"
        "resources/icon/gitCommit.svg"
        "resources/icon/gitFetch.svg"
        "resources/icon/github.svg"
        "resources/icon/gitMerge.svg"
        "resources/icon/gitPull.svg"
        "resources/icon/gitPush.svg"
        "resources/icon/hide.svg"
        "resources/icon/hint.svg"
        "resources/icon/home.svg"
        "resources/icon/info.svg"
        "resources/icon/location.svg"
        "resources/icon/lockDark.svg"
        "resources/icon/lockLight.svg"
        "resources/icon/matchCase.svg"
        "resources/icon/mcp.svg"
        "resources/icon/null.svg"
        "resources/icon/open.svg"
        "resources/icon/paste.svg"
        "resources/icon/pause.svg"
        "resources/icon/play.svg"
        "resources/icon/prohibited.svg"
        "resources/icon/property.svg"
        "resources/icon/quadrilateral.svg"
        "resources/icon/rectangle.svg"
        "resources/icon/redo.svg"
        "resources/icon/regExp.svg"
        "resources/icon/rename.svg"
        "resources/icon/replace.svg"
        "resources/icon/replaceAll.svg"
        "resources/icon/reset.svg"
        "resources/icon/resize.svg"
        "resources/icon/roi.svg"
        "resources/icon/save.svg"
        "resources/icon/saveAs.svg"
        "resources/icon/search.svg"
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
        "resources/icon/tag.svg"
        "resources/icon/tcpClient.svg"
        "resources/icon/tcpServer.svg"
        "resources/icon/themeDark.svg"
        "resources/icon/themeLight.svg"
        "resources/icon/udpSocket.svg"
        "resources/icon/undo.svg"
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
        ${QTKEYCHAIN_INCLUDE}
        ${SCINTILLA_INCLUDE}
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
        Qt6::Pdf
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
        ${QTKEYCHAIN_LIB}
        ${SCINTILLA_LIB}
        ${TESSERACT_LIB}
        #???
        ${VISA_LIB}
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

#auto deploy(do not use)
#add_custom_command(TARGET UniComm POST_BUILD
#        COMMAND Qt6::windeployqt
#        --qmldir "${CMAKE_SOURCE_DIR}/qml"
#        "$<TARGET_FILE:UniComm>")

#qt-6.11.0
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