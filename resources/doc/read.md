# read()

## signature

read([index]) -> str

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.

## returns

string: the receive buffer content of the current or specified port. The actual content depends on the RX format setting (e.g., hex/ascii/utf-8).

## examples

-- read from current port  
read()

-- read from specific port index  
read(1)
