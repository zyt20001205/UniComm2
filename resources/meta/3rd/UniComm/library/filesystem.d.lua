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
---Return whether the file cursor has reached the end.
---
---@return boolean
function FileHandle:atEnd() end

---
---Return the current file cursor position in bytes.
---
---@return integer
function FileHandle:pos() end

---
---Move the file cursor and return its new byte position.
---
---@param whence? FileSeekWhence (default: "cur") Cursor origin.
---@param offset? integer (default: 0) Byte offset from the origin.
---@return integer position
function FileHandle:seek(whence, offset) end

---
---Return the file size in bytes.
---
---@return integer
function FileHandle:size() end

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
---Access files and directories under the current workspace.
---
---[Filesystem demo](../demo/filesystem.lua)
---
---@class filesystem
filesystem = {}

---
---Open a file and return its handle.
---
---@param path string Workspace-relative file path.
---@param mode? openmode (default: "r") File open mode.
---@return FileHandle
function filesystem.open(path, mode) end

---
---Check whether a file, directory, or symbolic link exists.
---
---@param path string Workspace-relative path.
---@return boolean
function filesystem.exists(path) end

---
---List entries in a directory.
---
---@param path? string (default: ".") Workspace-relative directory path.
---@return FileInfo[]
function filesystem.list(path) end

---
---Return information about one file, directory, or symbolic link.
---
---@param path string Workspace-relative path.
---@return FileInfo
function filesystem.stat(path) end

---
---Copy a file to a path that does not already exist.
---
---@param from string Workspace-relative source file path.
---@param to string Workspace-relative target file path.
---@return nil
function filesystem.copy(from, to) end

---
---Create a directory and any missing parent directories.
---
---@param path string Workspace-relative directory path.
---@return nil
function filesystem.mkdir(path) end

---
---Remove a file or symbolic link.
---
---@param path string Workspace-relative path.
---@return nil
function filesystem.remove(path) end

---
---Rename a file or directory to a path that does not already exist.
---
---@param from string Workspace-relative source path.
---@param to string Workspace-relative target path.
---@return nil
function filesystem.rename(from, to) end

---
---Remove an empty directory.
---
---@param path string Workspace-relative directory path.
---@return nil
function filesystem.rmdir(path) end

---
---Open a file or directory with the system default application.
---
---@param path string Workspace-relative path.
---@return nil
function filesystem.openExternal(path) end
