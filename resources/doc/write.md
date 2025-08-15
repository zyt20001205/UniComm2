# write()

## signature

write(command[, index]) -> nil

## parameters

command(required, str/int): Integers are converted to string before sending.                        
index(optional, int): Target port index; when omitted or set to -1, the current port is used.

## returns

nil

## examples

-- write to current port  
write("010203")

-- write to specific port index  
write("AT+RST\r\n", 1)
