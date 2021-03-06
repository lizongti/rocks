local Error = require('core').Error
local utils = {}
local p = require('pretty-print')
for name, value in pairs(p) do utils[name] = value end

local function bind(fn, self, ...)
    assert(fn, "fn is nil")
    local bindArgsLength = select("#", ...)

    -- Simple binding, just inserts self (or one arg or any kind)
    if bindArgsLength == 0 then return function(...) return fn(self, ...) end end

    -- More complex binding inserts arbitrary number of args into call.
    local bindArgs = {...}
    return function(...)
        local argsLength = select("#", ...)
        local args = {...}
        local arguments = {}
        for i = 1, bindArgsLength do arguments[i] = bindArgs[i] end
        for i = 1, argsLength do arguments[i + bindArgsLength] = args[i] end
        return fn(self, unpack(arguments, 1, bindArgsLength + argsLength))
    end
end

local function noop(err) if err then print("Unhandled callback error", err) end end

local function adapt(c, fn, ...)
    local nargs = select('#', ...)
    local args = {...}
    -- No continuation defaults to noop callback
    if not c then c = noop end
    local t = type(c)
    if t == 'function' then
        args[nargs + 1] = c
        return fn(unpack(args))
    elseif t ~= 'thread' then
        error("Illegal continuation type " .. t)
    end
    local err, data, waiting
    args[nargs + 1] = function(e, ...)
        if waiting then
            if e then
                assert(coroutine.resume(c, nil, e))
            else
                assert(coroutine.resume(c, ...))
            end
        else
            err, data = e and Error:new(e), {...}
            c = nil
        end
    end
    fn(unpack(args))
    if c then
        waiting = true
        return coroutine.yield(c)
    elseif err then
        return nil, err
    else
        return unpack(data)
    end
end

utils.bind = bind
utils.noop = noop
utils.adapt = adapt

return utils
