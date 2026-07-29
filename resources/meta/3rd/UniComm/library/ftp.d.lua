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

---@class (exact) FtpEntry
---@field name string Entry name.
---@field type "file"|"directory"|"link"|"unknown" Entry type.
---@field size? integer File size in bytes.
---@field modified? string UTC modification time in `YYYYMMDDHHMMSS[.sss]` format.

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
---List remote directory entries through a temporary passive data connection.
---
---@param path? string (default: "") Remote directory path; when omitted uses the current directory.
---@return FtpEntry[]
function ftp:list(path) end

---
---Send QUIT command and close the FTP session.
---
---@return nil
function ftp:quit() end
