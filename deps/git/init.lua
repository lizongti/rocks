local core = require('git/core')
local exports = {}
for key, value in pairs(core) do exports[key] = value end

function exports.mount(fs) return require('git/db')(require('git/storage')(fs)) end

return exports
