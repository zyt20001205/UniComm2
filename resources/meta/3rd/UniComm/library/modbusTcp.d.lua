---@meta

modbusTcp = {}

---Reads data from multiple holding registers of a Modbus TCP device.
---@param name portName Target port name.
---@param transactionId integer The transaction id used for pairing.
---@param unitId integer The unit id (1-247) of the target device on the network.
---@param startAddr integer The starting address of the first register to read from.
---@param quantity integer Number of registers to read.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return string
function modbusTcp.readHoldingRegisters(name, transactionId, unitId, startAddr, quantity, timeout) end

---Writes data to a single register of a Modbus TCP device.
---@param name portName Target port name.
---@param transactionId integer The transaction id used for pairing.
---@param unitId integer The unit id (1-247) of the target device on the network.
---@param regAddr integer The address of the register to write to.
---@param data string **Hex string** containing the data to be written.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function modbusTcp.writeSingleRegister(name, transactionId, unitId, regAddr, data, timeout) end

---Writes data to multiple holding registers of a Modbus TCP device.
---@param name portName Target port name.
---@param transactionId integer The transaction id used for pairing.
---@param unitId integer The unit id (1-247) of the target device on the network.
---@param startAddr integer The starting address of the first register to write to.
---@param data string **Hex string** containing the data to be written.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function modbusTcp.writeMultipleRegisters(name, transactionId, unitId, startAddr, data, timeout) end
