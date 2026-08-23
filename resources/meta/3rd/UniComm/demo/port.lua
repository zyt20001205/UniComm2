-- Configure an echo-capable stream port named "Echo" before running.
local name = "Echo"
local timeout = 1000

-- Discover configured ports and inspect the selected port.
for _, portName in ipairs(port.list()) do
    print(portName)
end
print(port.info(name))

port.open(name)
port.clear(name)

-- readUntil includes the delimiter in the returned data.
local line = "Hello from UniComm\r\n"
port.write(name, line)
print(port.readUntil(name, "\r\n", timeout))

-- read with a positive length waits for exactly that many bytes.
local request = string.fromHex("01 03 00 00 00 01")
port.write(name, request)
local response = port.read(name, #request, timeout)
---@cast response string
print(string.toHex(response, " "))

-- port.read(name) immediately drains all bytes currently in the receive buffer.

-- TCP, SSL, and WebSocket servers identify a peer as "address:port".
-- Build the identifier from peerAddress and peerPort returned by port.info().
-- Omitting peerIp from port.write broadcasts; omitting it from port.read selects
-- one unspecified connected peer.
-- local information = port.info("Server")
-- local peer = information.peer1
-- local peerIp = peer.peerAddress .. ":" .. peer.peerPort
-- port.write("Server", "Hello", peerIp)
-- local data = port.read("Server", 0, 0, peerIp)

port.close(name)
