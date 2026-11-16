#include "Debug/TDCPlaytestRunner.h"
#include "Character/TDCCharacter.h"
#include "Combat/TDCExtra.h"
#include "Stage/TDCLightReactiveComponent.h"
#include "Stage/TDCSetPiece.h"
#include "Stage/TDCStageLight.h"
#include "TheDirectorsCut.h"
#include "Dom/JsonObject.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/FileManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UnrealClient.h"

namespace
{
	// Layout de Tools/unreal/build_slice.py (X horizontal, Z vertical). Los actores se buscan por clase + cercanía,
	// así funciona también en el build empaquetado (sin labels de editor).
	const FVector L1Pos(1600.f, -150.f, 350.f);
	const FVector L2Pos(3050.f, -150.f, 350.f);
	const FVector L3Pos(6750.f, -150.f, 550.f);
	const FVector P1Pos(1600.f, 0.f, -25.f);
	const FVector W1Pos(2830.f, 0.f, 200.f);
	const FVector S1Pos(3500.f, 0.f, -25.f);
	const FVector ShelfPos(6550.f, 0.f, 275.f);
	const FVector P3Pos(6850.f, 0.f, 220.f);
	constexpr float BridgeMinX = 4800.f;
	constexpr float BridgeMaxX = 6000.f;
	constexpr float GoalMinX = 7200.f;
	constexpr float GoalMaxX = 7600.f;
	constexpr float GoalMinZ = 400.f; // techo de Exit_Ledge = 380
	constexpr float DriftTolerance = 30.f;
	constexpr int32 MaxShots = 40;
	constexpr float StandZ = 100.f; // piso en z=0 + media cápsula (90)

	struct FPieceDef { const TCHAR* Name; FVector Pos; };
	struct FLightDef { const TCHAR* Name; FVector Pos; TArray<FPieceDef> Pieces; };

	const TCHAR* ActionName(ETDCPlayAction A)
	{
		switch (A)
		{
		case ETDCPlayAction::Move:     return TEXT("move");
		case ETDCPlayAction::Jump:     return TEXT("jump");
		case ETDCPlayAction::Lights:   return TEXT("lights");
		case ETDCPlayAction::Rewind:   return TEXT("rewind");
		case ETDCPlayAction::Clap:     return TEXT("clap");
		case ETDCPlayAction::Teleport: return TEXT("teleport");
		}
		return TEXT("move");
	}

	bool ActionFromName(const FString& Name, ETDCPlayAction& Out)
	{
		for (ETDCPlayAction A : { ETDCPlayAction::Move, ETDCPlayAction::Jump, ETDCPlayAction::Lights,
			ETDCPlayAction::Rewind, ETDCPlayAction::Clap, ETDCPlayAction::Teleport })
		{
			if (Name == ActionName(A))
			{
				Out = A;
				return true;
			}
		}
		return false;
	}

	float DistXZ(const FVector& A, const FVector& B)
	{
		return static_cast<float>(FVector2D(A.X - B.X, A.Z - B.Z).Size());
	}
}

ATDCPlaytestRunner::ATDCPlaytestRunner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	SetActorHiddenInGame(true);
}

ETDCPlaytestMode ATDCPlaytestRunner::ModeFromCommandLine()
{
	const TCHAR* Cmd = FCommandLine::Get();
	FString Unused;
	if (FParse::Param(Cmd, TEXT("TDCPlaytest")))
	{
		return ETDCPlaytestMode::Scripted;
	}
	if (FParse::Param(Cmd, TEXT("TDCReplay")) || FParse::Value(Cmd, TEXT("TDCReplay="), Unused))
	{
		return ETDCPlaytestMode::Replay;
	}
	if (FParse::Param(Cmd, TEXT("TDCRecord")))
	{
		return ETDCPlaytestMode::Record;
	}
	return ETDCPlaytestMode::None;
}

void ATDCPlaytestRunner::Init(ATDCCharacter* InDirector, ETDCPlaytestMode InMode)
{
	Director = InDirector;
	Mode = InMode;

	// Paso fijo de 1/60 s (y tope de 60 fps): grabación y replay avanzan igual aunque cambie el rendimiento.
	FApp::SetUseFixedTimeStep(true);
	FApp::SetFixedDeltaTime(1.0 / 60.0);
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("t.MaxFPS 60"));
	}

	const TCHAR* ModeName = Mode == ETDCPlaytestMode::Record ? TEXT("record")
		: Mode == ETDCPlaytestMode::Replay ? TEXT("replay") : TEXT("bot");
	UE_LOG(LogTDC, Display, TEXT("[TDC-PLAYTEST] START modo=%s"), ModeName);

	if (Mode == ETDCPlaytestMode::Replay)
	{
		const FString Path = ResolveReplayPath();
		if (!LoadRun(Path))
		{
			Report(TEXT("replay_carga"), false, FString::Printf(TEXT("no pude leer la corrida '%s'"), *FPaths::GetCleanFilename(Path)));
			EndAt = 0.5;
		}
		else
		{
			UE_LOG(LogTDC, Display, TEXT("[TDC-REPLAY] corrida %s: %d eventos, %d checkpoints, %.1f s"),
				*FPaths::GetCleanFilename(Path), Events.Num(), Checkpoints.Num(), ReplayDuration);
		}
	}
}

// ---------------------------------------------------------------------------------------------------------- escena

ATDCStageLight* ATDCPlaytestRunner::FindLight(const FVector& Near) const
{
	ATDCStageLight* Best = nullptr;
	float BestDist = 400.f;
	for (TActorIterator<ATDCStageLight> It(GetWorld()); It; ++It)
	{
		const float D = DistXZ(It->GetActorLocation(), Near);
		if (D < BestDist)
		{
			BestDist = D;
			Best = *It;
		}
	}
	return Best;
}

ATDCSetPiece* ATDCPlaytestRunner::FindPiece(const FVector& Near) const
{
	ATDCSetPiece* Best = nullptr;
	float BestDist = 150.f;
	for (TActorIterator<ATDCSetPiece> It(GetWorld()); It; ++It)
	{
		if (It->Presence == ETDCLightPresence::Always)
		{
			continue;
		}
		const float D = DistXZ(It->GetActorLocation(), Near);
		if (D < BestDist)
		{
			BestDist = D;
			Best = *It;
		}
	}
	return Best;
}

ATDCExtra* ATDCPlaytestRunner::FindExtra(ETDCVulnerability Vulnerability) const
{
	for (TActorIterator<ATDCExtra> It(GetWorld()); It; ++It)
	{
		if (It->Vulnerability == Vulnerability)
		{
			return *It;
		}
	}
	return nullptr;
}

void ATDCPlaytestRunner::ScanWorld()
{
	bScanned = true;
	TArray<FLightDef> Defs;
	Defs.Add({ TEXT("L1"), L1Pos, { { TEXT("P1"), P1Pos } } });
	Defs.Add({ TEXT("L2"), L2Pos, { { TEXT("W1"), W1Pos }, { TEXT("S1"), S1Pos } } });
	Defs.Add({ TEXT("L3"), L3Pos, { { TEXT("SH"), ShelfPos }, { TEXT("P3"), P3Pos } } });
	for (const FLightDef& Def : Defs)
	{
		FTDCLightCheck Check;
		Check.Name = Def.Name;
		Check.Light = FindLight(Def.Pos);
		if (!Check.Light.IsValid())
		{
			Report(FString::Printf(TEXT("escena_%s"), Def.Name), false, TEXT("no encontré el reflector cerca de su posición de build_slice"));
			continue;
		}
		for (const FPieceDef& P : Def.Pieces)
		{
			ATDCSetPiece* Piece = FindPiece(P.Pos);
			if (!Piece)
			{
				Report(FString::Printf(TEXT("escena_%s"), P.Name), false, TEXT("no encontré la pieza de luz/oscuridad"));
				continue;
			}
			FTDCPieceCheck PieceCheck;
			PieceCheck.Name = P.Name;
			PieceCheck.Piece = Piece;
			Check.Pieces.Add(PieceCheck);
		}
		LightChecks.Add(Check);
	}

	for (TActorIterator<ATDCSetPiece> It(GetWorld()); It; ++It)
	{
		const FVector Loc = It->GetActorLocation();
		if (It->bRecordTime && !It->bStartsSimulating && Loc.X > BridgeMinX && Loc.X < BridgeMaxX)
		{
			Planks.Add(*It);
			PlankStart.Add(Loc);
		}
		else if (It->bRecordTime && It->bStartsSimulating && !Crate.IsValid())
		{
			Crate = *It;
			CrateStart = Loc;
		}
	}
	if (Planks.Num() == 0)
	{
		Report(TEXT("escena_puente"), false, TEXT("no encontré las tablas del puente (en toma, X 4800-6000)"));
	}
	ExtraCommon = FindExtra(ETDCVulnerability::Common);
	ExtraScripted = FindExtra(ETDCVulnerability::Scripted);
	ExtraResistant = FindExtra(ETDCVulnerability::Resistant);
	UE_LOG(LogTDC, Display, TEXT("[TDC-PLAYTEST] escena: %d reflectores, %d tablas, caja=%d, extras C/G/R=%d/%d/%d"),
		LightChecks.Num(), Planks.Num(), Crate.IsValid() ? 1 : 0, ExtraCommon.IsValid() ? 1 : 0,
		ExtraScripted.IsValid() ? 1 : 0, ExtraResistant.IsValid() ? 1 : 0);
}

// ---------------------------------------------------------------------------------------------------- verificación

void ATDCPlaytestRunner::Report(const FString& Step, bool bPass, const FString& Detail)
{
	if (bPass)
	{
		if (PassedSteps.Contains(Step))
		{
			return;
		}
		PassedSteps.Add(Step);
		++Passed;
	}
	else
	{
		++Failed;
	}
	UE_LOG(LogTDC, Display, TEXT("[TDC-PLAYTEST] %s %s: %s"), bPass ? TEXT("PASS") : TEXT("FAIL"), *Step, *Detail);
}

void ATDCPlaytestRunner::After(double Seconds, TFunction<void()> Run)
{
	FTDCPendingCheck Check;
	Check.At = Time + Seconds;
	Check.Run = MoveTemp(Run);
	Pending.Add(MoveTemp(Check));
}

void ATDCPlaytestRunner::Shot(const FString& Step)
{
	if (ShotIndex + ShotQueue.Num() < MaxShots)
	{
		ShotQueue.Add(Step);
	}
}

void ATDCPlaytestRunner::CheckLights(const FString& When)
{
	for (const FTDCLightCheck& Check : LightChecks)
	{
		if (!Check.Light.IsValid())
		{
			continue;
		}
		const bool bOn = Check.Light->IsOn();
		for (const FTDCPieceCheck& P : Check.Pieces)
		{
			ATDCSetPiece* Piece = P.Piece.Get();
			if (!Piece || !Piece->GetLightReactive())
			{
				continue;
			}
			const bool bExpected = Piece->Presence == ETDCLightPresence::OnlyInLight ? bOn : !bOn;
			const bool bPresent = Piece->GetLightReactive()->IsPresent();
			const bool bCollision = Piece->GetActorEnableCollision();
			Report(FString::Printf(TEXT("luces_%s_%s_%s"), *Check.Name, bOn ? TEXT("on") : TEXT("off"), *P.Name),
				bPresent == bExpected && bCollision == bExpected,
				FString::Printf(TEXT("%s (%s=%s) %s: presente=%d (esperado %d), colision=%d"), *When, *Check.Name,
					bOn ? TEXT("on") : TEXT("off"), *P.Name, bPresent ? 1 : 0, bExpected ? 1 : 0, bCollision ? 1 : 0));
		}
	}
}

void ATDCPlaytestRunner::CheckClap(const TArray<TWeakObjectPtr<ATDCExtra>>& Targets)
{
	for (const TWeakObjectPtr<ATDCExtra>& Weak : Targets)
	{
		ATDCExtra* Extra = Weak.Get();
		if (!Extra)
		{
			continue;
		}
		switch (Extra->Vulnerability)
		{
		case ETDCVulnerability::Common:
			Report(TEXT("claquetazo_comun"), Extra->IsDefeated(), Extra->IsDefeated() ? TEXT("extra común derrotado") : TEXT("el extra común sigue en pie"));
			break;
		case ETDCVulnerability::Resistant:
			Report(TEXT("claquetazo_resistente"), Extra->IsStunned() && !Extra->IsDefeated(),
				FString::Printf(TEXT("aturdido=%d derrotado=%d"), Extra->IsStunned() ? 1 : 0, Extra->IsDefeated() ? 1 : 0));
			break;
		default:
			Report(TEXT("claquetazo_guion"), !Extra->IsDefeated(), TEXT("el extra de guion es inmune al claquetazo"));
			break;
		}
	}
}

void ATDCPlaytestRunner::WatchBridge()
{
	if (bBridgeFell || Planks.Num() == 0)
	{
		return;
	}
	float MaxDrop = 0.f;
	for (int32 i = 0; i < Planks.Num(); ++i)
	{
		if (Planks[i].IsValid())
		{
			MaxDrop = FMath::Max(MaxDrop, static_cast<float>(PlankStart[i].Z - Planks[i]->GetActorLocation().Z));
		}
	}
	if (MaxDrop > 100.f)
	{
		bBridgeFell = true;
		Report(TEXT("puente_cae"), true, FString::Printf(TEXT("las tablas bajaron %.0f cm"), MaxDrop));
		Shot(TEXT("puente_cae"));
	}
}

void ATDCPlaytestRunner::CheckBridgeRestored()
{
	if (!bBridgeFell || bBridgeRestored)
	{
		return;
	}
	float MaxDist = 0.f;
	for (int32 i = 0; i < Planks.Num(); ++i)
	{
		if (Planks[i].IsValid())
		{
			MaxDist = FMath::Max(MaxDist, static_cast<float>(FVector::Dist(PlankStart[i], Planks[i]->GetActorLocation())));
		}
	}
	if (MaxDist < 25.f)
	{
		bBridgeRestored = true;
		Report(TEXT("puente_rebobinado"), true, FString::Printf(TEXT("tablas a %.0f cm o menos de su lugar"), MaxDist));
		Shot(TEXT("puente_rebobinado"));
	}
	else
	{
		UE_LOG(LogTDC, Display, TEXT("[TDC-PLAYTEST] INFO puente: soltó Q con tablas a %.0f cm (todavía no vuelve)"), MaxDist);
	}
}

void ATDCPlaytestRunner::WatchGoal()
{
	ATDCCharacter* D = Director.Get();
	if (bGoal || !D)
	{
		return;
	}
	const FVector Loc = D->GetActorLocation();
	const UCharacterMovementComponent* Move = D->GetCharacterMovement();
	if (Loc.X > GoalMinX && Loc.X < GoalMaxX && Loc.Z > GoalMinZ && Move && Move->IsMovingOnGround())
	{
		bGoal = true;
		GoalTime = Time;
		Report(TEXT("meta"), true, FString::Printf(TEXT("llegó a la salida en %.1f s"), Time));
		UE_LOG(LogTDC, Display, TEXT("[TDC-REPLAY] GOAL en %.1fs"), Time);
		Shot(TEXT("meta"));
		EndAt = Time + 2.0;
		if (Mode == ETDCPlaytestMode::Record || Mode == ETDCPlaytestMode::Scripted)
		{
			SaveRun();
		}
	}
}

void ATDCPlaytestRunner::NotifyAction(ETDCPlayAction Action, float Value, const FVector& Location)
{
	if (bFinished)
	{
		return;
	}
	if (Mode == ETDCPlaytestMode::Record || Mode == ETDCPlaytestMode::Scripted)
	{
		FTDCRunEvent Event;
		Event.T = Time;
		Event.Action = Action;
		Event.Value = Value;
		Event.Location = Location;
		Events.Add(Event);
	}

	switch (Action)
	{
	case ETDCPlayAction::Lights:
		After(0.2, [this]() { CheckLights(TEXT("después de E")); });
		Shot(TEXT("luces"));
		break;
	case ETDCPlayAction::Rewind:
		if (Value > 0.5f)
		{
			After(0.6, [this]() { Shot(TEXT("rebobinando")); });
		}
		else
		{
			After(0.3, [this]() { CheckBridgeRestored(); });
		}
		break;
	case ETDCPlayAction::Clap:
	{
		ATDCCharacter* D = Director.Get();
		if (Value < 0.5f || !D)
		{
			break; // en cooldown: no hubo golpe
		}
		Shot(TEXT("claquetazo"));
		// Extras al alcance del golpe (hacia donde mira el director), vivos antes del claquetazo.
		const float Facing = D->GetActorForwardVector().X >= 0.f ? 1.f : -1.f;
		TArray<TWeakObjectPtr<ATDCExtra>> Targets;
		for (TActorIterator<ATDCExtra> It(GetWorld()); It; ++It)
		{
			const FVector Delta = It->GetActorLocation() - Location;
			const float Ahead = static_cast<float>(Delta.X) * Facing;
			if (!It->IsDefeated() && !It->IsHidden() && Ahead > -20.f && Ahead < 190.f && FMath::Abs(Delta.Z) < 120.f)
			{
				Targets.Add(*It);
			}
		}
		if (Targets.Num() > 0)
		{
			After(0.25, [this, Targets]() { CheckClap(Targets); });
		}
		break;
	}
	case ETDCPlayAction::Teleport:
		UE_LOG(LogTDC, Display, TEXT("[TDC-PLAYTEST] teleport a X=%.0f Z=%.0f (salto de beat del bot)"), Location.X, Location.Z);
		break;
	default:
		break;
	}
}

// ------------------------------------------------------------------------------------------------------------ tick

void ATDCPlaytestRunner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bFinished || !Director.IsValid())
	{
		return;
	}
	if (!bScanned)
	{
		ScanWorld();
		After(0.3, [this]() { CheckLights(TEXT("inicio")); Shot(TEXT("inicio")); });
		if (Mode == ETDCPlaytestMode::Scripted)
		{
			BuildBot();
		}
	}
	Time += DeltaSeconds;

	switch (Mode)
	{
	case ETDCPlaytestMode::Record:   TickRecord(); break;
	case ETDCPlaytestMode::Replay:   TickReplay(); break;
	case ETDCPlaytestMode::Scripted: TickBot(); break;
	default: break;
	}

	for (int32 i = 0; i < Pending.Num();)
	{
		if (Pending[i].At <= Time)
		{
			TFunction<void()> Run = MoveTemp(Pending[i].Run);
			Pending.RemoveAt(i);
			if (Run)
			{
				Run();
			}
		}
		else
		{
			++i;
		}
	}

	// Una captura por frame (si se piden dos en el mismo frame, el motor guarda solo la última).
	if (ShotQueue.Num() > 0 && Time >= NextShotAt)
	{
		const FString Step = ShotQueue[0];
		ShotQueue.RemoveAt(0);
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("TDCPlaytest_%02d_%s"), ShotIndex++, *Step), false, false);
		NextShotAt = Time + 0.15;
	}

	WatchBridge();
	WatchGoal();

	if (Mode == ETDCPlaytestMode::Replay && !bGoal && EndAt < 0.0 && Time > ReplayDuration + 8.0)
	{
		EndAt = Time; // se terminó la corrida y no llegó a la meta
	}
	if (EndAt >= 0.0 && Time >= EndAt && ShotQueue.Num() == 0)
	{
		Finish();
	}
}

void ATDCPlaytestRunner::TickRecord()
{
	ATDCCharacter* D = Director.Get();
	if (Time >= NextCheckpoint && D)
	{
		FTDCRunCheckpoint Cp;
		Cp.T = Time;
		Cp.Location = D->GetActorLocation();
		Cp.Velocity = D->GetVelocity();
		Checkpoints.Add(Cp);
		NextCheckpoint += 0.5;
	}
}

void ATDCPlaytestRunner::TickReplay()
{
	ATDCCharacter* D = Director.Get();
	if (!D)
	{
		return;
	}
	while (ReplayEvent < Events.Num() && Events[ReplayEvent].T <= Time)
	{
		const FTDCRunEvent& E = Events[ReplayEvent++];
		if (E.Action == ETDCPlayAction::Teleport)
		{
			D->PlaytestTeleport(E.Location, E.Value);
		}
		else
		{
			D->PerformAction(E.Action, E.Value);
		}
	}
	while (ReplayCheckpoint < Checkpoints.Num() && Checkpoints[ReplayCheckpoint].T <= Time)
	{
		const FTDCRunCheckpoint& Cp = Checkpoints[ReplayCheckpoint++];
		const float Drift = static_cast<float>(FVector::Dist(D->GetActorLocation(), Cp.Location));
		if (Drift > DriftTolerance)
		{
			++DriftCorrections;
			UE_LOG(LogTDC, Display, TEXT("[TDC-REPLAY] drift %.0f cm en t=%.1f: corregido"), Drift, Cp.T);
			D->SetActorLocation(Cp.Location, false, nullptr, ETeleportType::TeleportPhysics);
			if (UCharacterMovementComponent* Move = D->GetCharacterMovement())
			{
				Move->Velocity = Cp.Velocity;
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------------------- bot

void ATDCPlaytestRunner::Teleport(const FVector& Location, float FacingX)
{
	if (ATDCCharacter* D = Director.Get())
	{
		D->PlaytestTeleport(Location, FacingX);
	}
}

void ATDCPlaytestRunner::BuildBot()
{
	auto Step = [this](double Delay, TFunction<void()> Run)
	{
		FTDCBotStep S;
		S.Delay = Delay;
		S.Run = MoveTemp(Run);
		Bot.Add(MoveTemp(S));
	};
	auto Act = [this](ETDCPlayAction A, float V)
	{
		if (ATDCCharacter* D = Director.Get())
		{
			D->PerformAction(A, V);
		}
	};

	// Beat 1: claquetazo al extra común y prender L1 (aparece P1).
	Step(1.0, [this]()
	{
		const float X = ExtraCommon.IsValid() ? static_cast<float>(ExtraCommon->GetActorLocation().X) - 110.f : 400.f;
		Teleport(FVector(FMath::Max(X, 60.f), 0.f, StandZ), 1.f);
	});
	Step(0.1, [Act]() { Act(ETDCPlayAction::Clap, 1.f); });
	Step(0.8, [this]() { Teleport(FVector(1000.f, 0.f, StandZ), 1.f); });
	Step(0.4, [Act]() { Act(ETDCPlayAction::Lights, 1.f); });
	Step(0.8, [this]() { Teleport(FVector(1600.f, 0.f, StandZ), 1.f); });
	Step(1.0, [this]()
	{
		const ATDCCharacter* D = Director.Get();
		const bool bOn = D && D->GetCharacterMovement() && D->GetCharacterMovement()->IsMovingOnGround() && D->GetActorLocation().Z > 40.f;
		Report(TEXT("p1_sostiene"), bOn, FString::Printf(TEXT("director sobre P1 con L1 prendido (Z=%.0f)"), D ? D->GetActorLocation().Z : 0.f));
	});

	// Beat 2: apagar L2 (se abre la pared, aparece S1 y la sombra de guion); prender L2 saca a la sombra de la toma.
	Step(0.2, [this]() { Teleport(FVector(2400.f, 0.f, StandZ), 1.f); });
	Step(0.4, [Act]() { Act(ETDCPlayAction::Lights, 1.f); });
	Step(0.8, [this]()
	{
		const ATDCExtra* X2 = ExtraScripted.Get();
		Report(TEXT("sombra_aparece"), X2 && !X2->IsHidden() && !X2->IsDefeated(), TEXT("con L2 apagado la sombra de guion está en escena"));
	});
	Step(0.2, [Act]() { Act(ETDCPlayAction::Lights, 1.f); });
	Step(0.6, [this]()
	{
		const ATDCExtra* X2 = ExtraScripted.Get();
		Report(TEXT("sombra_fuera_de_toma"), X2 && X2->IsDefeated(), TEXT("prender L2 de nuevo saca a la sombra de la toma"));
	});

	// Beat 3: pisar el trigger del puente, esperar que caiga y rebobinar.
	Step(0.2, [this]() { Teleport(FVector(4300.f, 0.f, StandZ), 1.f); });
	Step(0.4, [this]() { Teleport(FVector(4560.f, 0.f, StandZ), 1.f); });
	Step(2.2, [Act]() { Act(ETDCPlayAction::Rewind, 1.f); });
	Step(2.8, [Act]() { Act(ETDCPlayAction::Rewind, 0.f); });
	Step(0.8, [this]() { Teleport(FVector(5250.f, 0.f, StandZ), 1.f); });
	Step(1.0, [this]()
	{
		const ATDCCharacter* D = Director.Get();
		const bool bOn = D && D->GetCharacterMovement() && D->GetCharacterMovement()->IsMovingOnGround() && D->GetActorLocation().Z > 40.f;
		Report(TEXT("puente_sostiene"), bOn, FString::Printf(TEXT("director sobre el puente rebobinado (Z=%.0f)"), D ? D->GetActorLocation().Z : 0.f));
	});

	// Beat 4: aturdir al resistente, apagar L3 (cae la caja, aparece el escalón) y subir a la salida.
	Step(0.2, [this]()
	{
		const float X = ExtraResistant.IsValid() ? static_cast<float>(ExtraResistant->GetActorLocation().X) : 6250.f;
		const bool bLeft = X - 110.f >= 6040.f; // no pararse fuera del piso (Floor_D empieza en 6000)
		Teleport(FVector(bLeft ? X - 110.f : X + 110.f, 0.f, StandZ), bLeft ? 1.f : -1.f);
	});
	Step(0.1, [Act]() { Act(ETDCPlayAction::Clap, 1.f); });
	Step(0.6, [this]() { Teleport(FVector(6850.f, 0.f, StandZ), 1.f); });
	Step(0.4, [Act]() { Act(ETDCPlayAction::Lights, 1.f); });
	Step(1.5, [this]()
	{
		const ATDCSetPiece* C = Crate.Get();
		const float Drop = C ? static_cast<float>(CrateStart.Z - C->GetActorLocation().Z) : 0.f;
		Report(TEXT("caja_cae"), C && Drop > 50.f, FString::Printf(TEXT("con L3 apagado la caja bajó %.0f cm"), Drop));
	});
	Step(0.2, [this]() { Teleport(FVector(7400.f, 0.f, 480.f), 1.f); });
	// Si no detecta la meta en 4 s, termina igual (Finish marca FAIL meta).
	Step(4.0, [this]() { if (EndAt < 0.0) { EndAt = Time; } });
	BotNextAt = 0.0;
}

void ATDCPlaytestRunner::TickBot()
{
	if (BotIndex >= Bot.Num())
	{
		return;
	}
	if (BotNextAt <= 0.0)
	{
		BotNextAt = Time + Bot[BotIndex].Delay;
	}
	if (Time >= BotNextAt)
	{
		FTDCBotStep& S = Bot[BotIndex++];
		if (S.Run)
		{
			S.Run();
		}
		BotNextAt = BotIndex < Bot.Num() ? Time + Bot[BotIndex].Delay : 0.0;
	}
}

// ------------------------------------------------------------------------------------------------------- corridas

FString ATDCPlaytestRunner::ResolveReplayPath() const
{
	FString Arg;
	FParse::Value(FCommandLine::Get(), TEXT("TDCReplay="), Arg);
	if (Arg.IsEmpty())
	{
		Arg = TEXT("best.json");
	}
	const TArray<FString> Candidates = {
		Arg,
		FPaths::ProjectSavedDir() / TEXT("TDCRuns") / Arg,
		FPaths::ProjectDir() / TEXT("..") / TEXT("Tools") / TEXT("playtest") / TEXT("runs") / Arg,
	};
	for (const FString& C : Candidates)
	{
		if (FPaths::FileExists(C))
		{
			return C;
		}
	}
	return Candidates[1];
}

bool ATDCPlaytestRunner::LoadRun(const FString& Path)
{
	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *Path))
	{
		return false;
	}
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}
	const TArray<TSharedPtr<FJsonValue>>* EventValues = nullptr;
	if (Root->TryGetArrayField(TEXT("events"), EventValues) && EventValues)
	{
		for (const TSharedPtr<FJsonValue>& V : *EventValues)
		{
			if (!V.IsValid())
			{
				continue;
			}
			const TSharedPtr<FJsonObject> O = V->AsObject();
			FTDCRunEvent E;
			if (!O.IsValid() || !ActionFromName(O->GetStringField(TEXT("a")), E.Action))
			{
				continue;
			}
			E.T = O->GetNumberField(TEXT("t"));
			E.Value = static_cast<float>(O->GetNumberField(TEXT("v")));
			E.Location = FVector(O->GetNumberField(TEXT("x")), O->GetNumberField(TEXT("y")), O->GetNumberField(TEXT("z")));
			Events.Add(E);
		}
	}
	const TArray<TSharedPtr<FJsonValue>>* CpValues = nullptr;
	if (Root->TryGetArrayField(TEXT("checkpoints"), CpValues) && CpValues)
	{
		for (const TSharedPtr<FJsonValue>& V : *CpValues)
		{
			if (!V.IsValid())
			{
				continue;
			}
			const TSharedPtr<FJsonObject> O = V->AsObject();
			if (!O.IsValid())
			{
				continue;
			}
			FTDCRunCheckpoint Cp;
			Cp.T = O->GetNumberField(TEXT("t"));
			Cp.Location = FVector(O->GetNumberField(TEXT("x")), O->GetNumberField(TEXT("y")), O->GetNumberField(TEXT("z")));
			Cp.Velocity = FVector(O->GetNumberField(TEXT("vx")), O->GetNumberField(TEXT("vy")), O->GetNumberField(TEXT("vz")));
			Checkpoints.Add(Cp);
		}
	}
	ReplayDuration = Root->HasField(TEXT("duration")) ? Root->GetNumberField(TEXT("duration")) : 0.0;
	if (Events.Num() > 0)
	{
		ReplayDuration = FMath::Max(ReplayDuration, Events.Last().T);
	}
	return Events.Num() > 0 || Checkpoints.Num() > 0;
}

void ATDCPlaytestRunner::SaveRun()
{
	if (bSaved || (Mode != ETDCPlaytestMode::Record && Mode != ETDCPlaytestMode::Scripted))
	{
		return;
	}
	bSaved = true;

	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("version"), 1);
	Root->SetStringField(TEXT("source"), Mode == ETDCPlaytestMode::Record ? TEXT("record") : TEXT("bot"));
	Root->SetStringField(TEXT("map"), GetWorld() ? GetWorld()->GetMapName() : FString());
	Root->SetNumberField(TEXT("duration"), Time);
	Root->SetBoolField(TEXT("goal"), bGoal);
	Root->SetNumberField(TEXT("goalTime"), GoalTime);

	TArray<TSharedPtr<FJsonValue>> EventValues;
	for (const FTDCRunEvent& E : Events)
	{
		const TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
		O->SetNumberField(TEXT("t"), E.T);
		O->SetStringField(TEXT("a"), ActionName(E.Action));
		O->SetNumberField(TEXT("v"), E.Value);
		O->SetNumberField(TEXT("x"), E.Location.X);
		O->SetNumberField(TEXT("y"), E.Location.Y);
		O->SetNumberField(TEXT("z"), E.Location.Z);
		EventValues.Add(MakeShared<FJsonValueObject>(O));
	}
	Root->SetArrayField(TEXT("events"), EventValues);

	TArray<TSharedPtr<FJsonValue>> CpValues;
	for (const FTDCRunCheckpoint& Cp : Checkpoints)
	{
		const TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
		O->SetNumberField(TEXT("t"), Cp.T);
		O->SetNumberField(TEXT("x"), Cp.Location.X);
		O->SetNumberField(TEXT("y"), Cp.Location.Y);
		O->SetNumberField(TEXT("z"), Cp.Location.Z);
		O->SetNumberField(TEXT("vx"), Cp.Velocity.X);
		O->SetNumberField(TEXT("vy"), Cp.Velocity.Y);
		O->SetNumberField(TEXT("vz"), Cp.Velocity.Z);
		CpValues.Add(MakeShared<FJsonValueObject>(O));
	}
	Root->SetArrayField(TEXT("checkpoints"), CpValues);

	FString Out;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
	FJsonSerializer::Serialize(Root, Writer);

	const FString Dir = FPaths::ProjectSavedDir() / TEXT("TDCRuns");
	IFileManager::Get().MakeDirectory(*Dir, true);
	const FString Prefix = Mode == ETDCPlaytestMode::Record ? TEXT("run-") : TEXT("bot-");
	const FString Name = Prefix + FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S")) + TEXT(".json");
	FFileHelper::SaveStringToFile(Out, *(Dir / Name), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	UE_LOG(LogTDC, Display, TEXT("[TDC-REPLAY] corrida guardada: Saved/TDCRuns/%s (%d eventos, meta=%d)"), *Name, Events.Num(), bGoal ? 1 : 0);

	// best.json: la corrida de Steve que llega a la meta siempre lo reemplaza; la del bot solo si todavía no hay uno.
	const FString Best = Dir / TEXT("best.json");
	if (bGoal && (Mode == ETDCPlaytestMode::Record || !FPaths::FileExists(Best)))
	{
		FFileHelper::SaveStringToFile(Out, *Best, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		UE_LOG(LogTDC, Display, TEXT("[TDC-REPLAY] best.json actualizado (%s)"), Mode == ETDCPlaytestMode::Record ? TEXT("record") : TEXT("bot"));
	}
}

void ATDCPlaytestRunner::Finish()
{
	if (bFinished)
	{
		return;
	}
	if (Mode != ETDCPlaytestMode::Record && !bGoal)
	{
		Report(TEXT("meta"), false, TEXT("no llegó a la salida"));
	}
	if (bBridgeFell && !bBridgeRestored)
	{
		Report(TEXT("puente_rebobinado"), false, TEXT("el puente cayó y nunca volvió a su lugar"));
	}
	if (Mode == ETDCPlaytestMode::Replay)
	{
		UE_LOG(LogTDC, Display, TEXT("[TDC-REPLAY] correcciones de drift: %d"), DriftCorrections);
	}
	SaveRun();
	bFinished = true;
	const bool bOk = Failed == 0 && (bGoal || Mode == ETDCPlaytestMode::Record);
	UE_LOG(LogTDC, Display, TEXT("[TDC-PLAYTEST] RESULT %s %d/%d"), bOk ? TEXT("PASS") : TEXT("FAIL"), Passed, Passed + Failed);
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void ATDCPlaytestRunner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!bFinished)
	{
		SaveRun(); // record: Steve cerró el juego sin llegar a la meta; igual queda la corrida
	}
	FApp::SetUseFixedTimeStep(false);
	Super::EndPlay(EndPlayReason);
}
