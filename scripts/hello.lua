local coroutine = coroutine
local yield = coroutine.yield
local print = print

local co = coroutine.create(function()
    local i = 0
    while true do
        if i % 60 == 0 then
            print("Coro:", i, collectgarbage("count"))
        end
        i = i + 1
        yield()
    end
end)

print("Coroutine created")
return co
