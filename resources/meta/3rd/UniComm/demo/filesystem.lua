-- All paths are relative to the current workspace.
local path = "filesystem-demo.bin"
local data = "Hello\0from UniComm!"

-- Binary mode preserves embedded NUL bytes and other non-text data.
filesystem.open(path, "wb")
filesystem.write(path, data)
filesystem.close(path)

-- Read mode "a" returns all remaining content as one binary Lua string.
filesystem.open(path, "rb")
local result = filesystem.read(path, "a")
filesystem.close(path)

io.log(result)
