local uv = require("luv")

-- Returns the memory usage of the current process in bytes
-- in the form of a table with the structure:
--  { rss = value, heapUsed = value }
-- where rss is the resident set size for the current process,
-- and heapUsed is the memory used by the Lua VM
local function memoryUsage(_) -- unused args [self]
    return {
        rss = uv.resident_set_memory(),
        heapUsed = collectgarbage("count") * 1024
    }
end

local MICROS_PER_SEC = 1000000

-- Returns the user and system CPU time usage of the current process in microseconds
-- (as a table of the format {user=value, system=value})
-- The result of a previous call to process:cpuUsage() can optionally be passed as
-- an argument to get a diff reading
local function cpuUsage(_, prevValue) -- unused args [self]
    local rusage, err = uv.getrusage()
    if not rusage then return nil, err end
    local user = MICROS_PER_SEC * rusage.utime.sec + rusage.utime.usec
    local system = MICROS_PER_SEC * rusage.stime.sec + rusage.stime.usec
    if prevValue then
        user = user - prevValue.user
        system = system - prevValue.system
    end
    return {user = user, system = system}
end

return {memoryUsage = memoryUsage, cpuUsage = cpuUsage}
