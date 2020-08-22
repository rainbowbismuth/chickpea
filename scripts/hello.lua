local coroutine = coroutine
local yield = coroutine.yield
local print = print

local n = 1

local function do_a_sum()
    local s = 0
    for i = 0, n do
        s = s + i
    end
    return s
end

local co = coroutine.create(function()
    local i = 0
    while true do
        if i % 60 == 0 then
            n = n + 1
        --    print("Coro:", i)
        --    --collectgarbage("count"))
        end
        local x = do_a_sum()
        i = i + 1
        yield()
    end
end)

print("Coroutine created")
return co
