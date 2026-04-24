---@meta

---@alias mailbox
---| string
---| '"INBOX"'

imap = {}

---Send IDLE command to the server when the client is ready to accept unsolicited mailbox update messages.
---@param name portName Target port name.
---@param timeout? integer (default: 600000) Maximum time in **milliseconds** to wait for data to arrive.
---@return integer sequenceNumber
function imap.idle(name, timeout) end

---Send LOGIN command to authenticate with IMAP server.
---@param name portName Target port name.
---@param username string IMAP username/email address.
---@param password password IMAP password.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function imap.login(name, username, password, timeout) end

---Send SELECT command to select a mailbox so that messages in the mailbox can be accessed.
---@param name portName Target port name.
---@param mailbox mailbox IMAP mailbox.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function imap.select(name, mailbox, timeout) end

---Send FETCH command to retrieve data associated with a message in the mailbox.
---@param name portName Target port name.
---@param sequenceNumber integer Mail sequence number.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return table
function imap.fetch(name, sequenceNumber, timeout) end

---Wait for a new email (optionally from a specific sender) with IDLE and FETCH, then save it to a directory.
---@param name portName Target port name.
---@param from? string (default: "") Expected sender; when omitted accept any sender.
---@param path? string (default: "") Relative path under the workspace; when omitted uses workspace root.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function imap.receive(name, from, path, timeout) end