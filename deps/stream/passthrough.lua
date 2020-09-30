--[[
// a passthrough stream.
// basically just the most minimal sort of Transform stream.
// Every written chunk gets output as-is.
--]] local Transform = require("stream/transform").Transform

local PassThrough = Transform:extend()

function PassThrough:initialize(options)
    --[[
 if (!(this instanceof PassThrough))
    return new PassThrough(options)
  --]]
    Transform.initialize(self, options)
end

function PassThrough:_transform(chunk, cb) cb(nil, chunk) end

return {PassThrough = PassThrough}
