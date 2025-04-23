#pragma once

class SimForge
{
public:
	void Init(const WindowInfo& windowInfo);
	void Update();

	void ResizeWindow(int32 width, int32 height);
};

