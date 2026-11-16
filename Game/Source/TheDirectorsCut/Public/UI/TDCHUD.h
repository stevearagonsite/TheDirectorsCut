#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Subsystems/WorldSubsystem.h"
#include "TDCHUD.generated.h"

class UTexture2D;
class UFont;

/** Dónde se dibuja un mensaje del HUD. */
UENUM()
enum class ETDCHudSlot : uint8
{
	Prompt, // abajo al centro, encima de los toasts: "[E] ¡Luces!"
	Toast,  // abajo al centro: feedback del claquetazo, avisos de la escena
	Banner  // arriba al centro, grande: "REBOBINANDO"
};

struct FTDCHudMessage
{
	FString Text;
	FLinearColor Color = FLinearColor::White;
	ETDCHudSlot Slot = ETDCHudSlot::Toast;
	double ExpireTime = 0.0; // < 0 = hasta que se borre
	double PostTime = 0.0;
};

/** Mensajes del HUD por clave (reemplaza a GEngine->AddOnScreenDebugMessage, que no se ve en Shipping y luce a debug). */
UCLASS()
class THEDIRECTORSCUT_API UTDCHudMessages : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void Post(int32 Key, const FString& Text, const FLinearColor& Color, float Duration, ETDCHudSlot Slot);
	void Clear(int32 Key);
	/** El personaje lo llama cada frame mientras el panel de controles tenga que verse. */
	void RequestControls() { ControlsRequestTime = NowSeconds(); }

	bool WantsControls() const { return NowSeconds() - ControlsRequestTime < 0.25; }
	const TMap<int32, FTDCHudMessage>& GetMessages() const { return Messages; }
	double NowSeconds() const;
	void Prune();

private:
	TMap<int32, FTDCHudMessage> Messages;
	double ControlsRequestTime = -100.0;
};

namespace TDC
{
	/** Duration < 0 = hasta Clear. Sin mundo/subsystem no hace nada. */
	THEDIRECTORSCUT_API void HudMessage(const UObject* WorldContext, int32 Key, const FString& Text, const FLinearColor& Color,
		float Duration, ETDCHudSlot Slot = ETDCHudSlot::Toast);
	THEDIRECTORSCUT_API void HudClear(const UObject* WorldContext, int32 Key);
}

/**
 * HUD del juego dibujado con Canvas (sin assets obligatorios): panel de controles con íconos (si están importados en
 * /Game/UI/HUD), prompt de acción, toasts y banner de rebobinado con barras cian. Escala con la altura de pantalla (1080p).
 */
UCLASS()
class THEDIRECTORSCUT_API ATDCHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

private:
	void DrawControls(UTDCHudMessages& Hud, float Scale);
	void DrawMessages(UTDCHudMessages& Hud, float Scale);
	void DrawPlate(float X, float Y, float W, float H, const FLinearColor& Color);
	FVector2D TextSize(const FString& Text, UFont* Font, float Scale);
	void DrawShadowedText(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Color);
	UTexture2D* LoadUi(const TCHAR* Name) const;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> IconLights;
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> IconRewind;
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> IconClap;
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> KeyFrame;
};
