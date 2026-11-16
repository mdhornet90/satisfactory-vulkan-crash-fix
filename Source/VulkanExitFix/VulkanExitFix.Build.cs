using UnrealBuildTool;

public class VulkanExitFix : ModuleRules
{
	public VulkanExitFix(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;

		// Only what the mod uses: RHI for RHIExit/GDynamicRHI, SML for the native hook.
		// FactoryGame and DummyHeaders follow the Alpakit C++ template, so SML's headers resolve.
		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine",
			"RHI",
			"DummyHeaders",
			"FactoryGame", "SML",
		});
	}
}
