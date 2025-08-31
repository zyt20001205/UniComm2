# info()

## signature

port.info([index]) -> nil

## purpose

Retrieves information about a port.

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.

## returns

nil

## examples

-- get info from current port  
port.info()

-- get info from specific port index  
port.info(1)
