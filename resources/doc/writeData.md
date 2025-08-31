# writeData()

## purpose

Writes **raw binary data** to the port's transmit buffer.

## signature

port.writeData([index], command[, peerIp]) -> nil

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.  
command(required, str/int): Integers are converted to string before sending.  
peerIp(optional, str): (TCP Server port only) Specifies the target client for the command; when omitted or set to -1, the command will be broadcast to all connected clients.

## returns

nil

## examples

-- write to current port  
port.writeData("010203")

-- write to specific port index  
port.writeData(1,"AT+RST\r\n")

-- write to specific client  
port.writeData("010203", "192.169.1.56800")  