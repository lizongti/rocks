local uv = require('luv')
local Emitter = require('core').Emitter
local timer = require('timer')

local function start_listening(self)
    uv.udp_recv_start(self._handle, function(err, msg, rinfo, flags)
        timer.active(self)
        if err then
            self:emit('error', err)
        else
            if msg then self:emit('message', msg, rinfo, flags) end
        end
    end)
end

local function stop_listening(self) return uv.udp_recv_stop(self._handle) end

local Socket = Emitter:extend()
function Socket:initialize(type, callback)
    self._handle = uv.new_udp()
    if callback then self:on('message', callback) end
end

Socket.recvStart = start_listening

Socket.recvStop = stop_listening

function Socket:setTimeout(msecs, callback)
    if msecs > 0 then
        timer.enroll(self, msecs)
        timer.active(self)
        if callback then self:once('timeout', callback) end
    elseif msecs == 0 then
        timer.unenroll(self)
    end
end

function Socket:send(data, port, host, callback)
    timer.active(self)
    uv.udp_send(self._handle, data, host, port, callback)
end

function Socket:bind(port, host, options)
    uv.udp_bind(self._handle, host, port, options)
    self:recvStart()
end

function Socket:close(callback)
    timer.unenroll(self)
    if not self._handle then return end
    self:recvStop()
    uv.close(self._handle, callback)
    self._handle = nil
end

function Socket:address() return uv.udp_getsockname(self._handle) end

function Socket:setBroadcast(status) uv.udp_set_broadcast(self._handle, status) end

function Socket:setMembership(multicastAddress, multicastInterface, op)
    if not multicastAddress then error("multicast address must be specified") end

    if not multicastInterface then
        if self._family == 'udp4' then
            multicastInterface = '0.0.0.0'
        elseif self._family == 'udp6' then
            multicastInterface = '::0'
        end
    end
    return uv.udp_set_membership(self._handle, multicastAddress,
                                 multicastInterface, op)
end

function Socket:addMembership(multicastAddress, interfaceAddress)
    return self:setMembership(multicastAddress, interfaceAddress, 'join')
end

function Socket:dropMembership(multicastAddress, interfaceAddress)
    return self:setMembership(multicastAddress, interfaceAddress, 'leave')
end

function Socket:setTTL(ttl) uv.udp_set_ttl(self._handle, ttl) end

local function createSocket(type, callback)
    local ret = Socket:new(type, callback)
    ret._family = type
    return ret
end

return {Socket = Socket, createSocket = createSocket}
