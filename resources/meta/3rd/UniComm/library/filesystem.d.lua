---@meta

---
---Access files under the current workspace.
---
---[Filesystem demo](../demo/filesystem.lua)
---
---@class filesystem
filesystem = {}

---
---Open a file with the requested mode.
---
---@param path string Relative path under the workspace.
---@param mode? openmode (default: "r") File open mode.
---@return nil
function filesystem.open(path, mode) end

---
---Close an opened file.
---
---@param path string Relative path under the workspace.
---@return nil
function filesystem.close(path) end

---
---Open a file with the system default application.
---
---@param path string Relative path under the workspace.
---@return nil
function filesystem.popen(path) end

---
---Read values from an opened file.
---
---@param path string Relative path under the workspace.
---@param ... readmode
---@return any
---@return any ...
function filesystem.read(path, ...) end

---
---Write values to an opened file.
---
---@param path string Relative path under the workspace.
---@param ... string|number
---@return nil
function filesystem.write(path, ...) end
