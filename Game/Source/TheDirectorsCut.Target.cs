using UnrealBuildTool;

public class TheDirectorsCutTarget : TargetRules
{
	public TheDirectorsCutTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TheDirectorsCut");
	}
}
