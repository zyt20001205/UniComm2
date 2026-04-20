---@meta

io = {}

---Logging.
---@param ... any
---@return nil
function io.log(...) end

---Displays a message box with the specified text.
---@param text string The message text to display in the message box.
---@return nil
function io.message(text) end

---Converts text to speech and outputs it through the audio system.
---@param text string The text content to be spoken.
---@return nil
function io.speak(text) end
