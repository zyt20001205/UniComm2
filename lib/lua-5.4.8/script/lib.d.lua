--- @meta

--- Suspends the current thread for a specified amount of time.
--- @param ms integer The number of milliseconds to sleep.
--- @return nil
---
--- @usage — Sleep for 1 second.
---
--- sleep(1000)
function sleep(ms) end

--- Shows an input dialog for variable assignment.
--- @return string
---
--- @usage — Display input dialog and assign to variable.
---
--- local command = input()
function input() end

--- The output has been redirected to the logging system.
function print(...) end

port = {}

--- Opens a port connection for communication.
--- @param index? integer Target port index; when omitted or set to -1, the current selected port is used.
--- @return boolean status The status of the open operation.
---
--- @usage — Open current selected port.
---
--- port.open()
---
--- @usage — Open specific port.
---
--- port.open(1)
function port.open(index) end

--- Closes a port connection.
--- @param index? integer Target port index; when omitted or set to -1, the current selected port is used.
--- @return nil
---
--- @usage — Close current selected port.
---
--- port.close()
---
--- @usage — Close specific port.
---
--- port.close(1)
function port.close(index) end

--- Prints information about a port.
--- @param index? integer Target port index; when omitted or set to -1, the current selected port is used.
--- @return nil
---
--- @usage — Print information about current selected port.
---
--- port.info()
---
--- @usage — Print information about specific port.
---
--- port.info(1)
function port.info(index) end

--- Reads **raw binary data** from a port.
--- @param index? integer Target port index; when omitted or set to -1, the current selected port is used.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
---
--- * 0(default): The function returns immediately.
---
--- * &gt;0: The function will block for up to the specified time, waiting for data.
---
--- * -1: The function will block indefinitely until data arrives.
--- @return bytes data
---
--- @usage — Read data from current selected port.
---
--- port.readData()
---
--- @usage — Read data from specific port index.
---
--- port.readData(1)
---
--- @usage — Read data from specific port index under async mode.
---
--- port.writeText(0, "0110 0000 000102 0000")
---
--- sleep(50)
---
--- port.readData(0, 0)
---
--- @usage — Read data from specific port index under sync mode.
---
--- port.writeText(0, "0110 0000 000102 0000")
---
--- port.readData(0, 100)
function port.readData(index, timeout) end

--- Reads **decoded text data** from a port.
--- @param index? integer Target port index; when omitted or set to -1, the current selected port is used.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
---
--- * 0(default): The function returns immediately.
---
--- * &gt;0: The function will block for up to the specified time, waiting for data.
---
--- * -1: The function will block indefinitely until data arrives.
--- @return string text
---
--- @usage — Read text from current selected port.
---
--- port.readText()
---
--- @usage — Read text from specific port index.
---
--- port.readText(1)
---
--- @usage — Read text from specific port index under async mode.
---
--- port.writeText(0, "0110 0000 000102 0000")
---
--- sleep(50)
---
--- port.readText(0, 0)
---
--- @usage — Read text from specific port index under sync mode.
---
--- port.writeText(0, "0110 0000 000102 0000")
---
--- port.readText(0, 100)
function port.readText(index, timeout) end

--- Writes **raw binary data** to a port.
--- @param index? integer Target port index; when omitted or set to -1, the current selected port is used.
--- @param data bytes The raw binary data to write.
--- @param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted or set to -1, the command will be broadcast to all connected clients.
--- @return nil
---
--- @usage — Write data to current selected port.
---
--- port.writeData("/x01/x03")
---
--- @usage — Write data to specific port index.
---
--- port.writeData("/x01/x03", 1)
---
--- @usage — Write data to specific client.
---
--- port.writeData("/x01/x03", "192.169.1.56800")
function port.writeData(index, data, peerIp) end

--- Writes **text data** to a port.
--- @param index? integer Target port index; when omitted or set to -1, the current selected port is used.
--- @param text string The text data to write.
--- @param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted or set to -1, the command will be broadcast to all connected clients.
--- @return nil
---
--- @usage — Write text to current selected port.
---
--- port.writeText("010203")
---
--- @usage — Write text to specific port index.
---
--- port.writeText(1, "AT+RST\r\n")
---
--- @usage — Write text to specific client.
---
--- port.writeText("010203", "192.169.1.56800")
function port.writeText(index, text, peerIp) end

modbusRtu = {}
function modbusRtu.readHoldingRegisters() end
function modbusRtu.writeMultipleRegisters() end

modbusAscii = {}
function modbusAscii.readHoldingRegisters() end

database = {}
--- Writes data to a key in database.
--- @param key string The key to write to.
--- @param value string|number The value to write.
--- @return nil
function database.write(key, value) end

--- Clears all data in database.
function database.clear() end

datatable = {}
--- Writes data to a key in datatable.
--- @param key string The key to write to.
--- @param value string|number The value to write.
--- @return nil
function datatable.write(key, value) end