--- @meta

--- @alias portName
--- | string
--- | '"__PLACEHOLDER__PORTNAME__"'

--- @alias databaseKey
--- | string
--- | '"__PLACEHOLDER__DATABASEKEY__"'

--- @alias datatableKey
--- | string
--- | '"__PLACEHOLDER__DATATABLEKEY__"'

--- @alias password "__PLACEHOLDER__PASSWORD__"

--- @alias key
--- | '"0"'
--- | '"1"'
--- | '"2"'
--- | '"3"'
--- | '"4"'
--- | '"5"'
--- | '"6"'
--- | '"7"'
--- | '"8"'
--- | '"9"'
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
--- | '"BACKSPACE"'
--- | '"TAB"'
--- | '"ENTER"'
--- | '"SHIFT"'
--- | '"CTRL"'
--- | '"ALT"'
--- | '"PAUSE"'
--- | '"CAPSLOCK"'
--- | '"ESC"'
--- | '"SPACE"'
--- | '"PAGEUP"'
--- | '"PAGEDOWN"'
--- | '"END"'
--- | '"HOME"'
--- | '"LEFT"'
--- | '"UP"'
--- | '"RIGHT"'
--- | '"DOWN"'
--- | '"PRINTSCREEN"'
--- | '"INSERT"'
--- | '"DELETE"'

database = {}
--- Retrieves a list of all available database keys.
--- @return table
function database.list() end

--- Writes data to a key in database.
--- @param key databaseKey The key to write to.
--- @param value boolean|number|string The value to write.
--- @return nil
function database.write(key, value) end

--- Clears all data in database.
function database.clear() end

datatable = {}
--- Retrieves a list of all available datatable keys.
--- @return table
function datatable.list() end

--- Writes data to a key in datatable.
--- @param key datatableKey The key to write to.
--- @param value boolean|number|string The value to write.
--- @return nil
function datatable.write(key, value) end

--- Clears a column of datatable using a key identifier.
--- @param key? datatableKey The target column key identifier; when omitted or set to "all" clears all columns.
--- @return nil
function datatable.clear(key) end

--- Exports the datatable to a CSV file.
--- @param fileName? string The name of csv file; when omitted uses timestamp as file name.
--- @return nil
function datatable.export(fileName) end

http = {}
--- Performs an HTTP GET request to retrieve data from the specified URL.
--- @param name portName Target port name.
--- @param headers? table HTTP headers to include in the request.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
function http.get(name, headers, timeout) end

io = {}
--- Logging.
---@param ... any
function io.log(...) end

--- Displays a message box with the specified text.
--- @param text string The message text to display in the message box.
function io.message(text) end

--- Converts text to speech and outputs it through the audio system.
--- @param text string The text content to be spoken.
function io.speak(text) end

key = {}
--- Simulates tapping a keyboard key.
--- @param key key Key name.
--- @return nil
function key.tap(key) end

--- Types a unicode string.
--- @param text string string
--- @return nil
function key.type(text) end

modbusAscii = {}
--- Reads data from multiple holding registers of a Modbus ASCII device.
--- @param name portName Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param quantity integer Number of registers to read.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return string
function modbusAscii.readHoldingRegisters(name, slaveAddr, startAddr, quantity, timeout) end

--- Writes data to write single register to a Modbus ASCII device.
--- @param name portName Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param regAddr integer The address of the register to write to.
--- @param data string **Hex string** containing the raw data to be written.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function modbusAscii.writeSingleRegister(name, slaveAddr, regAddr, data, timeout) end

--- Writes data to multiple holding registers to a Modbus ASCII device.
--- @param name portName Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param data string **Hex string** string** containing the raw data to be written.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function modbusAscii.writeMultipleRegisters(name, slaveAddr, startAddr, data, timeout) end

modbusRtu = {}
--- Reads data from multiple holding registers of a Modbus RTU device.
--- @param name portName Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param quantity integer Number of registers to read.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return string
function modbusRtu.readHoldingRegisters(name, slaveAddr, startAddr, quantity, timeout) end

--- Writes data to write single register to a Modbus RTU device.
--- @param name portName Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param regAddr integer The address of the register to write to.
--- @param data string **Hex string** containing the data to be written.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function modbusRtu.writeSingleRegister(name, slaveAddr, regAddr, data, timeout) end

--- Writes data to multiple holding registers to a Modbus RTU device.
--- @param name portName Target port name.
--- @param slaveAddr integer The slave address (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param data string **Hex string** containing the data to be written.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function modbusRtu.writeMultipleRegisters(name, slaveAddr, startAddr, data, timeout) end

modbusTcp = {}
--- Reads data from multiple holding registers of a Modbus TCP device.
--- @param name portName Target port name.
--- @param transactionId integer The transaction id used for pairing.
--- @param unitId integer The unit id (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param quantity integer Number of registers to read.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return string
function modbusTcp.readHoldingRegisters(name, transactionId, unitId, startAddr, quantity, timeout) end

--- Writes data to write single register to a Modbus TCP device.
--- @param name portName Target port name.
--- @param transactionId integer The transaction id used for pairing.
--- @param unitId integer The unit id (1-247) of the target device on the network.
--- @param regAddr integer The address of the register to write to.
--- @param data string **Hex string** containing the data to be written.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function modbusTcp.writeSingleRegister(name, transactionId, unitId, regAddr, data, timeout) end

--- Writes data to multiple holding registers to a Modbus TCP device.
--- @param name portName Target port name.
--- @param transactionId integer The transaction id used for pairing.
--- @param unitId integer The unit id (1-247) of the target device on the network.
--- @param startAddr integer The starting address of the first register to write to.
--- @param data string **Hex string** containing the data to be written.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function modbusTcp.writeMultipleRegisters(name, transactionId, unitId, startAddr, data, timeout) end

mouse = {}

--- Simulates a left mouse button click at specified coordinates.
--- @param x integer | "Get Position" The horizontal screen position.
--- @param y integer The vertical screen position.
--- @return nil
function mouse.click(x, y) end

--- Simulates a left mouse button double click at specified coordinates.
--- @param x integer | "Get Position" The horizontal screen position.
--- @param y integer The vertical screen position.
--- @return nil
function mouse.doubleClick(x, y) end

--- Simulates a right mouse button click at specified coordinates.
--- @param x integer | "Get Position" The horizontal screen position.
--- @param y integer The vertical screen position.
--- @return nil
function mouse.rightClick(x, y) end

port = {}
--- Retrieves a list of all available communication ports.
--- @return table
function port.list() end

--- Returns information about a port.
--- @param name portName Target port name.
--- @return table information
---
--- @usage — Print information about port COM3.
---
--- port.info("COM3")
function port.info(name) end

--- Opens a port connection for communication.
--- @param name portName Target port name.
--- @return nil
---
--- @usage — Open port COM3.
---
--- port.open("COM3")
function port.open(name) end

--- Closes a port connection.
--- @param name portName Target port name.
--- @return nil
---
--- @usage — Close port COM3.
---
--- port.close("COM3")
function port.close(name) end

--- Clears buffer of a port.
--- @param name portName Target port name.
--- @return nil
---
--- @usage — Clears buffer on port COM3.
---
--- port.clear("COM3")
function port.clear(name) end

--- Writes data to a port.
--- @param name portName Target port name.
--- @param data string The data to write.
--- @param peerIp? string (TCP Server only) Specifies the target client for the command; when omitted, broadcast to all connected clients.
--- @return nil
---
--- @usage — Write data to port COM3.
---
--- port.write("COM3", "/x01/x03")
---
--- @usage — Write data to specific client.
---
--- port.write("TCP SERVER", "/x01/x03", "192.169.1.56800")
function port.write(name, data, peerIp) end

--- Reads data from a port.
--- @param name portName Target port name.
--- @param length? integer Number of bytes to read.
--- @param timeout? integer Maximum time in **milliseconds** to wait for data to arrive.
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
--- port.read("COM3")
---
--- @usage — Read data from port COM3 under immediately.
---
--- port.write("COM3", "0110 0000 000102 0000")
---
--- sleep(50)
---
--- port.read("COM3")
---
--- @usage — Read 8 bytes data from port COM3 within 100ms.
---
--- port.write("COM3", "0110 0000 000102 0000")
---
--- port.read("COM3", 8, 100)
function port.read(name, length, timeout, peerIp) end

smtp = {}
--- Send EHLO (Extended Hello) command to SMTP server to initiate session and discover server capabilities.
--- @param name portName Target port name.
---
function smtp.ehlo(name) end

--- Send AUTH LOGIN command to authenticate with SMTP server.
--- @param name portName Target port name.
--- @param username string SMTP username/email address.
--- @param password password SMTP password.
---
function smtp.authLogin(name, username, password) end

--- Send a simple email.
--- @param name portName Target port name.
--- @param from string
--- @param to string
--- @param subject string
--- @param body string
--- @param attachment? string Path to the attachment.
---
function smtp.mail(name, from, to, subject, body, attachment) end

string = {}
--- Convert a binary string to its base64 representation.
--- @param string string The binary string to convert.
--- @return string
function string.toBase64(string) end

--- Convert a base64 string to its binary representation.
--- @param string string The base64 string to convert.
--- @return string
function string.fromBase64(string) end

--- Convert a binary string to its hexadecimal representation.
--- @param string string The binary string to convert.
--- @param separator? string Optional separator between hex bytes.
--- @return string
function string.toHex(string, separator) end

--- Convert a hexadecimal string to its binary representation.
--- @param string string The hexadecimal string to convert.
--- @return string
function string.fromHex(string) end

thread = {}
--- Spawns a thread using the given file path.
--- @param filepath string Path to the Lua script.
--- @return string threadId Unique identifier for the spawned thread.
---
function thread.start(filepath) end

--- Stops the specified thread by sending a **termination request**.
--- @param threadId string The identifier of the thread to stop.
--- @return nil
---
function thread.stop(threadId) end

--- Suspends the current thread for a specified amount of time.
--- @param ms integer The number of milliseconds to sleep.
--- @return nil
---
--- @usage — Sleep for 1 second.
---
--- sleep(1000)
function thread.sleep(ms) end

--- Shows an input dialog for variable assignment.
--- @return string
---
--- @usage — Display input dialog and assign to variable.
---
--- local command = input()
function input() end
