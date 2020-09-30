local p = require("pretty-print").prettyPrint
local Redis = require("redis")

local c = Redis:new({db = 0})

c:send("set", "foo", "1")

--send a command with a callback
c:send("get", "foo", function(err, data)
  if err then error(err) end
  p(data)
end)