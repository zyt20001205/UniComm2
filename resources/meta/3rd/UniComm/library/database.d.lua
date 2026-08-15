---@meta

---@alias databaseKey
---| string
---| '"__PLACEHOLDER__DATABASEKEY__"'

---
---Access configured key-value cells in the current workspace.
---
---Keys must be created in the Database panel before scripts can write them.
---
---[Database demo](../demo/database.lua)
---
database = {}

---
---Returns all configured database keys.
---
---The order is unspecified.
---
---@return string[] keys
function database.list() end

---
---Clears the current value of every configured key without removing the keys.
---
---@return nil
function database.clear() end

---
---Replaces the current value of a configured database key.
---
---An unknown key raises an error.
---
---@param key databaseKey Target database key.
---@param value boolean|number|string Value to display.
---@return nil
function database.write(key, value) end
