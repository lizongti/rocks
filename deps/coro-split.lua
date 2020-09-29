return function(...)
    local tasks = {...}
    for i = 1, #tasks do
        assert(type(tasks[i]) == "function", "all tasks must be functions")
    end
    local thread = coroutine.running()
    local left = #tasks
    local results = {}
    local function check()
        left = left - 1
        if left == 0 then
            assert(coroutine.resume(thread, unpack(results)))
        end
    end
    for i = 1, #tasks do
        coroutine.wrap(function()
            results[i] = tasks[i]()
            check()
        end)()
    end
    return coroutine.yield()
end
