
return {
    speed = 5.0,

    _ready = function(self)
        print("Player script ready for node:", self.node:GetName())
    end,

    _process = function(self, delta)
        local pos = self.node:GetPosition()
        local moved = false

        if self.node:GetNodeType() == "RigidBody3d" then
            local vel = self.node:AsRigidBody():GetLinearVelocity()
            self.node:AsRigidBody():SetLinearVelocity(vel)
        end

        if Input.IsKeyHeld(Input.Key.W) then
            pos.z = pos.z - self.speed * delta
            moved = true
        end
        if Input.IsKeyHeld(Input.Key.S) then
            pos.z = pos.z + self.speed * delta
            moved = true
        end
        if Input.IsKeyHeld(Input.Key.A) then
            pos.x = pos.x - self.speed * delta
            moved = true
        end
        if Input.IsKeyHeld(Input.Key.D) then
            pos.x = pos.x + self.speed * delta
            moved = true
        end
        if Input.IsKeyHeld(Input.Key.SPACE) then
            pos.y = pos.y + self.speed * delta
            moved = true
        end
        if Input.IsKeyHeld(Input.Key.LCTRL) then
            pos.y = pos.y - self.speed * delta
            moved = true
        end

        if moved then
            self.node:SetPosition(pos)
        end

        if Input.IsKeyPressed(Input.Key.ESCAPE) then
            App.Quit()
        end
    end,
}