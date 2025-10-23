<h1 align="center">
    UniComm2
</h1>

<div align="center">

<img src="resources/icon/icon.ico" alt="icon" width="128">

</div>

<div align="center">

<a href="https://github.com/zyt20001205/UniComm2" target="_blank">GitHub</a>

</div>

<p align="center">
    A programmable communication debugging tool for multiple protocols
</p>

<div align="center">

[![GitHub release](https://img.shields.io/github/v/release/zyt20001205/UniComm2?color=%2334D058&label=Version)](https://github.com/zyt20001205/UniComm2/releases)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Qt](https://img.shields.io/badge/Qt-6.9.1-green)]()
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)]()

</div>

# Port Module

## Architecture

```mermaid
flowchart LR
    port[Port]

    subgraph Lua Basic APIS
        portControl[Port Control]
        portIO[Port IO]
    end

    subgraph Lua Service APIS
        modbus[Modbus]
    end

    port --> portControl & portIO
    portIO --> modbus
```

## Support Port Types

<table>
    <tr>
        <th colspan="4">OSI Model</th>
    </tr>
    <tr>
        <td>Application</td>
        <td colspan="2"></td>
        <td>Modbus</td>
    </tr>
    <tr>
        <td>Presentation</td>
        <td colspan="2"></td>
        <td></td>
    </tr>
    <tr>
        <td>Session</td>
        <td colspan="2"></td>
        <td></td>
    </tr>
    <tr>
        <td>Transport</td>
        <td>TCP</td>
        <td>UDP</td>
        <td></td>
    </tr>
    <tr>
        <td>Network</td>
        <td colspan="2">IP</td>
        <td></td>
    </tr>
    <tr>
        <td>Data Link</td>
        <td colspan="2">Ethernet</td>
        <td>Serial Framing</td>
    </tr>
    <tr>
        <td>Physical</td>
        <td colspan="2">RJ45</td>
        <td>Serial Port</td>
    </tr>
</table>

## Port Control APIS

|    APIS    |                             Serial Port                             |                             Tcp Client                              |                             Tcp Server                              |                             Udp Socket                              |                               Screen                                |                               Camera                                |
|:----------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|
| port.open  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.close | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.info  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |

## Port IO APIS

|      APIS      |                             Serial Port                             |                             Tcp Client                              |                             Tcp Server                              |                                 Udp Socket                                 |                               Screen                                |                               Camera                                |
|:--------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:--------------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|
| port.readData  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |
| port.writeData | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |
| port.readText  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.writeText | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |

## Modbus APIS

| Function Code & Name                             | RTU Mode                                                            | ASCII Mode                                             |
|:-------------------------------------------------|:--------------------------------------------------------------------|:-------------------------------------------------------|
| 01 (0x01) Read Coils                             | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 02 (0x02) Read Discrete Inputs                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 03 (0x03) Read Holding Registers                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 04 (0x04) Read Input Registers                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 05 (0x05) Write Single Coil                      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 06 (0x06) Write Single Register                  | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 08 (0x08) Diagnostics                            | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 11 (0x0B) Get Comm Event Counter                 | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 15 (0x0F) Write Multiple Coils                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 16 (0x10) Write Multiple Registers               | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 17 (0x11) Report Server ID                       | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 22 (0x16) Mask Write Register                    | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 23 (0x17) Read/Write Multiple Registers          | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |
| 43 / 14 (0x2B / 0x0E) Read Device Identification | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow) |

# Script Module

## Architecture

```mermaid
flowchart LR
    luaLanguageServer[LuaLanguageServer]
    scriptModule[Script Module]
    subgraph qScintilla[QScintilla]
        direction LR
        scriptPage1[Script Page 1]
        scriptPage2[Script Page 2]
        scriptPage3[Script Page 3]
        scriptPage4[...]
    end
    scriptModule e1@ -->|LSP Request| luaLanguageServer
    scriptModule <==>|LSP Notification| luaLanguageServer
    luaLanguageServer e2@ -->|LSP Response| scriptModule
    scriptModule --- qScintilla
    e1@{animate: true}
    e2@{animate: true}
```

## Editor Features

| Feature                 | Status                                                              |
|:------------------------|:--------------------------------------------------------------------|
| comment (Ctrl+/)        | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| duplicate line (Ctrl+D) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| auto pair ( [ { \" '    | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| search                  | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| replace                 | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |

## Supported LSP Specifications

| LSP Specification                                                  | Type         | Status                                                                     |
|:-------------------------------------------------------------------|:-------------|:---------------------------------------------------------------------------|
| textDocument/didChange                                             | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| textDocument/didClose                                              | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| textDocument/didOpen                                               | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| textDocument/didSave                                               | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| [textDocument/publishDiagnostics](#textdocumentpublishdiagnostics) | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| workspace/didChangeWorkspaceFolders                                | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| textDocument/codeAction                                            | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| textDocument/codeLens                                              | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| [textDocument/completion](#textdocumentcompletion)                 | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| [textDocument/definition](#textdocumentdefinition)                 | Request      | ![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) |
| textDocument/documentHighlight                                     | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| [textDocument/documentSymbol](#textdocumentdocumentsymbol)         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| [textDocument/foldingRange](#textdocumentfoldingrange)             | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| [textDocument/formatting](#textdocumentformatting)                 | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| [textDocument/hover](#textdocumenthover)                           | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| textDocument/implementation                                        | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| textDocument/onTypeFormatting                                      | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| textDocument/rangeFormatting                                       | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| textDocument/references                                            | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| textDocument/rename                                                | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |
| [textDocument/semanticTokens](#textdocumentsemantictokens)         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| [textDocument/signatureHelp](#textdocumentsignaturehelp)           | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)        |
| textDocument/typeDefinition                                        | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                     |

### textDocument/publishDiagnostics

![publishDiagnostics](resources/assets/lsp/publishDiagnostics.gif)

### textDocument/completion

![completion](resources/assets/lsp/completion.gif)

### textDocument/definition

![definition](resources/assets/lsp/definition.gif)

### textDocument/documentSymbol

![documentSymbol](resources/assets/lsp/documentSymbol.gif)

### textDocument/foldingRange

![foldingRange](resources/assets/lsp/foldingRange.gif)

### textDocument/formatting

Ctrl+Alt+L

![formatting](resources/assets/lsp/formatting.gif)

### textDocument/hover

![hover](resources/assets/lsp/hover.gif)

### textDocument/semanticTokens

![semanticTokens](resources/assets/lsp/semanticTokens.png)

### textDocument/signatureHelp

![signatureHelp](resources/assets/lsp/signatureHelp.gif)

## Debug Features

| Feature                                           | Status                                                              |
|:--------------------------------------------------|:--------------------------------------------------------------------|
| [stop](#stop)                                     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| [resume](#resume)                                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| [pause](#pause)                                   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| [step over](#step-over)                           | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| [step into](#step-into)                           | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| [step out](#step-out)                             | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| [run to cursor](#run-to-cursor)                   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
|                                                   |                                                                     |
| [breakpoint console](#breakpoint-console)         | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [conditional breakpoint](#conditional-breakpoint) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [variable watch](#variable-operation)             | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [variable hot update](#variable-operation)        | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [callstack](#callstack)                           | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

### stop

![stop](resources/assets/debug/stop.gif)

### resume

![resume](resources/assets/debug/resume.gif)

### pause

![pause](resources/assets/debug/pause.gif)

### step over

![stepOver](resources/assets/debug/stepOver.gif)

### step into

![stepInto](resources/assets/debug/stepInto.gif)

### step out

![stepOut](resources/assets/debug/stepOut.gif)

### run to cursor

![runToCursor](resources/assets/debug/runToCursor.gif)

### breakpoint console

![breakpointConsole](resources/assets/debug/breakpointConsole.gif)

### conditional breakpoint

![conditionalBreakpoint](resources/assets/debug/conditionalBreakpoint.gif)

### variable operation

![variableOperation](resources/assets/debug/variableOperation.gif)

### callstack

![callstack](resources/assets/debug/callstack.gif)