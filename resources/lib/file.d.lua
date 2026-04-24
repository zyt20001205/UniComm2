---@meta

f = {}

---Opens a file, in the mode specified in the string mode.
---@param path string Relative path under the workspace.
---@param mode? openmode
function f.open(path, mode) end

---Close a file.
---@param path string Relative path under the workspace.
function f.close(path) end

---Starts program prog in a separated process.
---@param path string Relative path under the workspace.
function f.popen(path) end

---Reads the file, according to the given formats, which specify what to read.
---@param path string Relative path under the workspace.
---@param ... readmode
---@return any
---@return any ...
function f.read(path, ...) end

---Writes the value of each of its arguments to the file.
---@param path string Relative path under the workspace.
---@param ... string|number
function f.write(path, ...) end
