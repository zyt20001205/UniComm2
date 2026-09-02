---@meta

---
---Access configured communication ports through a common I/O interface.
---
---[Port demo](../demo/port.lua)
---
port = {}

---@enum PortType
port.Type = {
    SerialPort = 0,
    TcpClient = 2,
    SslClient = 4,
}

---@alias PortLogFormat
---| "raw"
---| "hex"
---| "ascii"
---| "utf-8"

---@alias PortSuffix
---| "null"
---| "crlf"
---| "modbus crc"
---| "modbus lrc"

---@class PortConfig
---@field portType PortType Port type selected from `port.Type`.
---@field portName string Unique name used by `port.open`, `port.write`, and the other port APIs.

---@class SerialPortConfig : PortConfig
---@field baudRate? integer (default: 115200) Baud rate from 1 to 5000000.
---@field dataBits? 5|6|7|8 (default: 8)
---@field parity? 0|2|3|4|5 (default: 0) 0=none, 2=even, 3=odd, 4=space, 5=mark.
---@field stopBits? 1|2|3 (default: 1) 1=one, 2=two, 3=one and a half.
---@field logFormat? PortLogFormat (default: "utf-8") Format used to render both transmitted and received data in the port log.
---@field txSuffix? PortSuffix (default: "null")
---@field bufferSize? integer (default: 65536) Receive buffer capacity in bytes, from 1 to 1048576.

---@class TcpClientPortConfig : PortConfig
---@field remoteHost string Remote hostname or IP address.
---@field remotePort integer Remote port from 1 to 65535.
---@field logFormat? PortLogFormat (default: "utf-8") Format used to render both transmitted and received data in the port log.
---@field txSuffix? PortSuffix (default: "null")
---@field bufferSize? integer (default: 65536) Receive buffer capacity in bytes, from 1 to 1048576.

---@class SslClientPortConfig : PortConfig
---@field remoteHost string Remote hostname or IP address.
---@field remotePort integer Remote port from 1 to 65535.
---@field logFormat? PortLogFormat (default: "utf-8") Format used to render both transmitted and received data in the port log.
---@field txSuffix? PortSuffix (default: "null")
---@field bufferSize? integer (default: 65536) Receive buffer capacity in bytes, from 1 to 1048576.

---@alias PortCreateConfig
---| SerialPortConfig
---| TcpClientPortConfig
---| SslClientPortConfig

---
---Returns the names of all configured ports.
---
---The order is unspecified.
---
---@return portName[] names
function port.list() end

---
---Returns a current information snapshot for a configured port.
---
---Available fields depend on the port type and may include status, addresses,
---buffer usage, and connected server peers.
---
---@param name portName Target port name.
---@return table<string, any> information
function port.info(name) end

---
---Creates a configured port.
---
---Select the type with `port.Type`, provide the fields of its matching config
---class, then open the new port separately with `port.open`. The same validation
---as the graphical port editor is applied; invalid or duplicate configurations
---raise a Lua error.
---@param config PortCreateConfig Port configuration matching the selected `port.Type`.
---@return nil
function port.create(config) end

---
---Removes a configured port.
---
---The port name must exist. Invalid removal requests raise a Lua error.
---@param name portName Target port name.
---@return nil
function port.remove(name) end

---
---Opens or starts a configured port.
---
---@param name portName Target port name.
---@return nil
function port.open(name) end

---
---Closes or stops a configured port.
---
---@param name portName Target port name.
---@return nil
function port.close(name) end

---
---Discards data currently held in a port's receive buffer.
---
---For a server port, the receive buffers of all connected peers are cleared.
---
---@param name portName Target port name.
---@return nil
function port.clear(name) end

---
---Writes binary-safe data to a port.
---
---For TCP, SSL, and WebSocket server ports, omitting `peerIp` broadcasts to all
---connected peers. Other port types ignore `peerIp`.
---
---@param name portName Target port name.
---@param data string Data to write.
---@param peerIp? string Server peer identifier in `address:port` form.
---@return nil
function port.write(name, data, peerIp) end

---
---Reads data from a port's receive buffer.
---
---For stream ports, `length` greater than zero waits for exactly that many bytes;
---if the timeout expires first, an empty string is returned. A zero or omitted
---`length` immediately drains all currently buffered data and never waits.
---
---For TCP, SSL, and WebSocket server ports, omitting `peerIp` selects one
---unspecified connected peer. A VideoStream port instead returns the latest
---processed frame result and may return an array when it contains multiple values.
---
---@param name portName Target port name.
---@param length? integer (default: 0) Number of bytes to read.
---@param timeout? integer (default: 0) Wait timeout in milliseconds: `0` returns immediately, a positive value waits up to that duration, and `-1` waits indefinitely.
---@param peerIp? string Server peer identifier in `address:port` form.
---@return string|string[] data
function port.read(name, length, timeout, peerIp) end

---
---Reads through the first occurrence of a delimiter.
---
---The returned data includes the delimiter. If the delimiter is not found before
---the timeout expires, an empty string is returned.
---
---For TCP, SSL, and WebSocket server ports, omitting `peerIp` selects one
---unspecified connected peer. VideoStream ports do not support delimiter reads.
---
---@param name portName Target port name.
---@param text? string (default: "\r\n") Delimiter to read through.
---@param timeout? integer (default: 0) Wait timeout in milliseconds: `0` returns immediately, a positive value waits up to that duration, and `-1` waits indefinitely.
---@param peerIp? string Server peer identifier in `address:port` form.
---@return string data
function port.readUntil(name, text, timeout, peerIp) end
