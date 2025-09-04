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

# Roadmap

<div align="center">
<img src="resources/sketch/roadmap.svg" alt="roadmap" style="max-width: 100%; height: auto;">
</div>

# Supported Protocols

<style>
    .osi-table td {
        text-align: center;
        vertical-align: middle;
    }
</style>

<table class="osi-table">
    <tr>
        <th colspan="4">OSI Model</th>
    </tr>
    <tr>
        <td>Application</td>
        <td colspan="2"></td>
        <td><a href="#Modbus RTU/ASCII">Modbus RTU/ASCII</a></td>
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
        <td><a href="#TCP">TCP</a></td>
        <td><a href="#UDP">UDP</a></td>
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
        <td><a href="Serial Port">Serial Port</a></td>
    </tr>
</table>

<a id="TCP"></a>
## TCP

<a id="UDP"></a>
## UDP

<a id="Serial Port"></a>
## Serial Port

<a id="Modbus RTU/ASCII"></a>
### Modbus RTU/ASCII

<table>
    <tr>
        <th></th>
        <th>RTU</th>
        <th>ASCII</th>
    </tr>
    <tr>
        <td>01 (0x01) Read Coils</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>02 (0x02) Read Discrete Inputs</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>03 (0x03) Read Holding Registers</td>
        <td>
            <img src="https://img.shields.io/badge/Status-Passing-brightgreen" alt="Test Passing">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-Passing-brightgreen" alt="Test Passing">
        </td>
    </tr>
    <tr>
        <td>04 (0x04) Read Input Registers</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>05 (0x05) Write Single Coil</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>06 (0x06) Write Single Register</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>08 (0x08) Diagnostics</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>11 (0x0B) Get Comm Event Counter</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>15 (0x0F) Write Multiple Coils</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>    
    <tr>
        <td>16 (0x10) Write Multiple Registers</td>
        <td>
            <img src="https://img.shields.io/badge/Status-Passing-brightgreen" alt="Test Passing">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>17 (0x11) Report Server ID</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>22 (0x16) Mask Write Register</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>23 (0x17) Read/Write Multiple Registers</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
    <tr>
        <td>43 / 14 (0x2B / 0x0E) Read Device Identification</td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
        <td>
            <img src="https://img.shields.io/badge/Status-WIP-yellow" alt="WIP">
        </td>
    </tr>
</table>