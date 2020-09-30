return function(_, res, go)
    go()
    res.headers["Access-Control-Allow-Origin"] = "*"
    res.headers["Access-Control-Allow-Headers"] =
        "Origin, X-Requested-With, Content-Type, Accept"
end
