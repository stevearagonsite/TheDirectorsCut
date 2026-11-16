#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateBrush.h"

class SButton;
class SWidgetSwitcher;
class UTexture2D;

/** Pantallas del menú (T-0035). None = jugando, sin menú en pantalla. */
enum class ETDCMenuScreen : uint8
{
	None,
	Main,
	Controls,
	Loading,
	Pause
};

/**
 * Menú inicial, controles, carga y pausa en Slate puro (sin Blueprints ni UMG).
 * Navegación: mouse, flechas/D-pad/stick (Slate mueve el foco entre botones), Enter/Espacio/A para elegir,
 * Esc/B para volver (Controles -> Principal, Pausa -> Continuar).
 * Las texturas las pasa el ATDCPlayerController, que también las mantiene vivas (UPROPERTY) para el GC.
 */
class THEDIRECTORSCUT_API STDCMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STDCMenuWidget) {}
		SLATE_ARGUMENT(UTexture2D*, TitleTexture)
		SLATE_ARGUMENT(UTexture2D*, LoadingTexture)
		SLATE_ARGUMENT(UTexture2D*, KeyFrameTexture)
		SLATE_ARGUMENT(UTexture2D*, LightsIcon)
		SLATE_ARGUMENT(UTexture2D*, RewindIcon)
		SLATE_ARGUMENT(UTexture2D*, ClapIcon)
		SLATE_EVENT(FSimpleDelegate, OnPlay)
		SLATE_EVENT(FSimpleDelegate, OnQuit)
		SLATE_EVENT(FSimpleDelegate, OnResume)
		SLATE_EVENT(FSimpleDelegate, OnRestart)
		SLATE_EVENT(FSimpleDelegate, OnMainMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Cambia de pantalla y deja el foco en su botón por defecto (también para teclado y gamepad). */
	void ShowScreen(ETDCMenuScreen Screen);
	ETDCMenuScreen GetScreen() const { return Current; }

	/** Botón que tiene que recibir el foco en la pantalla actual (nullptr en Carga). */
	TSharedPtr<SWidget> GetDefaultFocus() const;

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TSharedRef<SWidget> MakeButton(const FText& Label, const FSimpleDelegate& Action, TSharedPtr<SButton>* OutButton = nullptr);
	TSharedRef<SWidget> MakeControlRow(const FSlateBrush* Icon, const FText& Keys, const FText& What);
	TSharedRef<SWidget> MakeBackground(const FSlateBrush* Brush);
	TSharedRef<SWidget> BuildMain();
	TSharedRef<SWidget> BuildControls();
	TSharedRef<SWidget> BuildLoading();
	TSharedRef<SWidget> BuildPause();
	void FocusDefault();

	static void SetTextureBrush(FSlateBrush& Brush, UTexture2D* Texture, const FLinearColor& Fallback);

	FSlateBrush TitleBrush;
	FSlateBrush LoadingBrush;
	FSlateBrush KeyBrush;
	FSlateBrush LightsBrush;
	FSlateBrush RewindBrush;
	FSlateBrush ClapBrush;
	FSlateBrush BlackBrush;
	FSlateBrush DimBrush;
	FSlateBrush PanelBrush;
	FButtonStyle ButtonStyle;

	TSharedPtr<SWidgetSwitcher> Switcher;
	TSharedPtr<SButton> PlayButton;
	TSharedPtr<SButton> BackButton;
	TSharedPtr<SButton> ResumeButton;
	ETDCMenuScreen Current = ETDCMenuScreen::Main;

	FSimpleDelegate OnPlay;
	FSimpleDelegate OnQuit;
	FSimpleDelegate OnResume;
	FSimpleDelegate OnRestart;
	FSimpleDelegate OnMainMenu;
};
