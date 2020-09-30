local los = require('los')
local path_base = require('path/base')

local function setup_meta(ospath)
    local path = {}
    path._internal = ospath
    setmetatable(path, {
        __index = function(_, key)
            if type(path._internal[key]) == 'function' then
                return function(...)
                    return path._internal[key](path._internal, ...)
                end
            else
                return path._internal:_get(key)
            end
        end
    })
    return path
end

if los.type() == "win32" then
    return setup_meta(path_base.nt)
else
    return setup_meta(path_base.posix)
end
