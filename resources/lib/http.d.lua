---@meta

http = {}

---Performs an HTTP GET request to retrieve data from the specified URL.
---@param name portName Target port name.
---@param headers? table HTTP headers to include in the request.
---@param timeout? integer (default: 30000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function http.get(name, headers, timeout) end
