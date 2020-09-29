local Emitter = require("core").Emitter
local uv = require("luv")

local signalWraps = {}
local signalListeners = {}

local function on(self, _type, listener)
    if _type == "error" or _type == "uncaughtException" or _type == "exit" then
        Emitter.on(self, _type, listener)
    else
        if not signalWraps[_type] then
            local signal = uv.new_signal()
            signalWraps[_type] = signal
            uv.unref(signal)
            uv.signal_start(signal, _type, function()
                self:emit(_type)
            end)
        end
        signalListeners[_type] = (signalListeners[_type] or 0) + 1
        Emitter.on(self, _type, listener)
    end
end

local function removeListener(self, _type, listener)
    if _type == "error" or _type == "uncaughtException" or _type == "exit" then
        return Emitter.removeListener(self, _type, listener)
    else
        local signal = signalWraps[_type]
        if not signal then return end
        local num_removed = Emitter.removeListener(self, _type, listener)
        if not num_removed then return end
        signalListeners[_type] = signalListeners[_type] - num_removed
        -- close the signal if there are no more listeners left
        if signalListeners[_type] == 0 then
            signal:stop()
            uv.close(signal)
            signalWraps[_type] = nil
            signalListeners[_type] = nil
        end
        return num_removed
    end
end

return {on = on, removeListener = removeListener}
