-- Configure an SSL client port named "IMAP" for the mail server before running.
local account = "receiver@example.com"
local imap = Imap.new("IMAP", 30000)

imap:login(account, "app-password")
imap:select("INBOX")

-- Change this value to test one workflow at a time: "fetch", "idle", or "receive".
local action = "fetch"

if action == "fetch" then
    local message = imap:fetch(1)
    io.log(message)
elseif action == "idle" then
    local sequenceNumber = imap:idle(600000)
    local message = imap:fetch(sequenceNumber)
    io.log(message)
elseif action == "receive" then
    imap:receive("", "mail", 600000)
end
