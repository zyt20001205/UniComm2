--[[
These demos use lib modbusRtu that provides low-level Modbus RTU communication over a port.
The helper functions string.pack, string.unpack, and string.toHex are used for binary data conversion.
]]

--Open port.
port.open("COM10")

--Read a single 16-bit register.
local raw = modbusRtu.readHoldingRegisters("COM10", 1, 0, 1)
local data = string.unpack(">i2", raw)

--Read two consecutive 16-bit registers and combine into a 32-bit value.
local raw = modbusRtu.readHoldingRegisters("COM10", 1, 0, 2)
local data = string.unpack(">i4", raw)

--Write a single 16-bit value.
local raw = string.pack(">i2", 200)
modbusRtu.writeSingleRegister("COM10", 1, 0, string.toHex(raw))

--Write multiple consecutive 16-bit registers.
local raw = string.pack(">i4", 1000000)
modbusRtu.writeMultipleRegisters("COM10", 1, 0, string.toHex(raw))