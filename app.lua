local p = require("pretty-print").prettyPrint
local dump = require('pretty-print').dump
local Mongodb = require("mongodb")

local c = Mongodb:new({db = "test"})

c:on(
    "connect",
    function()
        local Post = c:collection("post")
        local page = 0
        
        local function distinct()
            Post:distinct(
                "Age",
                function(err, res)
                    p("distinct:", err, res)
                end
            )
        end
        
        local function update()
            local posts = Post:find({author = "Cyril Hou"})
            posts:limit(10):skip(page * 10):update({Age = 25}):exec(
                function(err, res)
                    p("update:", err, res)
                    distinct()
                end
            )
        end
        
        local function insert()
            Post:insert(
                {
                    title = "Hello word!",
                    content = "Here is the first blog post ....",
                    author = "Cyril Hou"
                },
                function(err, res)
                    p("insert:", err, res)
                    update()
                end
            )
        end

        insert()
    end
)
c:on(
    "end",
    function()
        -- client disconnected or server disconnected
        print("connect end")
    end
)
c:on(
    "error", -- connot connect or socket error
    function(err)
        print("error", err)
    end
)
c:on(
    "close", -- equals to end
    function()
        print("close")
    end
)
