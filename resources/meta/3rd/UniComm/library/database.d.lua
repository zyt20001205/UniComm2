---@meta

---@alias databaseKey
---| string
---| '"__PLACEHOLDER__DATABASEKEY__"'

database = {}

---Retrieves a list of all available database keys.
---@return table
function database.list() end

---Clears all data from the database。
function database.clear() end

---Writes data to an available key in database.
---@param key databaseKey The key to write to.
---@param value boolean|number|string The value to write.
---@return nil
function database.write(key, value) end
