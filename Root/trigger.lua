local Trigger = {}

function Trigger:_ready()
    self.node:Connect("body_entered", self.node, "_on_body_entered")
    self.node:Connect("body_exited", self.node, "_on_body_exited")

    print("Area trigger ready")
end

function Trigger:_on_body_entered(body)
    if body == nil then
        return
    end

    print(body:GetName() .. " entered area")
end

function Trigger:_on_body_exited(body)
    if body == nil then
        return
    end

    print(body:GetName() .. " exited area")
end

return Trigger