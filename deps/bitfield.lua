local ffi = _G.jit and require("ffi") or require("cffi")
local bit = _G.jit and _G.bit or require("bit")
local rshift = bit.rshift
local lshift = bit.lshift
local band = bit.band
local bnot = bit.bnot
local bor = bit.bor

local Bitfield = {}

function Bitfield:get(index)
    assert(type(index) == 'number')
    local byte = self.data[rshift(index, 3)]
    return band(byte, lshift(1, index % 8)) > 0
end

function Bitfield:set(index, value)
    assert(type(index) == 'number')
    assert(type(value) == 'boolean')
    local offset = rshift(index, 3)
    local byte = self.data[offset]
    if value then
        self.data[offset] = bor(byte, lshift(1, index % 8))
    else
        self.data[offset] = band(byte, bnot(lshift(1, index % 8)))
    end
    return value
end

do
    local bitfield = ffi.metatype('struct { size_t len; uint8_t data[?]; }',
                                  {__index = Bitfield})
    function Bitfield.new(size)
        local len = rshift(size - 1, 3) + 1
        local bits = bitfield(len)
        bits.len = len
        return bits
    end
end

return Bitfield
