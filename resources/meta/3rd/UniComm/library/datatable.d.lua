---@meta

---@alias datatableKey
---| string
---| '"__PLACEHOLDER__DATATABLEKEY__"'

---
---Access configured columns in the current workspace Data Table.
---
---Each key identifies one column and maintains its own next row for appending.
---
---[Data Table demo](../demo/datatable.lua)
---
datatable = {}

---
---Returns all configured Data Table column keys.
---
---The order is unspecified.
---
---@return string[] keys
function datatable.list() end

---
---Removes all rows and resets every column's append position without removing columns.
---
---@return nil
function datatable.clear() end

---
---Appends a value to the next row of a configured Data Table column.
---
---An unknown key raises an error. Write once to every column for each logical row
---to keep their independently tracked append positions aligned.
---
---@param key datatableKey Target column key.
---@param value boolean|number|string Value to append.
---@return nil
function datatable.write(key, value) end

---
---Exports the current Data Table rows to a CSV file.
---
---Relative paths start from the current workspace. When omitted, a timestamped
---file name is generated automatically.
---
---@param path? string (default: "") CSV output path.
---@return nil
function datatable.export(path) end
