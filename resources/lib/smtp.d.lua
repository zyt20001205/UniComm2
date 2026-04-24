---@meta

smtp = {}

---Send EHLO (Extended Hello) command to SMTP server to initiate session and discover server capabilities.
---@param name portName Target port name.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function smtp.ehlo(name, timeout) end

---Send AUTH LOGIN command to authenticate with SMTP server.
---@param name portName Target port name.
---@param username string SMTP username/email address.
---@param password password SMTP password.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function smtp.authLogin(name, username, password, timeout) end

---Send an email with MAIL, RCPT and DATA commands.
---@param name portName Target port name.
---@param from string
---@param to string | table
---@param cc string | table | nil
---@param bcc string | table | nil
---@param subject string
---@param body string
---@param attachment? string (default: "") Path to the attachment; when omitted, no attachment is sent.
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return nil
function smtp.send(name, from, to, cc, bcc, subject, body, attachment, timeout) end

---Send QUIT command to SMTP server to end communication.
---@param name portName Target port name.
---@return nil
function smtp.quit(name, timeout) end