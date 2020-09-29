local uv = require("luv")

local _mapping = {}

local serializer = {} -- TODO
-- stream pack can be used

local function tcp_server(conf, user_accept, user_recv)
    local host, port, keep_alive, backlog = conf.host, conf.port,
                                            conf.keep_alive, conf.backlog
    local server

    local function close(client)
        if client:is_closing() then return end
        serializer.puts(_mapping[client])
        _mapping[client] = nil
        client:close()
    end

    local function send(client, id, p)
        if not client then return end
        local e, chunk = serializer.send(_mapping[client], id, p)
        if e <= 0 then
            print(
                "[network] when sending packet goes into exception, close the socket " ..
                    tostring(client))
            close(client)
        end
        client:write(chunk)
    end

    local function accept(err)
        if err then print("[network] when accepting goes into exception") end

        local client = uv.new_tcp()
        uv.tcp_keepalive(client, true, keep_alive)
        server:accept(client)

        _mapping[client] = serializer.gets()
        user_accept(client)

        client:read_start(function(err, chunk)
            if err then
                print(
                    "[network] when stream goes into exception, close the socket " ..
                        tostring(client))
                close(client)
            end

            if chunk then
                local e, id, p = serializer.recv(_mapping[client], chunk)
                while e > 0 do
                    user_recv(client, id, p)
                    e, id, p = serializer.recv(_mapping[client])
                end
                if e < 0 then
                    print(
                        "[network] when receiving packet goes into exception, close the socket " ..
                            tostring(client))
                    close(client)
                end
            else
                print("[network] when the stream ends, close the socket " ..
                          tostring(client))
                close(client)
            end
        end)
        print(
            "[network] when accepting a new client , start read from the socket " ..
                tostring(client))
    end

    server = uv.new_tcp()
    server:bind(host, port)
    server:listen(backlog, accept)

    assert(server:getsockname())
    print("[network] tcp listening on port " .. server:getsockname().port)

    local user_send = function(client, id, p) send(client, id, p) end

    local user_close = function(client)
        print(
            "[network] when received the user's close command, close the socket " ..
                tostring(client))
        close(client)
    end
    return user_send, user_close
end

local function accept(client)
    -- TODO
end

local function recv(client, id, packet)
    -- TODO
end

local net_send, net_close = tcp_server({
    host = "0.0.0.0",
    port = 10000,
    keep_alive = 60,
    backlog = 128
}, accept, recv)

net_send, net_close = network.tcp_server(_conf.tcp, accept, recv)
