---@meta

---@class ModbusRtu
ModbusRtu = {}

---
---Create a Modbus RTU instance.
---
---[Modbus RTU demo](../demo/modbusRtu.lua)
---
---@param name portName
---@param slaveAddr integer The slave address (1-247) of the target device on the network.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return modbusRtu
function ModbusRtu.new(name, slaveAddr, timeout) end

---@class modbusRtu
modbusRtu = {}

---
---Read data from multiple holding registers of a Modbus RTU device.
---
---@param startAddr integer The starting address of the first register to read from.
---@param quantity integer Number of registers to read.
---@return string
function modbusRtu:readHoldingRegisters(startAddr, quantity) end

---
---Write data to a single register of a Modbus RTU device.
---
---@param regAddr integer The address of the register to write to.
---@param data string **Hex string** containing the data to be written.
---@return nil
function modbusRtu:writeSingleRegister(regAddr, data) end

---
---Write data to multiple holding registers of a Modbus RTU device.
---
---@param startAddr integer The starting address of the first register to write to.
---@param data string **Hex string** containing the data to be written.
---@return nil
function modbusRtu:writeMultipleRegisters(startAddr, data) end
