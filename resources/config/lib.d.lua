--- @meta

--- @alias key
--- | '"A"'
--- | '"B"'
--- | '"C"'
--- | '"D"'
--- | '"E"'
--- | '"F"'
--- | '"G"'
--- | '"H"'
--- | '"I"'
--- | '"J"'
--- | '"K"'
--- | '"L"'
--- | '"M"'
--- | '"N"'
--- | '"O"'
--- | '"P"'
--- | '"Q"'
--- | '"R"'
--- | '"S"'
--- | '"T"'
--- | '"U"'
--- | '"V"'
--- | '"W"'
--- | '"X"'
--- | '"Y"'
--- | '"Z"'

--- Executes a Lua script file in a new dedicated thread.
--- @param filename string Path to the Lua script.
--- @return string threadId Unique identifier for the spawned thread.
---
function exec(filename) end

--- Stops the specified thread by sending a termination **request**.
--- @param threadId string The identifier of the thread to stop.
--- @return boolean success If the thread exists and stop request was sent.
---
function stop(threadId) end

--- Blocks the current thread until the specified thread terminates.
--- @param threadId string The identifier of the thread to wait for.
--- @return boolean success If the thread exists and terminated successfully.
---
function wait(threadId) end

--- Shows an input dialog for variable assignment.
--- @return string
---
--- @usage — Display input dialog and assign to variable.
---
--- local command = input()
function input() end

--- The output has been redirected to the logging system.
function print(...) end

--- Suspends the current thread for a specified amount of time.
--- @param ms integer The number of milliseconds to sleep.
--- @return nil
---
--- @usage — Sleep for 1 second.
---
--- sleep(1000)
function sleep(ms) end

--- Converts text to speech and outputs it through the audio system.
--- @param text string The text content to be spoken (use "help" to list available voices/languages).
function speak(text) end

control = {}
--- Simulates a left mouse button click at specified coordinates.
--- @param x integer The horizontal screen position.
--- @param y integer The vertical screen position.
--- @return nil
function control.leftClick(x, y) end

--- Simulates a left mouse button double click at specified coordinates.
--- @param x integer The horizontal screen position.
--- @param y integer The vertical screen position.
--- @return nil
function control.leftDoubleClick(x, y) end

--- Simulates a right mouse button click at specified coordinates.
--- @param x integer The horizontal screen position.
--- @param y integer The vertical screen position.
--- @return nil
function control.rightClick(x, y) end

--- Simulates a right mouse button double click at specified coordinates.
--- @param x integer The horizontal screen position.
--- @param y integer The vertical screen position.
--- @return nil
function control.rightDoubleClick(x, y) end

--- Simulates pressing a keyboard key.
--- @param key key Key name.
--- @return nil
function control.keyPress(key) end

port = {}
--- Retrieves a list of all available communication ports.
--- @return table
function port.list() end

--- Opens a port connection for communication.
--- @param name port Target port name.
--- @return boolean status The status of the open operation.
---
--- @usage — Open port COM3.
---
--- port.open("COM3")
function port.open(name) end

--- Closes a port connection.
--- @param name port Target port name.
--- @return nil
---
--- @usage — Close port COM3.
---
--- port.close("COM3")
function port.close(name) end

--- Prints information about a port.
--- @param name port Target port name.
--- @return table information
---
--- @usage — Print information about port COM3.
---
--- port.info("COM3")
function port.info(name) end

--- Writes data to a port.
--- @param name port Target port name.
--- @param data string The data to write.
--- @param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted, broadcast to all connected clients.
--- @return nil
---
--- @usage — Write data to port COM3.
---
--- port.writeData("COM3", "/x01/x03")
---
--- @usage — Write data to specific client.
---
--- port.writeData("TCP SERVER", "/x01/x03", "192.169.1.56800")
function port.write(name, data, peerIp) end

--- Reads data from a port.
--- @param name port Target port name.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @param length? integer Number of bytes to read.
--- @param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted, read from any available client.
---
--- * 0(default): The function returns immediately.
---
--- * &gt;0: The function will block for up to the specified time, waiting for data.
---
--- * -1: The function will block indefinitely until data arrives.
--- @return string data
---
--- @usage — Read data from port COM3.
---
--- port.readData("COM3")
---
--- @usage — Read data from port COM3 under async mode.
---
--- port.writeText("COM3", "0110 0000 000102 0000")
---
--- sleep(50)
---
--- port.readData("COM3", 0)
---
--- @usage — Read data from port COM3 index under sync mode.
---
--- port.writeText("COM3", "0110 0000 000102 0000")
---
--- port.readData("COM3", 100)
function port.read(name, timeout, length, peerIp) end

modbusRtu = {}
--- Reads data from multiple holding registers of a Modbus RTU device.
--- @param name port Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param quantity integer Number of registers to read.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return bytes
function modbusRtu.readHoldingRegisters(name, slaveAddr, startAddr, quantity, timeout) end

--- Writes data to multiple holding registers to a Modbus RTU device.
--- @param name port Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param data string **Binary string** containing the raw data to be written.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function modbusRtu.writeMultipleRegisters(name, slaveAddr, startAddr, data, timeout) end

modbusAscii = {}
function modbusAscii.readHoldingRegisters() end

database = {}
--- Retrieves a list of all available database keys.
--- @return table
function database.list() end

--- Writes data to a key in database.
--- @param key database The key to write to.
--- @param value string|number The value to write.
--- @return nil
function database.write(key, value) end

--- Clears all data in database.
function database.clear() end

datatable = {}
--- Retrieves a list of all available datatable keys.
--- @return table
function datatable.list() end

--- Writes data to a key in datatable.
--- @param key datatable The key to write to.
--- @param value string|number The value to write.
--- @return nil
function datatable.write(key, value) end

--- Clears a column of datatable using a key identifier.
--- @param key? datatable The target column key identifier. ; when omitted or set to "all" clears all columns.
--- @return nil
function datatable.clear(key) end

--- Exports the datatable to a CSV file.
--- @return nil
function datatable.export() end

dataplot = {}
--- Appends a column from the datatable to dataplot using a key identifier
--- @param key datatable The named key of the column to append from the datatable.
--- @param position? integer 0=left Y-axis (default), 1=right Y-axis.
--- @return nil
function dataplot.append(key, position) end
