---@meta

---@alias FileSeekWhence
---| "set"
---| "cur"
---| "end"

---@class FileHandle
local FileHandle = {}

---
---Close the file after flushing pending writes.
---
---@return boolean
function FileHandle:close() end

---
---Flush buffered writes to the underlying file.
---
---@return boolean
function FileHandle:flush() end

---
---Move the file cursor and return its new byte position.
---
---@param whence? FileSeekWhence (default: "cur") Cursor origin.
---@param offset? integer (default: 0) Byte offset from the origin.
---@return integer position
function FileHandle:seek(whence, offset) end

---
---Read values from the current file cursor.
---
---@param ... readmode
---@return any
---@return any ...
function FileHandle:read(...) end

---
---Write string or number values at the current file cursor.
---
---@param ... string|number
---@return FileHandle self
function FileHandle:write(...) end

---
---Open, remove, and rename files.
---
---[Filesystem demo](../demo/filesystem.lua)
---
---@class filesystem
filesystem = {}

---
---Open a file and return its handle.
---
---@param path string File path. Relative paths start from the current workspace.
---@param mode? openmode (default: "r") File open mode.
---@return FileHandle
function filesystem.open(path, mode) end

---
---Remove a file.
---
---@param path string File path. Relative paths start from the current workspace.
---@return nil
function filesystem.remove(path) end

---
---Rename a file or directory.
---
---@param from string Source path. Relative paths start from the current workspace.
---@param to string Target path. Relative paths start from the current workspace.
---@return nil
function filesystem.rename(from, to) end
