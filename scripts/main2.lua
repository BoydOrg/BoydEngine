-- Example script
-- (Copy this to `scripts/main.lua` in the build folder to get it to run on Scripting module startup)

print('Hello, Lua!')

local ent = boyd.Entity.create()
print(tostring(ent) .. ' created')

local transf = boyd.Transform():translated(10, 10, 10)
local transfComp = ent:comp('Transform')
transfComp:set(transf)
print('Transform is now ' .. tostring(transfComp:get()))

boyd.Entity.destroy(ent.id)
print('Is ' .. tostring(ent) .. ' valid after destruction? ' .. tostring(ent:isvalid()))

function update()
    print('Update from Lua!')
end

function halt()
    print('Goodbye from Lua!')
end
