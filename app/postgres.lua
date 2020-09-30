local p = require("pretty-print").prettyPrint
-- local Postgres = require("postgres")
-- Postgres TODO

local connect = require('postgres').connect

coroutine.wrap(function()
    local psql = assert(connect {
        username = "postgres",
        password = "123456",
        database = "postgres"
    })
    p(psql)
    p(psql.query("SELECT 'Hello' AS greeting"))
    psql.close()
end)()
