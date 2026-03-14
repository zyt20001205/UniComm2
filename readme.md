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
        <td align = "center" colspan="2">Image Processing</td>
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
        <td align = "center"><a href ="#screen">Screen</a></td>
        <td align = "center"><a href ="#camera">Camera</a></td>
    </tr>
</table>

### Serial Port

| Feature      | Status                                                              |
|--------------|---------------------------------------------------------------------|
| Baud Rate    | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| Data Bits    | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| Parity       | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| Stop Bits    | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| Flow Control | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |

### TCP

| Feature              | Status                                                              |
|----------------------|---------------------------------------------------------------------|
| TCP Client           | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| TCP Server Unicast   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| TCP Server Broadcast | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

### UDP

| Feature    | Status                                                              |
|------------|---------------------------------------------------------------------|
| UDP Socket | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

### Screen

| Feature          | Status                                                              |
|------------------|---------------------------------------------------------------------|
| Area Selection   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| Image Processing | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| OCR              | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| DPI Adapt        | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

### Camera

| Feature          | Status                                                              |
|------------------|---------------------------------------------------------------------|
| Area Selection   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| Image Processing | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| OCR              | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

## APIS

|    APIS    |                             Serial Port                             |                             Tcp Client                              |                             Tcp Server                              |                                 Udp Socket                                 |                               Screen                                |                               Camera                                |
|:----------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:--------------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|
| port.open  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.close | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.info  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |    ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)     |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |       ![WIP](https://img.shields.io/badge/Status-WIP-yellow)        |
| port.read  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| port.write | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |    ![Unsupported](https://img.shields.io/badge/unsupported-red)     |

- [How is data processed in the write method?](#write-method-data-process)

- [How does timeout argument work in the read method?](#difference-between-blocking--non-blocking)

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
| search                  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| replace                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

## Supported LSP Specifications

| LSP Specification                                                  | Type         | Status                                                                      |
|:-------------------------------------------------------------------|:-------------|:----------------------------------------------------------------------------|
| textDocument/didChange                                             | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| textDocument/didClose                                              | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| textDocument/didOpen                                               | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| textDocument/didSave                                               | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/publishDiagnostics](#textdocumentpublishdiagnostics) | Notification | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| textDocument/codeAction                                            | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                      |
| textDocument/codeLens                                              | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                      |
| [textDocument/completion](#textdocumentcompletion)                 | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/definition](#textdocumentgoto)                       | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/documentHighlight](#textdocumentdocumentHighlight)   | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/documentSymbol](#textdocumentdocumentsymbol)         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/foldingRange](#textdocumentfoldingrange)             | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/formatting](#textdocumentformatting)                 | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/hover](#textdocumenthover)                           | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/implementation](#textdocumentgoto)                   | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| textDocument/onTypeFormatting                                      | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| textDocument/rangeFormatting                                       | Request      | ![Not Planned](https://img.shields.io/badge/Status-Not%20Planned-lightgrey) |
| [textDocument/references](#textdocumentgoto)                       | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| textDocument/rename                                                | Request      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)                      |
| [textDocument/semanticTokens](#textdocumentsemantictokens)         | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/signatureHelp](#textdocumentsignaturehelp)           | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |
| [textDocument/typeDefinition](#textdocumentgoto)                   | Request      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)         |

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
| [heatmap](#heatmap)                               | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
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

### heatmap

![heatmap](resources/assets/debug/heatmap.gif)

### multithreading

![multithreading](resources/assets/debug/multithreading.gif)

# Data Process

## Database Module

### APIS

|      APIS      |                               Status                                |                           
|:--------------:|:-------------------------------------------------------------------:|
| database.list  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| database.write | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| database.clear | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |

# FAQ

## write method data process

```mermaid
flowchart RL
    subgraph dataProcessWorkFlow[data process workflow]
        input[/input/]
        write["write()"]
        handleWrite["handleWrite()"]
        output[/output/]
    end

    input --> write
    write -->|reformat\nsuffix| handleWrite
    handleWrite --> output
```

- example: hex & modbus crc

```lua
port.write("Actual Port", "0103 0000 0001")
```

```mermaid
flowchart TB
    input[/0103 0000 0001/]
    step1[010300000001]
    step2["\x01\x03\x00\x00\x00\x01"]
    step3[/"\x01\x03\x00\x00\x00\x01\x84\x0A"/]
    input -->|space removed| step1
    step1 -->|formatted| step2
    step2 -->|suffix appended| step3
```

- example: ascii & crlf

```lua
port.write("Actual Port", "AT+STACH1=1")
```

```mermaid
flowchart TB
    input[/AT+STACH1=1/]
    step1["\x41\x54\x2B\x53\x54\x41\x43\x48\x31\x3D\x31"]
    step2[/"\x41\x54\x2B\x53\x54\x41\x43\x48\x31\x3D\x31"\x0D\x0A/]
    input -->|formatted| step1
    step1 -->|suffix appended| step2
```

## difference between blocking & non-blocking

### blocking dataflow

```lua
port.write("Actual Port", "How are you?")
local rx = port.read("Actual Port", 1000, 6)
```

```mermaid
sequenceDiagram
    participant Lua Thread
    participant Port Thread
    participant Physical Port
    Lua Thread ->> Port Thread: write
    Note over Lua Thread, Port Thread: "How are you?"
    activate Port Thread
    Port Thread ->> Physical Port: dataflow
    deactivate Port Thread
    Note over Port Thread, Physical Port: "/x48/x6F/x77..."
    Lua Thread ->> Port Thread: read
    activate Port Thread
    loop check every 10ms
        Physical Port ->> Port Thread: dataflow
    end
    deactivate Port Thread
    alt time <= 1000ms && length == 6bytes
        Note over Port Thread, Physical Port: "/x47/x72/x65..."
        Port Thread ->> Lua Thread: return
        Note over Lua Thread, Port Thread: "Great!"
        Note over Lua Thread: rx = "Great!"
    else time > 1000ms || length != 6bytes
        Port Thread ->> Lua Thread: return
        Note over Lua Thread, Port Thread: ""
        Note over Lua Thread: rx = ""
    end

```

### non-blocking dataflow

```lua
port.write("Actual Port", "How are you?")
sleep(1000)
local rx = port.read("Actual Port", 0)
```

```mermaid

sequenceDiagram
    participant Lua Thread
    participant Port Thread
    participant Physical Port
    Lua Thread ->> Port Thread: write
    activate Lua Thread
    Note over Lua Thread, Port Thread: "How are you?"
    activate Port Thread
    Port Thread ->> Physical Port: dataflow
    deactivate Port Thread
    Note over Port Thread, Physical Port: "/x48/x6F/x77..."
    Note over Lua Thread: sleep 1000ms
    activate Port Thread
    loop fill buffer with last pack
        Physical Port ->> Port Thread: dataflow 1
        Note over Port Thread, Physical Port: "/x00/x00/x00..."
        Note over Port Thread: buffer = "/x00/x00/x00..."
        Physical Port ->> Port Thread: dataflow 2
        Note over Port Thread, Physical Port: "/x47/x72/x65..."
        Note over Port Thread: buffer = "/x47/x72/x65..."
    end
    deactivate Port Thread
    Port Thread ->> Lua Thread: read
    Note over Lua Thread, Port Thread: rx = buffer
    deactivate Lua Thread

```