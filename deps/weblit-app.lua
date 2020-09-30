-- Ignore SIGPIPE if it exists on platform
local uv = require('luv')
if uv.constants.SIGPIPE then uv.new_signal():start("sigpipe") end

local router = require('weblit-router').newRouter()
local server = require('weblit-server').newServer(router.run)

-- Forward router methods from app instance
local serverMeta = {}
function serverMeta:__index(name)
    if type(router[name] == "function") then
        return function(...)
            router[name](...)
            return self
        end
    end
end
setmetatable(server, serverMeta)

return server
