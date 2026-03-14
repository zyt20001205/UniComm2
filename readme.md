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
    A programmable communication debugging platform for multiple protocols
</p>

<div align="center">

[![GitHub release](https://img.shields.io/github/v/release/zyt20001205/UniComm2?color=%2334D058&label=Version)](https://github.com/zyt20001205/UniComm2/releases)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Qt](https://img.shields.io/badge/Qt-6.10.1-green)]()
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)]()

</div>

<div align="center">

<img src="resources/assets/screenshot/preview.png" alt="preview">

</div>

```mermaid
flowchart RL
    subgraph Lua Language Server
        LuaLS[LuaLS]
    end

    subgraph UniComm
        direction LR
        scriptModule[Script Module]
        luaInterpreter[Lua Interpreter]
        debugModule[Debug Module]
        subgraph threadpoolModule[Threadpool Module]
            direction LR
            luaInterpreter1["Lua Interpreter 1<br>(run)"]
            luaInterpreter2["Lua Interpreter 2<br>(run)"]
            luaInterpreter3["Lua Interpreter 3<br>(debug)"]
            more["..."]
        end
        portModule[Port Module]
        subgraph dataModule[Data Module]
            direction LR
            databaseModule["Database"]
            datatableModule["Data Table"]
            dataplotModule["Data Plot"]
        end
    end

    UniComm -->|LSP Request| LuaLS
    LuaLS <==>|LSP Notification| UniComm
    LuaLS -->|LSP Response| UniComm
%%    UniComm e1@ -->|LSP Request| LuaLS
%%    LuaLS <==>|LSP Notification| UniComm
%%    LuaLS e2@ -->|LSP Response| UniComm
%%    e1@{animate: true}
%%    e2@{animate: true}
    scriptModule -->|run/debug signal| threadpoolModule
    luaInterpreter -->|instantiation| threadpoolModule
    debugModule <-->|debug session| threadpoolModule
    threadpoolModule <-->|port control| portModule
    threadpoolModule <-->|dataflow| dataModule

```

```mermaid
gantt
    title Development Schedule 2026
    dateFormat MM-DD
    axisFormat %m-%d
    tickInterval 1month

    section version
        v0.1.0: done, 02-10, 1d
        v0.2.0-alpha1: active, 02-11, 48d
        v0.2.0-alpha2: 03-31, 30d
        v0.2.0-alpha3: 04-30, 30d

    section coding
        scintilla migration: done, 02-11, 30d
        status bar: done, 3d
        textDocument/rename: 03-31, 30d
        search bar: 03-31, 30d
        git integration: 04-30, 30d

    section infra
        ringbuffer class: active, 03-01, 15d
        ringbuffer apis: 15d

    section customize
        setting window: 03-31, 30d
```

# APIS

## Port Communication

### [Base APIS](#base-apis)

### [Modbus APIS](#modbus-apis)

### [SMTP APIS](#smtp-apis)

## Data Process

### [Database APIS](#database-apis)

### [Datatable APIS](#datatable-apis)

# Port Module

## Support Port Types

<table align = "center">
    <tr>
        <th colspan="7">OSI Model</th>
    </tr>
    <tr>
        <td>Application</td>
        <td colspan="2"></td>
        <td align = "center">Modbus</td>
        <td align = "center">USB TMC</td>
        <td align = "center" colspan="2">OCR</td>
    </tr>
    <tr>
        <td>Presentation</td>
        <td colspan="2"></td>
        <td colspan="2"></td>
        <td align = "center" colspan="2"><a href ="#video-stream">Video Stream</a></td>
    </tr>
    <tr>
        <td>Session</td>
        <td colspan="2"></td>
        <td colspan="2"></td>
        <td colspan="2"></td>
    </tr>
    <tr>
        <td>Transport</td>
        <td align = "center"><a href ="#tcp">TCP</a></td>
        <td align = "center"><a href ="#udp">UDP</a></td>
        <td colspan="2"></td>
        <td colspan="2"></td>
    </tr>
    <tr>
        <td>Network</td>
        <td align = "center" colspan="2">IP</td>
        <td colspan="2"></td>
        <td colspan="2"></td>
    </tr>
    <tr>
        <td>Data Link</td>
        <td align = "center" colspan="2">Ethernet</td>
        <td align = "center" colspan="2">Serial Framing</td>
        <td colspan="2"></td>
    </tr>
    <tr>
        <td>Physical</td>
        <td align = "center" colspan="2">RJ45</td>
        <td align = "center" colspan="2"><a href ="#serial-port">Serial Port</a></td>
        <td align = "center">Screen</td>
        <td align = "center">Camera</td>
    </tr>
</table>

## Base APIS

|    APIS    |                             Serial Port                             |                             Tcp Client                              |                             Tcp Server                              |                                 Udp Socket                                 |                               Screen                                |                               Camera                                |
|:----------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:--------------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|
| port.open  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.close | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.info  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
| port.read  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.write | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |

## Modbus APIS

| Modbus Protocol                                  |                                  APIS                                  | RTU Mode                                                            | ASCII Mode                                                          |
|:-------------------------------------------------|:----------------------------------------------------------------------:|:--------------------------------------------------------------------|:--------------------------------------------------------------------|
| 01 (0x01) Read Coils                             |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 02 (0x02) Read Discrete Inputs                   |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 03 (0x03) Read Holding Registers                 |      modbusRtu.readHoldRegisters<br>modbusAscii.readHoldRegisters      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| 04 (0x04) Read Input Registers                   |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 05 (0x05) Write Single Coil                      |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 06 (0x06) Write Single Register                  |    modbusRtu.writeSingleRegister<br>modbusAscii.writeSingleRegister    | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| 08 (0x08) Diagnostics                            |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 11 (0x0B) Get Comm Event Counter                 |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 15 (0x0F) Write Multiple Coils                   |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 16 (0x10) Write Multiple Registers               | modbusRtu.writeMultipleRegisters<br>modbusAscii.writeMultipleRegisters | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| 17 (0x11) Report Server ID                       |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 22 (0x16) Mask Write Register                    |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 23 (0x17) Read/Write Multiple Registers          |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 43 / 14 (0x2B / 0x0E) Read Device Identification |                                                                        | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |

## SMTP APIS

|   RFC 4954    |      APIS      |                               Status                                |                           
|:-------------:|:--------------:|:-------------------------------------------------------------------:|
|  AUTH LOGIN   | smtp.authLogin | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
|  AUTH PLAIN   |                |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        | 
| AUTH CRAM-MD5 |                |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        | 

|               RFC 5321                |   APIS    |                               Status                                |                           
|:-------------------------------------:|:---------:|:-------------------------------------------------------------------:|
| Extended HELLO (EHLO) or HELLO (HELO) | smtp.ehlo | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
|              MAIL (MAIL)              | smtp.mail | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
|           RECIPIENT (RCPT)            |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|              DATA (DATA)              |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|             RESET (RSET)              |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|             VERIFY (VRFY)             |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|             EXPAND (EXPN)             |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|              HELP (HELP)              |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|              NOOP (NOOP)              |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|              QUIT (QUIT)              |           |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |

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
| search                  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| replace                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

## Supported LSP Specifications

| LSP Specification                                                  | Type         | Status                                                              |
|:-------------------------------------------------------------------|:-------------|:--------------------------------------------------------------------|
| [textDocument/publishDiagnostics](#textdocumentpublishdiagnostics) | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/codeAction                                            | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/codeLens                                              | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| [textDocument/completion](#textdocumentcompletion)                 | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/definition](#textdocumentgoto)                       | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/documentHighlight](#textdocumentdocumentHighlight)   | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/documentSymbol](#textdocumentdocumentsymbol)         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/foldingRange](#textdocumentfoldingrange)             | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/formatting](#textdocumentformatting)                 | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/hover](#textdocumenthover)                           | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/implementation](#textdocumentgoto)                   | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/onTypeFormatting                                      | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/rangeFormatting                                       | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/references](#textdocumentgoto)                       | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/rename                                                | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| [textDocument/semanticTokens](#textdocumentsemantictokens)         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/signatureHelp](#textdocumentsignaturehelp)           | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| [textDocument/typeDefinition](#textdocumentgoto)                   | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

### textDocument/publishDiagnostics

![publishDiagnostics](resources/assets/lsp/publishDiagnostics.gif)

### textDocument/completion

![completion](resources/assets/lsp/completion.gif)

### textDocument/documentHighlight

![documentHighlight](resources/assets/lsp/documentHighlight.png)

### textDocument/documentSymbol

![documentSymbol](resources/assets/lsp/documentSymbol.gif)

### textDocument/foldingRange

![foldingRange](resources/assets/lsp/foldingRange.gif)

### textDocument/formatting

Ctrl+Alt+L

![formatting](resources/assets/lsp/formatting.gif)

### textDocument/goto

![goto](resources/assets/lsp/goto.png)

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
| [multithreading](#multithreading)                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

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

### multithreading

![multithreading](resources/assets/debug/multithreading.gif)

# Data Process

## Database Module

### Database APIS

|      APIS      |                               Status                                |                           
|:--------------:|:-------------------------------------------------------------------:|
| database.list  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| database.write | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| database.clear | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

## Datatable Module

### Datatable APIS

|       APIS       |                               Status                                |                           
|:----------------:|:-------------------------------------------------------------------:|
|  datatable.list  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| datatable.write  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| datatable.clear  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| datatable.export | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |