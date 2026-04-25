local function normalize_xz(v)
    local len = math.sqrt(v.x * v.x + v.z * v.z)

    if len <= 0.00001 then
        v.x = 0.0
        v.z = 0.0
        return v
    end

    v.x = v.x / len
    v.z = v.z / len

    return v
end

return {
    base_speed = 5.0,
    speed = 5.0,
    jump_velocity = 7.0,

    jump_buffer_time = 0.12,
    coyote_time = 0.10,

    jump_buffer_timer = 0.0,
    coyote_timer = 0.0,

    body = nil,
    camera_node = nil,
    ground_ray_node = nil,

    _ready = function(self)
        self.body = self.node:AsRigidBody()

        if self.body == nil then
            print("ERROR: player.lua must be on RigidBody3d")
            return
        end

        self.node:Set("body_type", "Dynamic")
        self.node:Set("gravity_scale", 1.0)
        self.node:Set("lock_rotation", true)
        self.node:Set("sync_to_physics", true)

        self.camera_node = self.node:FindChildByType("CameraNode3d", true)
        self.ground_ray_node = self.node:FindChildByType("RayCastNode3d", true)

        if self.camera_node == nil then
            print("Player could not find CameraNode3d child")
        end

        if self.ground_ray_node == nil then
            print("Player could not find RayCastNode3d child")
        else
            local ray = self.ground_ray_node:AsRayCast()
            if ray ~= nil then
                ray:SetTargetPosition(vec3.new(0.0, -0.9, 0.0))
                ray:SetExcludeParentBody(true)
            end
        end
    end,

    _process = function(self, delta)
        if Input.IsKeyPressed(Input.Key.SPACE) then
            self.jump_buffer_timer = self.jump_buffer_time
        end

        if Input.IsKeyHeld(Input.Key.LSHIFT) then
           self.speed = self.base_speed * 4.5
        else
            self.speed = self.base_speed
        end

        if Input.IsKeyPressed(Input.Key.ESCAPE) then
            App.Quit()
        end
    end,

    _physics_process = function(self, delta)
        if self.body == nil then
            return
        end

        if self.jump_buffer_timer > 0.0 then
            self.jump_buffer_timer = self.jump_buffer_timer - delta
        end

        if self.coyote_timer > 0.0 then
            self.coyote_timer = self.coyote_timer - delta
        end

        local move_x = 0.0
        local move_z = 0.0

        if self.camera_node ~= nil then
            local camera = self.camera_node:AsCamera()

            if camera ~= nil then
                local forward = normalize_xz(camera:GetFront())
                local right = normalize_xz(camera:GetRight())

                if Input.IsKeyHeld(Input.Key.W) then
                    move_x = move_x + forward.x
                    move_z = move_z + forward.z
                end

                if Input.IsKeyHeld(Input.Key.S) then
                    move_x = move_x - forward.x
                    move_z = move_z - forward.z
                end

                if Input.IsKeyHeld(Input.Key.D) then
                    move_x = move_x + right.x
                    move_z = move_z + right.z
                end

                if Input.IsKeyHeld(Input.Key.A) then
                    move_x = move_x - right.x
                    move_z = move_z - right.z
                end
            end
        else
            if Input.IsKeyHeld(Input.Key.W) then move_z = move_z - 1.0 end
            if Input.IsKeyHeld(Input.Key.S) then move_z = move_z + 1.0 end
            if Input.IsKeyHeld(Input.Key.A) then move_x = move_x - 1.0 end
            if Input.IsKeyHeld(Input.Key.D) then move_x = move_x + 1.0 end
        end

        local horizontal_len = math.sqrt(move_x * move_x + move_z * move_z)

        if horizontal_len > 0.00001 then
            move_x = move_x / horizontal_len
            move_z = move_z / horizontal_len
        end

        local grounded = false

        if self.ground_ray_node ~= nil then
            local ray = self.ground_ray_node:AsRayCast()

            if ray ~= nil then
                ray:ForceUpdate()

                if ray:IsColliding() then
                    local normal = ray:GetCollisionNormal()
                    if normal.y > 0.55 then
                        grounded = true
                    end
                end
            end
        end

        if grounded then
            self.coyote_timer = self.coyote_time
        end

        local velocity = self.body:GetLinearVelocity()

        velocity.x = move_x * self.speed
        velocity.z = move_z * self.speed

        if self.jump_buffer_timer > 0.0 and self.coyote_timer > 0.0 then
            velocity.y = self.jump_velocity

            self.jump_buffer_timer = 0.0
            self.coyote_timer = 0.0
        end

        self.body:SetLinearVelocity(velocity)
    end
}