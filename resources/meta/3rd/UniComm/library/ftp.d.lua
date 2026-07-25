---@meta

---@class Ftp
Ftp = {}

---@class FtpStartOptions
---@field username? string Named account accepted by the server.
---@field password? string Password for the named account.
---@field allowAnonymous? boolean (default: true) Accept `anonymous` or `ftp` with any password.
---@field maxAttempts? integer (default: 1) Maximum number of failed authentication attempts.

---Create an FTP server instance over an existing TCP server port.
---@param name portName Target TCP server port name.
---@param timeout? integer (default: 30000) Timeout in **milliseconds** for blocking FTP operations.
---@return ftp
function Ftp.new(name, timeout) end

---@class ftp
ftp = {}

---Start the FTP service and handle each connected peer with an independent state machine.
---@param options FtpStartOptions Authentication options.
function ftp:start(options) end
