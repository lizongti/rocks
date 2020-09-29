local Emitter = require("core").Emitter
local e = {}
setmetatable(e, Emitter.meta)
if e.init then e:init() end
return e
