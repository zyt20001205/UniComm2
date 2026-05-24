---@meta

port = {}

---Retrieves a list of all available communication ports.
---@return table
function port.list() end

---Returns information about a port.
---@param name portName Target port name.
---@return table information
---@usage — Print information about port COM3.
---port.info("COM3")
function port.info(name) end

---Opens a port connection for communication.
---@param name portName Target port name.
---@return nil
---@usage — Open port COM3.
---port.open("COM3")
function port.open(name) end

---Closes a port connection.
---@param name portName Target port name.
---@return nil
---@usage — Close port COM3.
---port.close("COM3")
function port.close(name) end

---Clears buffer of a port.
---@param name portName Target port name.
---@return nil
---@usage — Clears buffer on port COM3.
---port.clear("COM3")
function port.clear(name) end

---Writes data to a port.
---@param name portName Target port name.
---@param data string The data to write.
---@param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted, broadcast to all connected clients.
---@return nil
---@usage — Write data to port COM3.
---port.write("COM3", "/x01/x03")
---@usage — Write data to specific client.
---port.write("TCP SERVER", "/x01/x03", "192.169.1.56800")
function port.write(name, data, peerIp) end

---Reads data from a port.
---@param name portName Target port name.
---@param length? integer (default: 0) Number of bytes to read; when omitted, read all from buffer.
---@param timeout? integer (default: 0) Maximum time in **milliseconds** to wait for data to arrive.
---* 0: The function returns immediately.
---* &gt;0: The function will block for up to the specified time, waiting for data.
---* -1: The function will block indefinitely until data arrives.
---@param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted, read from any available client.
---@return string data
---@usage — Read data from port COM3.
---port.read("COM3")
---@usage — Read data from port COM3 immediately.
---port.write("COM3", "0110 0000 000102 0000")
---thread.sleep(50)
---port.read("COM3")
---@usage — Read 8 bytes data from port COM3 within 100ms.
---port.write("COM3", "0110 0000 000102 0000")
---port.read("COM3", 8, 100)
function port.read(name, length, timeout, peerIp) end

---Reads data from a port until the specified string is found.
---@param name portName Target port name.
---@param text? string (default: "\r\n") String pattern to search for; when omitted, reads until newline.
---@param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
---* 0(default): The function returns immediately.
---* &gt;0: The function will block for up to the specified time, waiting for data.
---* -1: The function will block indefinitely until data arrives.
---@param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted, read from any available client.
---@return string data
---@usage — Read line from port COM3.
---port.readUntil("COM3")
---@usage — Read data until "OK" from port COM3.
---port.readUntil("COM3", "OK")
function port.readUntil(name, text, timeout, peerIp) end
