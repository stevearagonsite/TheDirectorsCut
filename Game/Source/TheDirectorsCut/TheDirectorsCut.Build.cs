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

		// Json: corridas grabadas del playtest automático (Saved/TDCRuns/*.json).
		PrivateDependencyModuleNames.AddRange(new string[] { "Json" });

		PublicIncludePaths.Add(ModuleDirectory + "/Public");
	}
}
