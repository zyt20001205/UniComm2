-- Relative paths start from the current workspace.
local source = "filesystem-demo.txt"
local renamed = "filesystem-demo-renamed.txt"

-- A to-be-closed FileHandle closes automatically when its block ends.
do
    local file <close> = filesystem.open(source, "w+")
    file:write("Hello from UniComm!\n", 42)
    file:flush()

    file:seek("set", 0)
    io.log(file:read("a"))
end

-- Rename and remove files without opening a FileHandle.
filesystem.rename(source, renamed)
filesystem.remove(renamed)
