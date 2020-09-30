local os = require("os")
local hooks = require("hooks")
local utils = require("utils")
local uv = require("luv")
local core = require("core")
local pp = require("pretty-print")
local UvStreamWritable = require("process/uv_stream").UvStreamWritable
local UvStreamReadable = require("process/uv_stream").UvStreamReadable
local memoryUsage = require("process/stats").memoryUsage
local cpuUsage = require("process/stats").cpuUsage
local on = require("process/signal").on
local removeListener = require("process/signal").removeListener

local process = core.process

local function kill(pid, signal) uv.kill(pid, signal or "sigterm") end

local function exit(self, code)
    local left = 2
    code = code or 0
    local function onFinish()
        left = left - 1
        if left > 0 then return end
        self:emit("exit", code)
        os.exit(code)
    end
    process.stdout:once("finish", onFinish)
    process.stdout:_end()
    process.stderr:once("finish", onFinish)
    process.stderr:_end()
end

local function bootstrap(main, ...)
    process.args = {...}
    local success, err = xpcall(function()
        -- Call the main app
        main(unpack(process.args))

        -- Start the event loop
        uv.run()
    end, function(err)
        -- During a stack overflow error, this can fail due to exhausting the remaining stack.
        -- We can't recover from that failure, but wrapping it in a pcall allows us to still
        -- return the stack overflow error even if the 'process.uncaughtException' fails to emit
        pcall(function() hooks:emit('process.uncaughtException', err) end)
        return debug.traceback(err)
    end)

    if success then
        -- Allow actions to run at process exit.
        hooks:emit('process.exit')
        uv.run()
    else
        process.exitCode = -1
        pp.stderr:write("Uncaught exception:\n" .. err .. "\n")
    end

    local function isFileHandle(handle, name, fd)
        return process[name].handle == handle and uv.guess_handle(fd) == 'file'
    end
    local function isStdioFileHandle(handle)
        return isFileHandle(handle, 'stdin', 0) or
                   isFileHandle(handle, 'stdout', 1) or
                   isFileHandle(handle, 'stderr', 2)
    end
    -- When the loop exits, close all unclosed uv handles (flushing any streams found).
    uv.walk(function(handle)
        if handle then
            local function close()
                if not handle:is_closing() then handle:close() end
            end
            -- The isStdioFileHandle check is a hacky way to avoid an abort when a stdio handle is a pipe to a file
            -- TODO: Fix this in a better way, see https://github.com/luvit/luvit/issues/1094
            if handle.shutdown and not isStdioFileHandle(handle) then
                handle:shutdown(close)
            else
                close()
            end
        end
    end)
    uv.run()
end

local function initProcess()
    process.bootstrap = bootstrap
    process.exitCode = 0
    process.kill = kill
    process.exit = exit
    process.on = on
    process.removeListener = removeListener
    process.memoryUsage = memoryUsage
    process.cpuUsage = cpuUsage
    if uv.guess_handle(0) ~= "file" then
        process.stdin = UvStreamReadable:new(pp.stdin)
    else
        -- special case for 'file' stdin handle to avoid aborting from
        -- reading from a pipe to a file descriptor
        -- see https://github.com/luvit/luvit/issues/1094
        process.stdin = require("fs").ReadStream:new(nil, {fd = 0})
    end
    process.stdout = UvStreamWritable:new(pp.stdout)
    process.stderr = UvStreamWritable:new(pp.stderr)
    hooks:on("process.exit", utils.bind(process.emit, process, "exit"))
    hooks:on("process.uncaughtException",
             utils.bind(process.emit, process, "uncaughtException"))
end

initProcess()

return process
