---@meta

---@class Http
Http = {}

---@class HttpResponse
---@field version string HTTP version from the status line.
---@field statusCode integer HTTP status code.
---@field reason string HTTP reason phrase.
---@field header table<string, string> Response headers with lowercase names.

---Create an HTTP instance over an existing port.
---@param name portName Target port name.
---@param timeout? integer (default: 30000) Maximum time in **milliseconds** to wait for data to arrive.
---@return http
function Http.new(name, timeout) end

---@class http
http = {}

---Send an HTTP HEAD request.
---@param target string HTTP request target.
---@param header? table<string, string|number> Additional request header.
---@return HttpResponse
function http:head(target, header) end

---Send an HTTP POST request.
---@param target string HTTP request target.
---@param body string HTTP request body.
---@param header? table<string, string|number> Additional request header. `Host` and `Content-Length` are generated automatically.
---@return HttpResponse
function http:post(target, body, header) end
