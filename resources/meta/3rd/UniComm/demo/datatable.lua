-- Configure at least one column in the Data Table panel before running.
-- This demo clears the current rows to create a new aligned dataset.
local columns = datatable.list()

-- list() returns configured column keys in an unspecified order.
for _, key in ipairs(columns) do
    print("datatable column", key)
end

if #columns == 0 then
    error("Configure at least one Data Table column before running this demo.")
end

datatable.clear()

-- Each column tracks its own next row. Writing once to every column in each
-- iteration keeps the values aligned as logical rows.
for row = 1, 5 do
    for column, key in ipairs(columns) do
        datatable.write(key, row * column)
    end
end

-- Relative paths start from the current workspace. Omitting the path creates
-- a timestamped file name instead.
datatable.export("datatable-demo.csv")
