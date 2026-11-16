#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FVulkanExitFixModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	// Deliberately empty: the hook must stay installed through shutdown, because RHIExit runs after every
	// module's ShutdownModule (FEngineLoop::Exit calls UnloadModulesAtShutdown first).
	virtual void ShutdownModule() override {}
};
