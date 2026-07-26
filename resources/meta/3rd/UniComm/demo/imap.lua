-- Configure an SSL client port named "IMAP" for the mail server before running.
local account = "receiver@example.com"
local imap = Imap.new("IMAP", 30000)

imap:login(account, "app-password")
local header = imap:receive("", "mail", 600000)
io.log(header)
imap:logout()
