# open()

## signature

port.open([index]) -> nil

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.

## returns

nil

## examples

-- open current port  
port.open()

-- open specific port index  
port.open(1)
