local tls = require('tls')
local http = require('http')

local function createServer(options, onRequest)
    return tls.createServer(options, function(socket)
        return http.handleConnection(socket, onRequest)
    end)
end

local function createConnection(...)
    local args = {...}
    local options = {}
    local callback
    if type(args[1]) == 'table' then
        options = args[1]
    elseif type(args[2]) == 'table' then
        options = args[2]
        options.port = args[1]
    elseif type(args[3]) == 'table' then
        options = args[3]
        options.port = args[2]
        options.host = args[1]
    else
        if type(args[1]) == 'number' then options.port = args[1] end
        if type(args[2]) == 'string' then options.host = args[2] end
    end

    if type(args[#args]) == 'function' then callback = args[#args] end

    return tls.connect(options, callback)
end

local function request(options, callback)
    options = http.parseUrl(options)
    if options.protocol and options.protocol ~= 'https' then
        error(string.format('Protocol %s not supported', options.protocol))
    end
    options.port = options.port or 443
    options.connect_emitter = 'secureConnection'
    options.socket = options.socket or createConnection(options)
    return http.request(options, callback)
end

local function get(options, onResponse)
    options = http.parseUrl(options)
    options.method = 'GET'
    local req = request(options, onResponse)
    req:done()
    return req
end

return {createServer = createServer, request = request, get = get}
