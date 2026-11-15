#include "Misc/AutomationTest.h"
#include "Stage/TDCStageTypes.h"
#include "Combat/TDCCombatTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// Macro (no constexpr) para compilar tanto con EAutomationTestFlags viejo (namespace) como con el enum class de 5.5+.
#define TDC_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
	FTDCLightSample MakeLight(const FVector& Location, float Radius, bool bOn)
	{
		FTDCLightSample Sample;
		Sample.Location = Location;
		Sample.Radius = Radius;
		Sample.bOn = bOn;
		return Sample;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTDCLightingIsLitTest, "TDC.Stage.Lighting.IsLit", TDC_TEST_FLAGS)
bool FTDCLightingIsLitTest::RunTest(const FString& Parameters)
{
	// Plataforma de 300x400x50 centrada en (1600, 0, -25)
	const FBox Platform(FVector(1450.f, -200.f, -50.f), FVector(1750.f, 200.f, 0.f));

	TArray<FTDCLightSample> Lights;
	TestFalse(TEXT("Sin luces no hay iluminación"), TDC::IsLit(Platform, Lights));

	Lights.Add(MakeLight(FVector(1600.f, -150.f, 350.f), 500.f, false));
	TestFalse(TEXT("Luz apagada no ilumina"), TDC::IsLit(Platform, Lights));

	Lights[0].bOn = true;
	TestTrue(TEXT("Luz prendida dentro del radio ilumina"), TDC::IsLit(Platform, Lights));

	Lights[0].Location = FVector(3000.f, 0.f, 350.f);
	TestFalse(TEXT("Luz prendida fuera del radio no ilumina"), TDC::IsLit(Platform, Lights));

	Lights.Add(MakeLight(FVector(1760.f, 0.f, 0.f), 20.f, true));
	TestTrue(TEXT("Alcanza con que una luz llegue al borde de la caja"), TDC::IsLit(Platform, Lights));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTDCLightingPresenceTest, "TDC.Stage.Lighting.Presence", TDC_TEST_FLAGS)
bool FTDCLightingPresenceTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Always con luz"), TDC::IsPresent(ETDCLightPresence::Always, true));
	TestTrue(TEXT("Always a oscuras"), TDC::IsPresent(ETDCLightPresence::Always, false));
	TestTrue(TEXT("OnlyInLight con luz"), TDC::IsPresent(ETDCLightPresence::OnlyInLight, true));
	TestFalse(TEXT("OnlyInLight a oscuras"), TDC::IsPresent(ETDCLightPresence::OnlyInLight, false));
	TestFalse(TEXT("OnlyInDark con luz"), TDC::IsPresent(ETDCLightPresence::OnlyInDark, true));
	TestTrue(TEXT("OnlyInDark a oscuras"), TDC::IsPresent(ETDCLightPresence::OnlyInDark, false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTDCTimeHistoryTest, "TDC.Stage.Time.History", TDC_TEST_FLAGS)
bool FTDCTimeHistoryTest::RunTest(const FString& Parameters)
{
	FTDCTimeHistory History(3);
	FTDCTimeFrame Out;
	TestFalse(TEXT("Pop en vacío falla"), History.Pop(Out));

	for (int32 i = 1; i <= 4; ++i)
	{
		FTDCTimeFrame Frame;
		Frame.Transform.SetLocation(FVector(static_cast<double>(i), 0.0, 0.0));
		History.Push(Frame);
	}
	TestEqual(TEXT("Capacidad respetada"), History.Num(), 3);

	TestTrue(TEXT("Pop 1"), History.Pop(Out));
	TestEqual(TEXT("LIFO: último grabado primero"), Out.Transform.GetLocation().X, 4.0);
	History.Pop(Out);
	TestEqual(TEXT("Luego el anterior"), Out.Transform.GetLocation().X, 3.0);
	History.Pop(Out);
	TestEqual(TEXT("El más viejo que sobrevivió (el 1 fue pisado)"), Out.Transform.GetLocation().X, 2.0);
	TestFalse(TEXT("Vacío al final"), History.Pop(Out));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTDCCombatResolveClapTest, "TDC.Combat.ResolveClap", TDC_TEST_FLAGS)
bool FTDCCombatResolveClapTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Común: el claquetazo lo derriba"), TDC::ResolveClap(ETDCVulnerability::Common), ETDCClapResult::Defeated);
	TestEqual(TEXT("Resistente: el claquetazo lo aturde"), TDC::ResolveClap(ETDCVulnerability::Resistant), ETDCClapResult::Stunned);
	TestEqual(TEXT("De guion: el claquetazo rebota"), TDC::ResolveClap(ETDCVulnerability::Scripted), ETDCClapResult::Immune);
	return true;
}

#endif
