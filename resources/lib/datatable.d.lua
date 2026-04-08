--- @meta

--- @alias datatableKey
--- | string
--- | '"__PLACEHOLDER__DATATABLEKEY__"'

datatable = {}

--- Retrieves a list of all available datatable keys.
--- @return table
function datatable.list() end

--- Writes data to a key in datatable.
--- @param key datatableKey The key to write to.
--- @param value boolean|number|string The value to write.
--- @return nil
function datatable.write(key, value) end

--- Clears all data in datatable.
--- @return nil
function datatable.clear() end

--- Exports the datatable to a CSV file.
--- @param fileName? string (default: "") The name of csv file; when omitted uses timestamp as file name.
--- @return nil
function datatable.export(fileName) end
