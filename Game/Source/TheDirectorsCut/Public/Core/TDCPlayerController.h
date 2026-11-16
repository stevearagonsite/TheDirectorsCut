#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/TDCMenuWidget.h"
#include "TDCPlayerController.generated.h"

class UTexture2D;

/**
 * Menú inicial y pausa (T-0035). L_Slice arranca en "modo menú": el juego queda en pausa debajo del menú hasta Jugar.
 * Jugar -> pantalla de carga 0.8 s -> "¡Acción!" y control al jugador. Esc / Start durante el juego -> Pausa.
 * El menú no aparece con -TDCAutoRun, -TDCPlaytest, -TDCRecord, -TDCReplay, -TDCNoMenu ni al reiniciar (?TDCNoMenu).
 */
UCLASS()
class THEDIRECTORSCUT_API ATDCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATDCPlayerController();

	/** true si la línea de comandos pide jugar sin menú (demos, tests, playtest). */
	static bool IsMenuDisabledByCommandLine();

	/** Segundos de la pantalla de carga entre Jugar y el control del director. */
	UPROPERTY(EditDefaultsOnly, Category = "TDC|Menu", meta = (ClampMin = "0"))
	float LoadingSeconds = 0.8f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

private:
	void EnsureMenu();
	void ShowMenu(ETDCMenuScreen Screen);
	void HideMenu();
	void StartTake();

	void HandlePlay();
	void FinishLoading();
	void HandleQuit();
	void HandleResume();
	void HandleRestart();
	void HandleMainMenu();
	void HandlePausePressed();

	UTexture2D* LoadUITexture(const TCHAR* Path);

	TSharedPtr<STDCMenuWidget> Menu;
	bool bMenuInViewport = false;
	FTimerHandle LoadingTimer;

	/** Mantiene vivas las texturas que usan los brushes de Slate (Slate no las referencia para el GC). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> UITextures;
};
