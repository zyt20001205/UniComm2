--- @meta

--- @alias mailbox
--- | string
--- | '"INBOX"'

imap = {}

--- Send IDLE command to the server when the client is ready to accept unsolicited mailbox update messages.
--- @param name portName Target port name.
--- @param timeout? integer (default: 600000) Maximum time in **milliseconds** to wait for data to arrive.
--- @return integer sequenceNumber
function imap.idle(name, timeout) end

--- Send LOGIN command to authenticate with IMAP server.
--- @param name portName Target port name.
--- @param username string IMAP username/email address.
--- @param password password IMAP password.
--- @param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function imap.login(name, username, password, timeout) end

--- Send SELECT command to select a mailbox so that messages in the mailbox can be accessed.
--- @param name portName Target port name.
--- @param mailbox mailbox IMAP mailbox.
--- @param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
--- @return nil
function imap.select(name, mailbox, timeout) end

--- Send FETCH command to retrieve data associated with a message in the mailbox.
--- @param name portName Target port name.
--- @param sequenceNumber integer Mail sequence number.
--- @param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
--- @return table
function imap.fetch(name, sequenceNumber, timeout) end
