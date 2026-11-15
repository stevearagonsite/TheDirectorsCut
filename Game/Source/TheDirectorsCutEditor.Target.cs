using UnrealBuildTool;

public class TheDirectorsCutEditorTarget : TargetRules
{
	public TheDirectorsCutEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("TheDirectorsCut");
	}
}
