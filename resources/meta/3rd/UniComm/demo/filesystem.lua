-- All paths are relative to the current workspace.
local directory = "filesystem-demo-" .. os.time()
local source = directory .. "/source.bin"
local copy = directory .. "/copy.bin"
local renamed = directory .. "/renamed.bin"

-- Create a disposable directory, including missing parent directories.
filesystem.mkdir(directory)

-- A to-be-closed FileHandle closes automatically when its block ends.
do
    local file <close> = filesystem.open(source, "wb")
    file:write("Hello\0from UniComm!", 42)
    file:flush()
    io.log("write position", file:pos(), "size", file:size())
end

-- Binary reads preserve embedded NUL bytes and use the current cursor.
do
    local file <close> = filesystem.open(source, "rb")
    io.log("at end before read", file:atEnd())
    io.log("first five bytes", file:read(5))

    file:seek("set", 0)
    local data = file:read("a")
    io.log("all data", data, "at end after read", file:atEnd())
end

-- stat() returns one FileInfo; list() returns all entries in a directory.
local info = filesystem.stat(source)
io.log(info.name, info.type, info.size, info.modified)

for _, entry in ipairs(filesystem.list(directory)) do
    io.log(entry.name, entry.type, entry.size, entry.modified)
end

-- Query and modify paths without opening a FileHandle.
io.log("source exists", filesystem.exists(source))
filesystem.copy(source, copy)
filesystem.rename(copy, renamed)

-- Uncomment to open the file with the system default application.
-- filesystem.openExternal(renamed)

-- Remove all files and directories created by this demo.
filesystem.remove(source)
filesystem.remove(renamed)
filesystem.rmdir(directory)
