-- Configure an SSL client port named "SMTP" for the mail server before running.
local account = "sender@example.com"
local smtp = Smtp.new("SMTP", 30000)

smtp:ehlo()
smtp:authLogin(account, "app-password")

smtp:send {
    from = account,
    to = {
        "recipient@example.com"
    },
    -- cc = "copy@example.com",
    -- bcc = { "hidden@example.com" },
    subject = "UniComm SMTP test",
    body = [[
Hello from UniComm.
This message was sent through the SMTP API.
]],
    -- A single path is also accepted: attachment = "README.md"
    attachment = {
        "README.md"
    }
}

smtp:quit()
