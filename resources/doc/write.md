# write()

## signature

port.write([index], command[, peerIp]) -> nil

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.
command(required, str/int): Integers are converted to string before sending.
peerIp(optional, str): (TCP Server port only) Specifies the target client for the command; when omitted or set to -1, the command will be broadcast to all connected clients.

## returns

nil

## examples

-- write to current port  
port.rite("010203")

-- write to specific port index  
port.write(1,"AT+RST\r\n")

-- write to specific client  
port.write("010203", "192.169.1.56800")