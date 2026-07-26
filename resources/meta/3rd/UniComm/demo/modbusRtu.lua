-- Configure a serial port named "COM10" before running.
port.open("COM10")
local modbusRtu = ModbusRtu.new("COM10", 1, 1000)

-- Read a single 16-bit register.
local raw = modbusRtu:readHoldingRegisters(0, 1)
local data = string.unpack(">i2", raw)

-- Read two consecutive 16-bit registers and combine them into a 32-bit value.
local raw = modbusRtu:readHoldingRegisters(0, 2)
local data = string.unpack(">i4", raw)

-- Write a single 16-bit value.
local raw = string.pack(">i2", 200)
modbusRtu:writeSingleRegister(0, string.toHex(raw))

-- Write multiple consecutive 16-bit registers.
local raw = string.pack(">i4", 1000000)
modbusRtu:writeMultipleRegisters(0, string.toHex(raw))
