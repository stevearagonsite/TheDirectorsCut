#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Debug/TDCPlaytestTypes.h"
#include "Combat/TDCCombatTypes.h"
#include "TDCPlaytestRunner.generated.h"

class ATDCCharacter;
class ATDCStageLight;
class ATDCSetPiece;
class ATDCExtra;

/** Un evento de la corrida grabada (segundos desde el inicio). */
struct FTDCRunEvent
{
	double T = 0.0;
	ETDCPlayAction Action = ETDCPlayAction::Move;
	float Value = 0.f;
	FVector Location = FVector::ZeroVector;
};

/** Posición y velocidad del director cada 0.5 s: el replay corrige el desvío contra esto. */
struct FTDCRunCheckpoint
{
	double T = 0.0;
	FVector Location = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
};

struct FTDCPieceCheck
{
	FString Name;
	TWeakObjectPtr<ATDCSetPiece> Piece;
};

/** Reflector + piezas que dependen solo de él (para verificar presencia y colisión). */
struct FTDCLightCheck
{
	FString Name;
	TWeakObjectPtr<ATDCStageLight> Light;
	TArray<FTDCPieceCheck> Pieces;
};

struct FTDCPendingCheck
{
	double At = 0.0;
	TFunction<void()> Run;
};

struct FTDCBotStep
{
	double Delay = 0.0; // espera antes de ejecutar este paso
	TFunction<void()> Run;
};

/**
 * Playtest automático (T-0032). Lo crea ATDCCharacter si la línea de comandos trae:
 *  -TDCRecord           graba los inputs del jugador + checkpoints en Saved/TDCRuns/run-<fecha>.json (meta -> best.json)
 *  -TDCReplay[=archivo] reproduce una corrida (por defecto best.json) corrigiendo desvíos > 30 cm
 *  -TDCPlaytest         bot guionado por beats (teletransporta entre beats) que además graba la corrida
 * En todos los modos verifica la escena y loguea "[TDC-PLAYTEST] PASS|FAIL <paso>: <detalle>", saca capturas
 * TDCPlaytest_<NN>_<paso>.png y al final "[TDC-PLAYTEST] RESULT PASS|FAIL n/m" (record y replay/bot salen solos).
 */
UCLASS(NotPlaceable, Transient)
class THEDIRECTORSCUT_API ATDCPlaytestRunner : public AActor
{
	GENERATED_BODY()

public:
	ATDCPlaytestRunner();

	static ETDCPlaytestMode ModeFromCommandLine();

	void Init(ATDCCharacter* InDirector, ETDCPlaytestMode InMode);

	/** Lo llama el personaje con cada acción (jugador, replay o bot): la graba y programa las verificaciones. */
	void NotifyAction(ETDCPlayAction Action, float Value, const FVector& Location);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Escena
	void ScanWorld();
	ATDCStageLight* FindLight(const FVector& Near) const;
	ATDCSetPiece* FindPiece(const FVector& Near) const;
	ATDCExtra* FindExtra(ETDCVulnerability Vulnerability) const;

	// Verificaciones
	void Report(const FString& Step, bool bPass, const FString& Detail);
	void CheckLights(const FString& When);
	void CheckClap(const TArray<TWeakObjectPtr<ATDCExtra>>& Targets);
	void CheckBridgeRestored();
	void WatchBridge();
	void WatchGoal();
	void After(double Seconds, TFunction<void()> Run);
	void Shot(const FString& Step);

	// Modos
	void TickRecord();
	void TickReplay();
	void TickBot();
	void BuildBot();
	void Teleport(const FVector& Location, float FacingX);
	bool LoadRun(const FString& Path);
	void SaveRun();
	FString ResolveReplayPath() const;
	void Finish();

	ETDCPlaytestMode Mode = ETDCPlaytestMode::None;
	TWeakObjectPtr<ATDCCharacter> Director;
	double Time = 0.0;
	bool bScanned = false;
	bool bFinished = false;
	bool bSaved = false;
	double EndAt = -1.0;

	// Escena
	TArray<FTDCLightCheck> LightChecks;
	TArray<TWeakObjectPtr<ATDCSetPiece>> Planks;
	TArray<FVector> PlankStart;
	TWeakObjectPtr<ATDCSetPiece> Crate;
	FVector CrateStart = FVector::ZeroVector;
	TWeakObjectPtr<ATDCExtra> ExtraCommon;
	TWeakObjectPtr<ATDCExtra> ExtraScripted;
	TWeakObjectPtr<ATDCExtra> ExtraResistant;
	bool bBridgeFell = false;
	bool bBridgeRestored = false;
	bool bGoal = false;
	double GoalTime = 0.0;

	// Resultado
	int32 Passed = 0;
	int32 Failed = 0;
	TSet<FString> PassedSteps;
	TArray<FTDCPendingCheck> Pending;
	TArray<FString> ShotQueue;
	double NextShotAt = 0.0;
	int32 ShotIndex = 0;

	// Corrida
	TArray<FTDCRunEvent> Events;
	TArray<FTDCRunCheckpoint> Checkpoints;
	double NextCheckpoint = 0.5;
	int32 ReplayEvent = 0;
	int32 ReplayCheckpoint = 0;
	double ReplayDuration = 0.0;
	int32 DriftCorrections = 0;

	// Bot
	TArray<FTDCBotStep> Bot;
	int32 BotIndex = 0;
	double BotNextAt = 0.0;
};
