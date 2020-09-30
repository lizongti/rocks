return function(req, res, go)
    if req.socket.tls then return go() end
    res.code = 301
    res.headers["Location"] = "https://" .. req.headers.Host .. req.path
    res.body = "Redirecting to HTTPS...\n"
end
