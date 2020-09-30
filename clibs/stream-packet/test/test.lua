package.cpath = "lib?.dylib;" .. package.cpath
local spack = require "stream_packet"

local index = spack.get()
local e, id, p = spack.recv(s, "002400000000000aeyJhIjoxLCJiIjoyfQ==")
print(e, id, p)
e, p = spack.send(index, 10, '{"a":"\0","b":"\0"}')
print(e, p)
e, id, p = spack.recv(index, p)
print(e, id, p)
spack.put(s)
