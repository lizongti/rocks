local vm = require("vm")
vm.register("a")
vm.register("b")

print(
    vm.call(
        "a",
        1,
        function(a, b)
            print(a, b)
            return 1
        end,
        "aaa",
        "bbbb"
    )
)
print(
    vm.mono_call(
        "a",
        1,
        function(a, b)
            print(a, b)
            return 1
        end,
        "aaa",
        "bbbb"
    )
)
