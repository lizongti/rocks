local redisCodec = require("redis/codec")
local sha1 = require("sha1")
local net = require("net")
local Emitter = require("core").Emitter
local p = require("pretty-print").prettyPrint

local Redis = Emitter:extend()
function Redis:send(...)
    local arg = {...}
    local cmd = arg[1]:lower()
    local callback = type(arg[#arg]) == "function" and table.remove(arg, #arg)
    if cmd == "self.multi" then
        self.socket:cork()
    elseif cmd == "exec" then
        self.socket:uncork()
        self.multi = false
        do
            local mCb = self.multiCallbacks
            local originalCallback = callback
            callback = function(err, data)
                if not err and type(data) == "table" and #data == #mCb then
                    for i, d in ipairs(data) do
                        if type(d) == "table" and d.error then
                            mCb[i].cb(d.error, nil)
                        elseif d == false then
                            mCb[i].cb(nil, nil)
                        else
                            mCb[i].cb(nil, d)
                        end
                    end
                end

                if originalCallback then
                    originalCallback(err, data)
                end
            end
        end
        self.multiCallbacks = {}

    elseif cmd == "discard" then
        self.socket:uncork()
        self.multi = false
        self.multiCallbacks = {}
    elseif cmd == "hmset" and type(arg[3]) == "table" then
        local rearg = {}
        table.insert(rearg, arg[1])
        table.insert(rearg, arg[2])
        for k, v in pairs(arg[3]) do
            table.insert(rearg, k)
            table.insert(rearg, v)
        end
        arg = rearg
    elseif callback and cmd == "hgetall" then
        local originalCallback = callback
        callback = function(err, data)
            if not err and data then
                local tdata = {}
                if type(data) == "table" then
                    local k = nil
                    for i, v in ipairs(data) do
                        if not k then
                            k = v
                        else
                            tdata[k] = v
                            k = nil
                        end
                    end
                    data = tdata
                end
            end
            originalCallback(err, data)
        end
    end

    if self.multi then
        table.insert(self.multiCallbacks, {cmd = cmd, cb = callback})
        table.insert(self.callbacks, {cmd = cmd, cb = false})
    else
        table.insert(self.callbacks, {cmd = cmd, cb = callback})
    end

    if cmd == "self.multi" then self.multi = true end

    self.socket:write(redisCodec.encode(arg))

    return self
end

function Redis:subscribe(channel, callback)
    self:send("subscribe", channel, function(err, d)
        if d then
            self.pubsub[channel] = callback
        else
            callback(err, nil)
        end
    end)
    return self
end

function Redis:unsubscribe(channel)
    self:send("unsubscribe", channel,
              function(err, d) if d then self.pubsub[channel] = nil end end)
end

function Redis:disconnect() self.socket:shutdown() end

function Redis:loadScript(name, script, callback)
    local src
    self.scripts[name] = sha1(script)
    self:send("script", "load", script, function(err, data)
        if callback then
            callback(err, data)
        else
            self.failHard(err, data)
        end
        assert(self.scripts[name] == data)
    end)
end

function Redis:runScript(name, keys, args, callback)
    if self.scripts[name] == false then
        error("script hasn't loaded yet")
    elseif self.scripts[name] then
        if callback then
            self:send("evalsha", self.scripts[name], #keys, unpack(keys),
                      unpack(args), callback)
        else
            self:send("evalsha", self.scripts[name], #keys, unpack(keys),
                      unpack(args))
        end
    else
        error("Unknown Redis script " .. tostring(name))
    end
end

function Redis:initialize(options)
    options = options or {}
    self.options = options
    p("Redis create connection.......")

    self.host = self.options.host or "127.0.0.1"
    self.port = self.options.port or 6379
    self.password = self.options.password or nil
    self.db = self.options.db or 0

    self.pubsub = {}
    self.callbacks = {}
    self.scripts = {}

    self.socket = nil

    self.multi = false
    self.multiCallbacks = {}

    self.failHard = function(err, ok) if (err) then error(err) end end

    self:connect()
end

function Redis:connect()
    p("Redis database is connected.......")
    self.socket = net.connect(self.port, self.host)
    self.socket:cork()

    if self.password then
        self:send("auth", self.password, function(err, d)
            self.failHard(err, d)
            if not self.db then self:emit("connect", err, d) end
        end)
    end

    if self.db then
        self:send("select", self.db, function(err, d)
            self.failHard(err, d)
            self:emit("connect", err, d)
        end)
    end

    self.socket:on("connect", function(err, d)
        -- p("connected")
        self.socket:uncork()
        if err then
            if err == "ECONNREFUSED" then
                error("Cound not connect to Redis at " .. self.host .. ":" ..
                          self.port)
            else
                error(err)
            end
        end
        if not (self.password or self.db) then
            self:emit("connect", err, d)
        end
    end)

    self.socket:on("disconnect",
                   function(err, d) self:emit("disconnect", err, d) end)

    self.socket:on('data', function(data)
        -- If error, print and close connection
        while data and #data > 0 do
            local d
            d, data = self:decode(data)
            if type(d) == "table" and d[1] == "message" then
                self.pubsub[d[2]](d[3])
            else
                if self.callbacks[1].cb then
                    if type(d) == "table" and d.error then
                        self.callbacks[1].cb(d.error, nil)
                    else
                        self.callbacks[1].cb(nil, d)
                    end
                end
                table.remove(self.callbacks, 1)
            end
        end
    end)
end

function Redis:decode(chunk)
    local value, index = redisCodec.decode(chunk, 1)
    if not index then return end
    return value, string.sub(chunk, index)
end

return Redis
