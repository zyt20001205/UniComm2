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

|    APIS    |                             Serial Port                             |                             Tcp Client                              |                             Tcp Server                              |                             Udp Socket                              | Screen | Camera |
|:----------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:------:|:------:|
| port.open  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |        |        |
| port.close | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |        |        |
| port.info  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |        |        |

## Port IO APIS

|      APIS      |                             Serial Port                             |                             Tcp Client                              |                             Tcp Server                              |                                       Udp Socket                                       | Screen | Camera |
|:--------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:-------------------------------------------------------------------:|:--------------------------------------------------------------------------------------:|:------:|:------:|
| port.readData  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | (ONLY ASYNC)![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) |        |        |
| port.writeData | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |          ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)           |        |        |
| port.readText  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | (ONLY ASYNC)![Partial Pass](https://img.shields.io/badge/Status-Partial%20Pass-yellow) |        |        |
| port.writeText | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |          ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen)           |        |        |

## Modbus APIS

| Function Code & Name                             | RTU Mode                                                            | ASCII Mode                                                          |
|:-------------------------------------------------|:--------------------------------------------------------------------|:--------------------------------------------------------------------|
| 01 (0x01) Read Coils                             | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 02 (0x02) Read Discrete Inputs                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 03 (0x03) Read Holding Registers                 | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| 04 (0x04) Read Input Registers                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 05 (0x05) Write Single Coil                      | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 06 (0x06) Write Single Register                  | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 08 (0x08) Diagnostics                            | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 11 (0x0B) Get Comm Event Counter                 | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 15 (0x0F) Write Multiple Coils                   | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 16 (0x10) Write Multiple Registers               | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 17 (0x11) Report Server ID                       | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 22 (0x16) Mask Write Register                    | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 23 (0x17) Read/Write Multiple Registers          | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| 43 / 14 (0x2B / 0x0E) Read Device Identification | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |

# Script Module

## LSP Integration

```mermaid
flowchart LR
    A[Script Module]
    B[LuaLanguageServer]
    B -->|LSP Response| A
    A -->|LSP Request| B
    A -->|LSP Notification| B
```

| LSP Request Type            | Status                                                              |
|:----------------------------|:--------------------------------------------------------------------|
| textDocument/completion     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/foldingRange   | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/formatting     | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/hover          | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/semanticTokens | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/signatureHelp  | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) |
| textDocument/definition     | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| textDocument/references     | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |

## Editor Features

| Feature                 | Status                                                              |
|:------------------------|:--------------------------------------------------------------------|
| comment (Ctrl+/)        | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| duplicate line (Ctrl+D) | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| auto pair ( [ { \" '    | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| search                  | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| replace                 | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |
| workspace               | ![WIP](https://img.shields.io/badge/Status-WIP-yellow)              |

## Debugging Features

| Feature             | Status                                                              |
|:--------------------|:--------------------------------------------------------------------|
| continue            | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| pause               | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| step into           | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| step over           | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| step out            | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| variable watch      | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
| variable hot update | ![Passing](https://img.shields.io/badge/Status-Passing-brightgreen) | 
