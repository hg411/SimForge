#include "pch.h"
#include "SimForge.h"
#include "Engine.h"
#include "SimulationManager.h"

void SimForge::Init(const WindowInfo& windowInfo)
{
	GEngine->Init(windowInfo);

	GET_SINGLE(SimulationManager)->LoadSimulation();
}

void SimForge::Update()
{
	GEngine->Update();
}
