local uv = _G.require("uv")
local deque = require("deque")

local _res = {}

local function init(name)
    _res[name] = {}
    _res[name].pipe = deque()
    local function run()
        local pipe = _res[name].pipe
        if pipe.count == 0 then
            return
        end
        local obj = pipe:popRight()

        coroutine.resume(obj, obj)
    end
    _res[name].async = uv.new_async(run)
end

local function enqueue(name, f, ...)
    if not _res[name] then
        init(name)
    end
    local arg = {...}

    local co
    co =
        coroutine.create(
        function()
            f(co, unpack(arg))
            uv.async_send(_res[name].async)
        end
    )
    _res[name].pipe:pushLeft(co)
    uv.async_send(_res[name].async)

    return true
end

local function coro(f, ...)
    local arg = {...}
    local co
    co =
        coroutine.create(
        function()
            f(co, unpack(arg))
        end
    )
end

local function async(co, f, ...)
    local arg = {...}
    table.insert(
        arg,
        function(...)
            coroutine.resume(co, ...)
        end
    )
    f(unpack(arg))
    return coroutine.yield(co)
end

return {
    enqueue = enqueue,
    async = async
}
