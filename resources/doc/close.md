# close()

## signature

port.close([index]) -> nil

## purpose

Closes a port connection.

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.

## returns

nil

## examples

-- close current port  
port.close()

-- close specific port index  
port.close(1)
