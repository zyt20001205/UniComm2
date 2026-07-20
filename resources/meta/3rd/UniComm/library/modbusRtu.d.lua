---@meta

modbusRtu = {}

---Reads data from multiple holding registers of a Modbus RTU device.
---@param name portName Target port name.
---@param slaveAddr integer The slave address (1-247) of the target device on the network.
---@param startAddr integer The starting address of the first register to read from.
---@param quantity integer Number of registers to read.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return string
function modbusRtu.readHoldingRegisters(name, slaveAddr, startAddr, quantity, timeout) end

---Writes data to a single register of a Modbus RTU device.
---@param name portName Target port name.
---@param slaveAddr integer The slave address (1-247) of the target device on the network.
---@param regAddr integer The address of the register to write to.
---@param data string **Hex string** containing the data to be written.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function modbusRtu.writeSingleRegister(name, slaveAddr, regAddr, data, timeout) end

---Writes data to multiple holding registers of a Modbus RTU device.
---@param name portName Target port name.
---@param slaveAddr integer The slave address (1-247) of the target device on the network.
---@param startAddr integer The starting address of the first register to write to.
---@param data string **Hex string** containing the data to be written.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function modbusRtu.writeMultipleRegisters(name, slaveAddr, startAddr, data, timeout) end
