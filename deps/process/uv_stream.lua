local uv = require("luv")
local utils = require("utils")
local Readable = require("stream").Readable
local Writable = require("stream").Writable

local UvStreamWritable = Writable:extend()
function UvStreamWritable:initialize(handle)
    Writable.initialize(self)
    self.handle = handle
end

function UvStreamWritable:_write(data, callback)
    uv.write(self.handle, data, callback)
end

local UvStreamReadable = Readable:extend()
function UvStreamReadable:initialize(handle)
    Readable.initialize(self, {highWaterMark = 0})
    self._readableState.reading = false
    self.reading = false
    self.handle = handle
    self:on("pause", utils.bind(self._onPause, self))
end

function UvStreamReadable:_onPause()
    self._readableState.reading = false
    self.reading = false
    uv.read_stop(self.handle)
end

function UvStreamReadable:_read(_) -- unused args [n]
    local function onRead(err, data)
        if err then return self:emit("error", err) end
        self:push(data)
    end
    if not uv.is_active(self.handle) then
        self.reading = true
        uv.read_start(self.handle, onRead)
    end
end

return {
    UvStreamWritable = UvStreamWritable,
    UvStreamReadable = UvStreamReadable
}
