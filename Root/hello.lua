return {
	mouse_capture = true,
	mouse_delta_capture = true,

	sensitivity = 0.08,

	zoom_speed = 50.0,
	min_fov = 30.0,
	max_fov = 90.0,

	_init = function(self)
		print("Camera script initialized for node:", self.node:GetName())
	end,

	_ready = function(self)
		Input.SetMouseCaptureEnabled(self.mouse_capture)
		Input.SetMouseDeltaEnabled(self.mouse_delta_capture)
	end,

	_process = function(self, delta)
		if self.node:GetNodeType() ~= "CameraNode3d" then
			return
		end

		if self.mouse_delta_capture then
			local md = Input.GetMouseDelta()
			self.node:AsCamera():Rotate(md.x * self.sensitivity, -md.y * self.sensitivity)
		end

		local current_fov = self.node:Get("fov")

		if Input.IsKeyHeld(Input.Key.Q) then
			current_fov = current_fov - self.zoom_speed * delta
		end

		if Input.IsKeyHeld(Input.Key.E) then
			current_fov = current_fov + self.zoom_speed * delta
		end

		if current_fov < self.min_fov then
			current_fov = self.min_fov
		end

		if current_fov > self.max_fov then
			current_fov = self.max_fov
		end

		self.node:Set("fov", current_fov)

		if Input.IsKeyPressed(Input.Key.TAB) then
			self.mouse_capture = not self.mouse_capture
			self.mouse_delta_capture = not self.mouse_delta_capture

			Input.SetMouseCaptureEnabled(self.mouse_capture)
			Input.SetMouseDeltaEnabled(self.mouse_delta_capture)
		end
	end
}