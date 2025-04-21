#include "pch.h"
#include "SimForge.h"
#include "Engine.h"

void SimForge::Init(const WindowInfo& windowInfo)
{
	GEngine->Init(windowInfo);


}

void SimForge::Update()
{
	GEngine->Update();
}
