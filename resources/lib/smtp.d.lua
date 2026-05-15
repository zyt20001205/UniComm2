---@meta

---@class Smtp
Smtp = {}

---Create an SMTP instance.
---@param name portName
---@param timeout? integer (default: 1000) Maximum time in **milliseconds** to wait for data to arrive.
---@return smtp
function Smtp.new(name, timeout) end

---@class smtp
smtp = {}

---Send AUTH LOGIN command to authenticate with SMTP server.
---@param username string SMTP username/email address.
---@param password password SMTP password.
---@return nil
function smtp:authLogin(username, password) end

---Send EHLO (Extended Hello) command to SMTP server to initiate session and discover server capabilities.
---@return nil
function smtp:ehlo() end

---Send an email with MAIL, RCPT and DATA commands.
---@param from string
---@param to string | table
---@param cc string | table | nil
---@param bcc string | table | nil
---@param subject string
---@param body string
---@param attachment? string (default: "") Path to the attachment; when omitted, no attachment is sent.
---@return nil
function smtp:send(from, to, cc, bcc, subject, body, attachment) end

---Send QUIT command to SMTP server to end communication.
---@return nil
function smtp:quit() end
