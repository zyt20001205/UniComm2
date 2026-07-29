-- Configure a TCP client port named "FTP" for the FTP server before running.
local ftp = Ftp.new("FTP", 30000)

ftp:login("anonymous", "anonymous@example.com")

local entries = ftp:list()
for _, entry in ipairs(entries) do
    io.log(entry.name, entry.type, entry.size, entry.modified)
end

ftp:quit()
