-- Configure at least one key in the Database panel before running this demo.
local keys = database.list()

-- list() returns configured keys in an unspecified order.
for _, key in ipairs(keys) do
    print("database key", key)
end

local key = keys[1]
if key == nil then
    error("Configure at least one Database key before running this demo.")
end

-- write() replaces the current value of an existing key.
database.write(key, "Hello from UniComm!")

-- clear() clears all current values but preserves the configured keys.
-- database.clear()
