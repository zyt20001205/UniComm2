---@meta

---@class Imap
Imap = {}

---Create an IMAP instance.
---@param name portName
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return imap
function Imap.new(name, timeout) end

---@alias mailbox
---| string
---| '"INBOX"'

---@class imap
imap = {}

---Send IDLE command to the server when the client is ready to accept unsolicited mailbox update messages.
---@param timeout? integer (default: 600000) Maximum time in **milliseconds** to wait for mail to arrive.
---@return integer sequenceNumber
function imap:idle(timeout) end

---Send LOGIN command to authenticate with IMAP server.
---@param username string IMAP username/email address.
---@param password password IMAP password.
---@return nil
function imap:login(username, password) end

---Send SELECT command to select a mailbox so that messages in the mailbox can be accessed.
---@param mailbox mailbox IMAP mailbox.
---@return nil
function imap:select(mailbox) end

---Send FETCH command to retrieve data associated with a message in the mailbox.
---@param sequenceNumber integer Mail sequence number.
---@return table
function imap:fetch(sequenceNumber) end

---Wait for a new email (optionally from a specific sender) with IDLE and FETCH, then save it to a directory.
---@param from? string (default: "") Expected sender; when omitted accept any sender.
---@param path? string (default: "") Relative path under the workspace; when omitted uses workspace root.
---@param timeout? integer (default: 600000) Maximum time in **milliseconds** to wait for mail to arrive.
---@return nil
function imap:receive(from, path, timeout) end
