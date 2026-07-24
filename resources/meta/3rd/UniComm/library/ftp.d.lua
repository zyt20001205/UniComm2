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
---@param timeout? integer (default: 30000) Maximum time in **milliseconds** to wait for protocol data.
---@return ftp
function Ftp.new(name, timeout) end

---@class ftp
ftp = {}

---Wait for one client, send the `220` response, and run the FTP command state machine.
---@param options FtpStartOptions Authentication options.
function ftp:start(options) end
