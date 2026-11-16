#include "Core/TDCPlayerController.h"
#include "Core/TDCVisuals.h"
#include "TheDirectorsCut.h"
#include "Components/InputComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/HUD.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

namespace
{
	const TCHAR* NoMenuOption = TEXT("TDCNoMenu");
}

ATDCPlayerController::ATDCPlayerController()
{
}

bool ATDCPlayerController::IsMenuDisabledByCommandLine()
{
	const TCHAR* Cmd = FCommandLine::Get();
	FString Ignored;
	return FParse::Param(Cmd, TEXT("TDCAutoRun")) || FParse::Param(Cmd, TEXT("TDCPlaytest"))
		|| FParse::Param(Cmd, TEXT("TDCRecord")) || FParse::Param(Cmd, TEXT("TDCReplay"))
		|| FParse::Value(Cmd, TEXT("TDCReplay="), Ignored) || FParse::Param(Cmd, NoMenuOption);
}

void ATDCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController())
	{
		return;
	}
	const UWorld* World = GetWorld();
	const bool bSkipMenu = IsMenuDisabledByCommandLine() || (World && World->URL.HasOption(NoMenuOption));
	if (bSkipMenu)
	{
		StartTake();
		return;
	}
	ShowMenu(ETDCMenuScreen::Main);
	UGameplayStatics::SetGamePaused(this, true);
	UE_LOG(LogTDC, Log, TEXT("[TDC-MENU] menú principal visible"));
}

void ATDCPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LoadingTimer);
	// El contenido del viewport sobrevive al cambio de nivel: hay que sacarlo a mano.
	if (bMenuInViewport && Menu.IsValid())
	{
		if (UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
		{
			Viewport->RemoveViewportWidgetContent(Menu.ToSharedRef());
		}
	}
	bMenuInViewport = false;
	Menu.Reset();
	Super::EndPlay(EndPlayReason);
}

void ATDCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// Pausa: bindings clásicos (conviven con Enhanced Input del personaje). bExecuteWhenPaused por si ya está en pausa.
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ATDCPlayerController::HandlePausePressed).bExecuteWhenPaused = true;
	InputComponent->BindKey(EKeys::Gamepad_Special_Right, IE_Pressed, this, &ATDCPlayerController::HandlePausePressed).bExecuteWhenPaused = true;
}

UTexture2D* ATDCPlayerController::LoadUITexture(const TCHAR* Path)
{
	UTexture2D* Texture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, Path, nullptr,
		LOAD_NoWarn | LOAD_Quiet));
	if (Texture)
	{
		UITextures.Add(Texture);
	}
	return Texture;
}

void ATDCPlayerController::EnsureMenu()
{
	if (!Menu.IsValid())
	{
		// Texturas de import_ui.py; si faltan, el menú se arma igual sobre colores lisos.
		SAssignNew(Menu, STDCMenuWidget)
			.TitleTexture(LoadUITexture(TEXT("/Game/UI/T_UI_title_screen_1920x1080.T_UI_title_screen_1920x1080")))
			.LoadingTexture(LoadUITexture(TEXT("/Game/UI/T_UI_loading_1920x1080.T_UI_loading_1920x1080")))
			.KeyFrameTexture(LoadUITexture(TEXT("/Game/UI/HUD/T_UI_key_frame.T_UI_key_frame")))
			.LightsIcon(LoadUITexture(TEXT("/Game/UI/HUD/T_UI_icon_luces.T_UI_icon_luces")))
			.RewindIcon(LoadUITexture(TEXT("/Game/UI/HUD/T_UI_icon_rebobinar.T_UI_icon_rebobinar")))
			.ClapIcon(LoadUITexture(TEXT("/Game/UI/HUD/T_UI_icon_claquetazo.T_UI_icon_claquetazo")))
			.OnPlay(FSimpleDelegate::CreateUObject(this, &ATDCPlayerController::HandlePlay))
			.OnQuit(FSimpleDelegate::CreateUObject(this, &ATDCPlayerController::HandleQuit))
			.OnResume(FSimpleDelegate::CreateUObject(this, &ATDCPlayerController::HandleResume))
			.OnRestart(FSimpleDelegate::CreateUObject(this, &ATDCPlayerController::HandleRestart))
			.OnMainMenu(FSimpleDelegate::CreateUObject(this, &ATDCPlayerController::HandleMainMenu));
	}
	if (!bMenuInViewport)
	{
		if (UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
		{
			Viewport->AddViewportWidgetContent(Menu.ToSharedRef(), 50);
			bMenuInViewport = true;
		}
	}
}

void ATDCPlayerController::ShowMenu(ETDCMenuScreen Screen)
{
	EnsureMenu();
	if (!Menu.IsValid())
	{
		return;
	}
	Menu->SetVisibility(EVisibility::Visible);
	Menu->ShowScreen(Screen);

	FInputModeUIOnly Mode;
	TSharedPtr<SWidget> Focus = Menu->GetDefaultFocus();
	Mode.SetWidgetToFocus(Focus.IsValid() ? Focus : TSharedPtr<SWidget>(Menu));
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
	bShowMouseCursor = Screen != ETDCMenuScreen::Loading;
	if (AHUD* Hud = GetHUD())
	{
		Hud->bShowHUD = false; // el HUD in-game (ATDCHUD) no se dibuja debajo del menú
	}
	// SetInputMode cambia el foco de Slate: volver a ponerlo en el botón por defecto.
	Menu->ShowScreen(Screen);
}

void ATDCPlayerController::HideMenu()
{
	if (Menu.IsValid())
	{
		Menu->ShowScreen(ETDCMenuScreen::None);
		Menu->SetVisibility(EVisibility::Collapsed);
	}
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	if (AHUD* Hud = GetHUD())
	{
		Hud->bShowHUD = true;
	}
}

void ATDCPlayerController::StartTake()
{
	TDC::PlaySfx2D(this, TEXT("vo_accion"), 0.9f); // "¡Acción!": empieza la toma
}

void ATDCPlayerController::HandlePlay()
{
	ShowMenu(ETDCMenuScreen::Loading);
	// El mundo corre detrás de la pantalla de carga (los timers necesitan el juego despausado);
	// el director no recibe input hasta FinishLoading porque el modo de input es solo UI.
	UGameplayStatics::SetGamePaused(this, false);
	if (LoadingSeconds > 0.f)
	{
		GetWorldTimerManager().SetTimer(LoadingTimer, this, &ATDCPlayerController::FinishLoading, LoadingSeconds, false);
	}
	else
	{
		FinishLoading();
	}
}

void ATDCPlayerController::FinishLoading()
{
	HideMenu();
	StartTake();
	UE_LOG(LogTDC, Log, TEXT("[TDC-MENU] Jugar: empieza la toma"));
}

void ATDCPlayerController::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ATDCPlayerController::HandleResume()
{
	HideMenu();
	UGameplayStatics::SetGamePaused(this, false);
}

void ATDCPlayerController::HandleRestart()
{
	// Reabre el nivel sin menú: el jugador vuelve directo a la toma.
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true, NoMenuOption);
}

void ATDCPlayerController::HandleMainMenu()
{
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)), true);
}

void ATDCPlayerController::HandlePausePressed()
{
	// Solo desde el juego: en el menú la tecla la maneja el widget (Volver / Continuar).
	if (Menu.IsValid() && Menu->GetScreen() != ETDCMenuScreen::None)
	{
		return;
	}
	ShowMenu(ETDCMenuScreen::Pause);
	UGameplayStatics::SetGamePaused(this, true);
}
