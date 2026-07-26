local http = Http.new("HTTP", 30000)

local header = {
    ["Accept"] = "application/json",
    ["Content-Type"] = "application/json; charset=utf-8"
}

local body = [[
{
    "message": "Hello from UniComm"
}
]]

local result = http:post("/post", header, body)

io.log(result.statusCode)
io.log(result.header)
io.log(result.body)
