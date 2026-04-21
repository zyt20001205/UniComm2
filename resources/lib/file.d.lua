---@meta

f = {}

---Opens a file, in the mode specified in the string mode.
---@param path string The relative path to the workspace.
---@param mode? openmode
function f.open(path, mode) end

---Close a file.
---@param path string The relative path to the workspace.
function f.close(path) end

---Reads the file, according to the given formats, which specify what to read.
---@param path string The relative path to the workspace.
---@param ... readmode
---@return any
---@return any ...
function f.read(path, ...) end

---Writes the value of each of its arguments to the file.
---@param path string The relative path to the workspace.
---@param ... string|number
function f.write(path, ...) end
