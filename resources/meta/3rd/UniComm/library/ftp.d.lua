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

---
---Send USER and PASS commands to authenticate with FTP server.
---
---@param username string FTP username.
---@param password password FTP password.
---@return nil
function ftp:login(username, password) end

---
---Send QUIT command and close the FTP session.
---
---@return nil
function ftp:quit() end
