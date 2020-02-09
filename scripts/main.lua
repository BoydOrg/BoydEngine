-- Example script
-- (Copy this to `scripts/main.lua` in the build folder to get it to run on Scripting module startup)

print('Initing camera example script')

--- Camera-related values

local yaw = 0.0
local pitch = 0.0
local roll = 0.0

--- Create the entity, as usual

local ent = boyd.Entity.create()
print(tostring(ent) .. ' created')

local transf = boyd.Transform():translated(10, 0, 10)
local transfComp = ent:comp('Transform')
transfComp:set(transf)

print('Testing camera!')
print('Transform is now ' .. tostring(transfComp:get()))

local camera = boyd.Camera():perspective(45.0, 0.1, 1000)
local cameraComp = ent:comp('Camera')
local activeCamera = boyd.ActiveCamera()
local activeCameraComp = ent:comp('ActiveCamera')

cameraComp:set(camera)
activeCameraComp:set(activeCamera)

local yaw = 0.0
local pitch = 0.0

local position = {0, 0, 0}

-- A hack to get a clone of the transform
local newCamera = transf:translated(0, 0, 0)

function update()
    -- print('Update from Lua!')
    xoffset = boyd.Input.getAxis(0)
    yoffset = boyd.Input.getAxis(1)

    yaw = yaw + (xoffset / 10)
    pitch = pitch + (yoffset / 10)

    print(boyd.Input.getAxis(3))

    newCamera = transf:translated(position[1], position[2], position[3])
    newCamera = newCamera:rotated(yaw, 0, 1, 0)
    newCamera = newCamera:rotated(pitch, 1, 0, 0)
    transfComp:set(newCamera)
    --print(transf)
end

function halt()
    -- print('Goodbye from Lua!')
end