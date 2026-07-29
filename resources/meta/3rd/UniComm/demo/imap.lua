-- Configure an SSL client port named "IMAP" for the mail server before running.
local account = "receiver@example.com"
local imap = Imap.new("IMAP", 30000)

imap:login(account, "app-password")

-- An empty sender accepts the next message from anyone.
-- receive() waits up to ten minutes and returns all parsed content in memory.
local mail = imap:receive("", 600000)
io.log(mail.header)

-- body is an array because multipart messages can contain plain-text and HTML alternatives.
for index, body in ipairs(mail.body) do
    local extension = "bin"
    if body.contentType:find("text/plain", 1, true) then
        extension = "txt"
    elseif body.contentType:find("text/html", 1, true) then
        extension = "html"
    end

    local path = "imap-body-" .. index .. "." .. extension
    local output <close> = filesystem.open(path, "wb")
    output:write(body.data)
end

-- Attachment names are sender-controlled, so this demo only logs the original name
-- and writes each binary payload to a generated workspace-relative path.
for index, attachment in ipairs(mail.attachments) do
    io.log(attachment.name, attachment.contentType)

    local path = "imap-attachment-" .. index .. ".bin"
    local output <close> = filesystem.open(path, "wb")
    output:write(attachment.data)
end

imap:logout()
