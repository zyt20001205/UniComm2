-- Configure an echo-capable communication port named "Echo" before running.
local name = "Echo"

-- List configured ports and inspect the selected port.
io.log(port.list())
io.log(port.info(name))

port.open(name)
port.clear(name)

-- Write and read a line.
port.write(name, "Hello from UniComm\r\n")
local line = port.readUntil(name, "\r\n", 1000)
io.log(line)

-- Write and read binary data.
local request = string.fromHex("01 03 00 00 00 01")
port.write(name, request)
local response = port.read(name, #request, 1000)
io.log(string.toHex(response, " "))

port.close(name)
