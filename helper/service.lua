local uv = require("uv")
local vm = require("vm")
local queue = require("queue")

local function start(service_name, ...)
    vm.register(service_name)
    vm.call(
        service_name,
        0,
        function(service_name, ...)
            local dispatch = require(service_name)
            local queue = require("queue")

            require("pack")
            local send = function(service_name, content)
                queue.push("task", string.pack(">p>p", service_name, content))
            end
            _G.__update = function(msg)
                local s = {
                    send = send
                }
                dispatch(s, msg)
            end
        end,
        service_name,
        ...
    )
end

local function thread_func(id)
    local uv = require("uv")
    local spsc_queue = require("spsc_queue")
    local queue = require("queue")
    local vm = require("vm")
    require("pack")

    local timer = uv.new_timer()
    timer:start(
        0,
        1,
        function()
            local success, content = queue.pop("task")
            while success do
                local i, service_name, msg = string.unpack(content, ">p>p")
                local vm_success = vm.call(service_name, 0, "__update", msg)
                if not vm_success then
                    queue.push("task", content)
                end
                success, content = queue.pop("task")
            end
        end
    )

    uv.run()
end

local function init()
    for id = 1, #assert(uv.cpu_info()) do
        uv.new_thread(thread_func, id)
    end
end
init()

local send = function(service_name, content)
    queue.push("task", string.pack(">p>p", service_name, content))
end
local service = {
    init = init,
    start = start,
    send = send
}

return service
