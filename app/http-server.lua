local http = require("http")

local function http_server(conf, handler)
    local server = http.createServer(handler):listen(conf.port, conf.host)
    print(
        "[network] http listening at http://" .. conf.host .. ":" .. conf.port ..
            "/")
    return server
end

local function on_request(req, res)
    -- TODO
end
http_server({host = "0.0.0.0", port = 10080}, on_request)
