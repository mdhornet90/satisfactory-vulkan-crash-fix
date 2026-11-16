// Vulkan Exit Crash Fix
//
// With the Vulkan RHI, Satisfactory crashes on every quit to desktop: RHIExit -> FVulkanDynamicRHI::Shutdown ->
// FVulkanDevice::Destroy flushes the immediate command list, and FVulkanDynamicRHI::RHIFinalizeContext then makes a
// virtual call on a command context that was already freed (EXCEPTION_ACCESS_VIOLATION executing address 0).
//
// By the time RHIExit runs, everything that persists has already happened (saving, Steam/EOS shutdown, every module's
// ShutdownModule). What a clean exit still does afterwards is free GPU/driver memory, which the OS reclaims anyway,
// and AppExit's config flush and log teardown, which this mod does itself. So on Vulkan it replaces RHIExit with:
// flush config, close the log, end the process with exit code 0. On any other RHI it does nothing.

#include "VulkanExitFix.h"

#include "DynamicRHI.h"
#include "HAL/PlatformMisc.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Patching/NativeHookManager.h"
#include "RHI.h"

DEFINE_LOG_CATEGORY_STATIC(LogVulkanExitFix, Log, All);

namespace
{
	bool IsVulkanRHI()
	{
		return GDynamicRHI != nullptr && FCString::Strcmp(GDynamicRHI->GetName(), TEXT("Vulkan")) == 0;
	}

	void ExitWithoutVulkanTeardown()
	{
		UE_LOG(LogVulkanExitFix, Log,
			TEXT("Vulkan RHI: skipping the Vulkan device teardown (it crashes on exit). Saving config, closing the log and ending the process."));

		// What FEngineLoop::AppExit would do later: GConfig->Exit() is Flush(true); Flush(false) writes the same files.
		if (GConfig)
		{
			GConfig->Flush(false);
		}

		// Ends the log with "Log file closed", the same marker a clean exit leaves.
		if (GLog)
		{
			GLog->Flush();
			GLog->TearDown();
		}

		// Forced exit: TerminateProcess with exit code 0 (3 only if GIsCriticalError).
		FPlatformMisc::RequestExit(true, TEXT("VulkanExitFix"));
	}
}

void FVulkanExitFixModule::StartupModule()
{
#if !WITH_EDITOR
	// Not in the editor: the Linux editor runs on Vulkan too, and its shutdown must stay untouched.
	SUBSCRIBE_METHOD(RHIExit, [](auto& Scope)
	{
		if (!IsVulkanRHI())
		{
			return; // DX11, DX12, the null RHI on servers: the normal RHIExit runs.
		}
		ExitWithoutVulkanTeardown();
		Scope.Cancel(); // Not reached; RequestExit(true) doesn't return.
	});
	UE_LOG(LogVulkanExitFix, Log, TEXT("Hooked RHIExit (acts only when the RHI is Vulkan)."));
#endif
}

IMPLEMENT_MODULE(FVulkanExitFixModule, VulkanExitFix)
