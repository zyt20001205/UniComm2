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
        v0.2.0-alpha1: active, 02-11, 49d
        v0.2.0-alpha2: 04-01, 30d
        v0.2.0-alpha3: 05-01, 31d

    section coding
        scintilla migration: done, 02-11, 30d
        status bar: done, 03-10, 3d
        assembly view: active, 03-13, 19d
        search bar: 04-01, 30d
        textDocument/rename: 04-11, 20d
        git integration: 05-01, 31d

    section infra
        ringbuffer class: active, 03-01, 16d
        ringbuffer apis: active, 03-17, 15d
        ringbuffer ui: 03-29, 3d
        undo stack: 04-01, 10d

    section customize
        setting window: 04-01, 30d
```

# APIS

## Port Communication

### [Base](#base-apis)

### [Modbus](#modbus-apis)

### [SMTP](#smtp-apis)

## Data Process

### [Database](#database-apis)

### [Datatable](#datatable-apis)

# Port Module

## Support Port Types

<table align = "center">
    <tr>
        <th colspan="4">OSI Model</th>
    </tr>
    <tr>
        <td>Application</td>
        <td></td>
        <td align = "center">
            <img src="https://img.shields.io/badge/Modbus-Supported-brightgreen" alt="Modbus Support">
            <br><img src="https://img.shields.io/badge/SMTP-Supported-brightgreen" alt="SMTP Support">
            <br><img src="https://img.shields.io/badge/USB_TMC-Supported-brightgreen" alt="USB TMC Support">
        </td>
        <td align = "center">
            <img src="https://img.shields.io/badge/OCR-Supported-brightgreen" alt="OCR Support">
        </td>
    </tr>
    <tr>
        <td>Presentation</td>
        <td align = "center">
            <img src="https://img.shields.io/badge/SSL/TLS-Supported-brightgreen" alt="SSL/TLS Support">
        </td>
        <td></td>
        <td align = "center">
            <img src="https://img.shields.io/badge/Video_Stream-Supported-brightgreen" alt="Video Stream Support">
        </td>
    </tr>
    <tr>
        <td>Session</td>
        <td></td>
        <td></td>
        <td></td>
    </tr>
    <tr>
        <td>Transport</td>
        <td align = "center">
            <img src="https://img.shields.io/badge/TCP-Supported-brightgreen" alt="TCP Support">
            <br><img src="https://img.shields.io/badge/UDP-Supported-brightgreen" alt="UDP Support">
        </td>
        <td></td>
        <td></td>
    </tr>
    <tr>
        <td>Network</td>
        <td align = "center">IP</td>
        <td></td>
        <td></td>
    </tr>
    <tr>
        <td>Data Link</td>
        <td align = "center">Ethernet</td>
        <td align = "center">Serial Framing</td>
        <td></td>
    </tr>
    <tr>
        <td>Physical</td>
        <td align = "center">RJ45</td>
        <td align = "center">
            <img src="https://img.shields.io/badge/Serial_Port-Supported-brightgreen" alt="Serial Port Support">
        </td>
        <td align = "center">Screen
            <br>Camera</td>
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

|                                  APIS                                  | Modbus Protocol                                  | RTU Mode                                                            | ASCII Mode                                                          |
|:----------------------------------------------------------------------:|:-------------------------------------------------|:--------------------------------------------------------------------|:--------------------------------------------------------------------|
|                                                                        | 01 (0x01) Read Coils                             | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|                                                                        | 02 (0x02) Read Discrete Inputs                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|      modbusRtu.readHoldRegisters<br>modbusAscii.readHoldRegisters      | 03 (0x03) Read Holding Registers                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
|                                                                        | 04 (0x04) Read Input Registers                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|                                                                        | 05 (0x05) Write Single Coil                      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|    modbusRtu.writeSingleRegister<br>modbusAscii.writeSingleRegister    | 06 (0x06) Write Single Register                  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
|                                                                        | 08 (0x08) Diagnostics                            | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|                                                                        | 11 (0x0B) Get Comm Event Counter                 | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|                                                                        | 15 (0x0F) Write Multiple Coils                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| modbusRtu.writeMultipleRegisters<br>modbusAscii.writeMultipleRegisters | 16 (0x10) Write Multiple Registers               | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
|                                                                        | 17 (0x11) Report Server ID                       | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|                                                                        | 22 (0x16) Mask Write Register                    | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|                                                                        | 23 (0x17) Read/Write Multiple Registers          | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
|                                                                        | 43 / 14 (0x2B / 0x0E) Read Device Identification | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |

## SMTP APIS

|      APIS      |   RFC 4954    |                               Status                                |                           
|:--------------:|:-------------:|:-------------------------------------------------------------------:|
| smtp.authLogin |  AUTH LOGIN   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
|                |  AUTH PLAIN   |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        | 
|                | AUTH CRAM-MD5 |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        | 

|   APIS    |               RFC 5321                |                               Status                                |                           
|:---------:|:-------------------------------------:|:-------------------------------------------------------------------:|
| smtp.ehlo | Extended HELLO (EHLO) or HELLO (HELO) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| smtp.mail |              MAIL (MAIL)              | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
|           |           RECIPIENT (RCPT)            |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|           |              DATA (DATA)              |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|           |             RESET (RSET)              |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|           |             VERIFY (VRFY)             |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|           |             EXPAND (EXPN)             |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|           |              HELP (HELP)              |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|           |              NOOP (NOOP)              |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
|           |              QUIT (QUIT)              |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |

# Script Module

## Architecture

```mermaid
flowchart LR
    luaLanguageServer[Lua Language Server 3.16.4]
    scriptModule[Script Module]
    subgraph scintilla[Scintilla 5.5.9]
        direction LR
        scriptPage1[Script Page 1]
        scriptPage2[Script Page 2]
        scriptPage3[Script Page 3]
        scriptPage4[...]
    end
    scriptModule e1@ -->|LSP Request| luaLanguageServer
    scriptModule <==>|LSP Notification| luaLanguageServer
    luaLanguageServer e2@ -->|LSP Response| scriptModule
    scriptModule --- scintilla
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

| LSP Specification               | Type         | Status                                                              |
|:--------------------------------|:-------------|:--------------------------------------------------------------------|
| textDocument/publishDiagnostics | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/codeAction         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/codeLens           | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| textDocument/completion         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/definition         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/documentHighlight  | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/documentSymbol     | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/foldingRange       | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/formatting         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/hover              | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/implementation     | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/onTypeFormatting   | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/rangeFormatting    | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/references         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/rename             | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| textDocument/semanticTokens     | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/signatureHelp      | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/typeDefinition     | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

## Debug Features

| Feature                | Status                                                              |
|:-----------------------|:--------------------------------------------------------------------|
| stop                   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| resume                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| pause                  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| step over              | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| step into              | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| step out               | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| run to cursor          | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
|                        |                                                                     |
| breakpoint console     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| conditional breakpoint | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| variable watch         | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| variable hot update    | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| callstack              | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| multithreading         | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

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