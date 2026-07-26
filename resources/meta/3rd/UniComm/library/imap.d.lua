---@meta

---@class Imap
Imap = {}

---
---Create an IMAP instance.
---
---[IMAP demo](../demo/imap.lua)
---
---@param name portName
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return imap
function Imap.new(name, timeout) end

---@class imap
imap = {}

---
---Send LOGIN command to authenticate with IMAP server.
---
---@param username string IMAP username/email address.
---@param password password IMAP application password.
---@return nil
function imap:login(username, password) end

---
---Wait for a new message in INBOX, optionally filter by sender, then save its body and attachments.
---
---@param from? string (default: "") Expected sender; when omitted accept any sender.
---@param path? string (default: "") Relative directory under the workspace; when omitted uses workspace root.
---@param timeout? integer (default: 600000) Maximum total time in **milliseconds** to wait for mail to arrive.
---@return table<string, string> header Message header fields.
function imap:receive(from, path, timeout) end

---
---Send LOGOUT command and close the IMAP session.
---
---@return nil
function imap:logout() end
