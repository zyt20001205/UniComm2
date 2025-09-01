# readData()

## purpose

Reads **raw binary data** from the port's receive buffer.

## signature

port.readData([index[, timeout]]) -> str

## parameters

index(optional, int): Target port index; when omitted or set to -1, the current port is used.  
timeout(optional, int): Maximum time in **milliseconds** to wait for data to arrive.
* 0(default): The function returns immediately.
* &gt;0: The function will block for up to the specified time, waiting for data.
* -1: The function will block indefinitely until data arrives.

## returns

string: the receive buffer content of the current or specified port. The actual content depends on the RX format setting (e.g., hex/ascii/utf-8).

## examples

-- read from current port  
port.readData()

-- read from specific port index  
port.readData(1)  

-- read from specific port index under async mode  
port.writeText(0, "0110 0000 000102 0000")  
sleep(50)  
port.readText(0, 0)

-- read from specific port index under sync mode  
port.writeText(0, "0110 0000 000102 0000")
port.readText(0, 100)  
