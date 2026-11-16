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

		// Menú inicial y pausa en Slate puro (T-0035).
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Json: corridas grabadas del playtest automático (Saved/TDCRuns/*.json).
		PrivateDependencyModuleNames.AddRange(new string[] { "Json" });

		PublicIncludePaths.Add(ModuleDirectory + "/Public");
	}
}
