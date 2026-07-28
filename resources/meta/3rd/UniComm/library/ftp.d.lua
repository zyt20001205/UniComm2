---@meta

---@class Ftp
Ftp = {}

---
---Create an FTP client instance and connect to the server.
---
---[FTP demo](../demo/ftp.lua)
---
---@param name portName Target TCP client port name.
---@param timeout? integer (default: 30000) Timeout in **milliseconds** for blocking FTP operations.
---@return ftp
function Ftp.new(name, timeout) end

---@class ftp
ftp = {}
