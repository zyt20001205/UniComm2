---@meta

---@class Http
Http = {}

---@class http
http = {}

---Create an HTTP instance over an existing port.
---@param name portName Target port name.
---@param timeout? integer (default: 30000) Maximum time in **milliseconds** to wait for data to arrive.
---@return http
function Http.new(name, timeout) end
