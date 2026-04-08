--- @meta

thread = {}

--- Spawns a thread using the given file path.
--- @param filepath string Path to the Lua script.
--- @return string threadId Unique identifier for the spawned thread.
function thread.start(filepath) end

--- Stops the specified thread by sending a **termination request**.
--- @param threadId string The identifier of the thread to stop.
--- @return nil
function thread.stop(threadId) end

--- Suspends the current thread for a specified amount of time.
--- @param ms integer The number of milliseconds to sleep.
--- @return nil
--- @usage — Sleep for 1 second.
--- sleep(1000)
function thread.sleep(ms) end
