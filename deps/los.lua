local ffi = _G.jit and require("ffi") or require("cffi")

local map = {
    ['Windows'] = 'win32',
    ['Linux'] = 'linux',
    ['OSX'] = 'darwin',
    ['BSD'] = 'bsd',
    ['POSIX'] = 'posix',
    ['Other'] = 'other'
}

local function type() return map[ffi.os] end

return {type = type}
