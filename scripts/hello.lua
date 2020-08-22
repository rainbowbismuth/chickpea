local coroutine = coroutine
local yield = coroutine.yield
local print = print

local co = coroutine.create(function()
    for i = 0, 10 do
        if i == 10 then
            print("Final one, i == 10")
        end
        print("Hello, from coroutine! " .. i)
        yield()
    end
end)

print("Coroutine created")
return co
