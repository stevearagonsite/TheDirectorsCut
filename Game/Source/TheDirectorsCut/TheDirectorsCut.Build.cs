using UnrealBuildTool;

public class TheDirectorsCut : ModuleRules
{
	public TheDirectorsCut(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput"
		});

		PublicIncludePaths.Add(ModuleDirectory + "/Public");
	}
}
