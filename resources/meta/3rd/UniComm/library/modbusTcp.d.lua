---@meta

---@class ModbusTcp
ModbusTcp = {}

---
---Create a Modbus TCP instance.
---
---[Modbus TCP demo](../demo/modbusTcp.lua)
---
---@param name portName
---@param transactionId integer The transaction id used for pairing.
---@param unitId integer The unit id (1-247) of the target device on the network.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return modbusTcp
function ModbusTcp.new(name, transactionId, unitId, timeout) end

---@class modbusTcp
modbusTcp = {}

---
---Read data from multiple holding registers of a Modbus TCP device.
---
---@param startAddr integer The starting address of the first register to read from.
---@param quantity integer Number of registers to read.
---@return string
function modbusTcp:readHoldingRegisters(startAddr, quantity) end

---
---Write data to a single register of a Modbus TCP device.
---
---@param regAddr integer The address of the register to write to.
---@param data string **Hex string** containing the data to be written.
---@return nil
function modbusTcp:writeSingleRegister(regAddr, data) end

---
---Write data to multiple holding registers of a Modbus TCP device.
---
---@param startAddr integer The starting address of the first register to write to.
---@param data string **Hex string** containing the data to be written.
---@return nil
function modbusTcp:writeMultipleRegisters(startAddr, data) end
