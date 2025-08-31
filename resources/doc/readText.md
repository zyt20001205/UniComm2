# readText()

## purpose

Reads **decoded text data** from the port's receive buffer.

## signature

port.readText([index]) -> str

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.

## returns

string: the receive buffer content of the current or specified port. The actual content depends on the RX format setting (e.g., hex/ascii/utf-8).

## examples

-- read from current port  
port.readText()

-- read from specific port index  
port.readText(1)
