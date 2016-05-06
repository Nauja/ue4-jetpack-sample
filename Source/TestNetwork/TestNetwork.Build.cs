using UnrealBuildTool;

public class TestNetwork : ModuleRules
{
	public TestNetwork(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
	}
}
