local uv = require("uv")

local _sig = {
    sigint = uv.new_signal(),
    sigquit = uv.new_signal(),
    sigterm = uv.new_signal(),
    sigusr1 = uv.new_signal(),
    sigusr2 = uv.new_signal()
}

local function set(f)
    uv.signal_start( -- kill -2
        _sig.sigint,
        "sigint",
        function(signal)
            print("[signal] got " .. signal)
            f()
        end
    )

    uv.signal_start( -- kill -3
        _sig.sigquit,
        "sigquit",
        function(signal)
            print("[signal] got " .. signal)
            f()
        end
    )

    uv.signal_start( -- kill -4
        _sig.sigterm,
        "sigterm",
        function(signal)
            print("[signal] got " .. signal)
            f()
        end
    )

    uv.signal_start( -- kill -usr1
        _sig.sigusr1,
        "sigusr1",
        function(signal)
            print("[signal] got " .. signal)
            f()
        end
    )

    uv.signal_start( -- kill -usr2
        _sig.sigusr2,
        "sigusr2",
        function(signal)
            print("[signal] got " .. signal)
            f()
        end
    )
end

return {
    set = set
}
