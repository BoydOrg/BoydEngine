-- Example script
-- (Copy this to `scripts/main.lua` in the build folder to get it to run on Scripting module startup)

print('Initing camera example script')

--- Camera-related values

local yaw = 0.0
local pitch = 0.0
local roll = 0.0

--- Create the entity, as usual

local ent = boyd.entity.create()
print(tostring(ent) .. ' created')

local position = boyd.utils.Vec3(10, 0, 10)
local transf = boyd.Transform():translated(position)
local transfComp = ent:comp('Transform')
transfComp:set(transf)

print('Testing camera!')
print('Transform is now ' .. tostring(transfComp:get()))

local camera = boyd.Camera():perspective(45.0, 0.1, 1000)
local cameraComp = ent:comp('Camera')
local activeCamera = boyd.ActiveCamera()
local activeCameraComp = ent:comp('ActiveCamera')


local ent2 = boyd.entity.create()
print('testing scripts')
local luabehaviour_comp = ent2:comp('LuaBehaviour')
print("Before it's done")
local component_load_request_comp = ent2:comp('ComponentLoadRequest')
print("Before it's done")
local component = boyd.ComponentLoadRequest()
component:add(luabehaviour_comp.typeid, "scripts/test.lua")
component_load_request_comp:set(component)

print("Done")


cameraComp:set(camera)
activeCameraComp:set(activeCamera)

local yaw = 0.0
local pitch = 0.0

-- A hack to get a clone of the transform
local newCamera = transf:translated(boyd.frame.zero)

function update()
    print("From update")
    xoffset = boyd.input.get_axis(0)
    yoffset = boyd.input.get_axis(1)

    yaw = yaw + (xoffset / 10)
    pitch = pitch + (yoffset / 10)

    newCamera = transf:translated(position)
    newCamera = newCamera:rotated(yaw, 0, 1, 0)
    newCamera = newCamera:rotated(pitch, 1, 0, 0)
    transfComp:set(newCamera)
end

function halt()
    ptrint('Goodbye from Lua!')
end
