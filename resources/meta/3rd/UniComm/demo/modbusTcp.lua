-- Configure a TCP client port named "Modbus TCP" before running.
port.open("Modbus TCP")
local modbusTcp = ModbusTcp.new("Modbus TCP", 1, 1, 1000)

-- Read a single 16-bit register.
local raw = modbusTcp:readHoldingRegisters(0, 1)
local data = string.unpack(">i2", raw)

-- Read two consecutive 16-bit registers and combine them into a 32-bit value.
local raw = modbusTcp:readHoldingRegisters(0, 2)
local data = string.unpack(">i4", raw)

-- Write a single 16-bit value.
local raw = string.pack(">i2", 200)
modbusTcp:writeSingleRegister(0, string.toHex(raw))

-- Write multiple consecutive 16-bit registers.
local raw = string.pack(">i4", 1000000)
modbusTcp:writeMultipleRegisters(0, string.toHex(raw))
