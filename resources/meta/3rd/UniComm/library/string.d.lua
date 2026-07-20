---@meta

string = {}

---Convert a binary string to its base64 representation.
---@param string string The binary string to convert.
---@return string
function string.toBase64(string) end

---Convert a base64 string to its binary representation.
---@param string string The base64 string to convert.
---@return string
function string.fromBase64(string) end

---Convert a binary string to its hexadecimal representation.
---@param string string The binary string to convert.
---@param separator? string Optional separator between hex bytes.
---@return string
function string.toHex(string, separator) end

---Convert a hexadecimal string to its binary representation.
---@param string string The hexadecimal string to convert.
---@return string
function string.fromHex(string) end
