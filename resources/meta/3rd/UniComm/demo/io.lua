-- Standard Lua file paths start from the current workspace.
local source = "io-demo.txt"
local renamed = "io-demo-renamed.txt"

-- A to-be-closed standard file handle closes automatically when its block ends.
do
    local file <close> = assert(io.open(source, "w+"))
    assert(file:write("Hello from UniComm!\n", 42))
    assert(file:flush())

    assert(file:seek("set", 0))
    print(assert(file:read("*a")))
end

-- Standard os functions use the same workspace-relative path behavior.
assert(os.rename(source, renamed))
assert(os.remove(renamed))
