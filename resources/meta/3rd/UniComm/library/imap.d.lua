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

---@alias mailbox
---| string
---| '"INBOX"'

---@class ImapMessage
---@field header table<string, string> Message header fields.
---@field body table<string, string>[] Decoded message body parts.
---@field attachment table<string, string>[] Decoded attachment parts.

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
---Send SELECT command to select a mailbox so that messages in the mailbox can be accessed.
---
---@param mailbox mailbox IMAP mailbox.
---@return nil
function imap:select(mailbox) end

---
---Send FETCH command to retrieve and decode a message from the selected mailbox.
---
---@param sequenceNumber integer Message sequence number.
---@return ImapMessage
function imap:fetch(sequenceNumber) end

---
---Send IDLE command and wait for the selected mailbox size to change.
---
---@param timeout? integer (default: 600000) Maximum time in **milliseconds** to wait for mail to arrive.
---@return integer sequenceNumber Current message count reported by EXISTS.
function imap:idle(timeout) end

---
---Wait for a new email with IDLE and FETCH, optionally filter by sender, then save its body and attachments.
---
---@param from? string (default: "") Expected sender; when omitted accept any sender.
---@param path? string (default: "") Relative directory under the workspace; when omitted uses workspace root.
---@param timeout? integer (default: 600000) Maximum time in **milliseconds** to wait for mail to arrive.
---@return nil
function imap:receive(from, path, timeout) end
