local codec = require("codec")
local connect = require("coro-net").connect

return function(config)
    if not config then config = {} end

    local read, write = assert(connect {
        host = config.host or "localhost",
        port = config.port or 6379,
        encode = codec.encode,
        decode = codec.decode
    })

    return function(command, ...)
        if not command then return write() end
        write {command, ...}
        local res = read()
        if type(res) == "table" and res.error then error(res.error) end
        return res
    end
end
