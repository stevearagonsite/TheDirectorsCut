#include "UI/TDCMenuWidget.h"
#include "Brushes/SlateColorBrush.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "TDCMenu"

namespace
{
	const FLinearColor TextColor(0.92f, 0.92f, 0.95f);
	// Amarillo de la luz: es el único acento saturado del menú, igual que en el título.
	const FLinearColor AccentColor(1.f, 0.8f, 0.2f);

	int32 IndexOf(ETDCMenuScreen Screen)
	{
		switch (Screen)
		{
		case ETDCMenuScreen::Controls: return 1;
		case ETDCMenuScreen::Loading:  return 2;
		case ETDCMenuScreen::Pause:    return 3;
		default:                       return 0;
		}
	}
}

void STDCMenuWidget::SetTextureBrush(FSlateBrush& Brush, UTexture2D* Texture, const FLinearColor& Fallback)
{
	if (Texture)
	{
		Brush = FSlateBrush();
		Brush.SetResourceObject(Texture);
		Brush.ImageSize = FVector2D(static_cast<double>(Texture->GetSizeX()), static_cast<double>(Texture->GetSizeY()));
		Brush.DrawAs = ESlateBrushDrawType::Image;
	}
	else
	{
		// Sin la textura importada (falta import_ui.py) el menú igual funciona, sobre un color liso.
		Brush = FSlateColorBrush(Fallback);
	}
}

void STDCMenuWidget::Construct(const FArguments& InArgs)
{
	OnPlay = InArgs._OnPlay;
	OnQuit = InArgs._OnQuit;
	OnResume = InArgs._OnResume;
	OnRestart = InArgs._OnRestart;
	OnMainMenu = InArgs._OnMainMenu;

	SetTextureBrush(TitleBrush, InArgs._TitleTexture, FLinearColor(0.02f, 0.02f, 0.04f));
	SetTextureBrush(LoadingBrush, InArgs._LoadingTexture, FLinearColor(0.f, 0.f, 0.f));
	SetTextureBrush(KeyBrush, InArgs._KeyFrameTexture, FLinearColor(0.12f, 0.12f, 0.14f));
	if (InArgs._KeyFrameTexture)
	{
		// El marco de tecla se estira a lo ancho sin deformar las esquinas.
		KeyBrush.DrawAs = ESlateBrushDrawType::Box;
		KeyBrush.Margin = FMargin(0.3f);
	}
	SetTextureBrush(LightsBrush, InArgs._LightsIcon, FLinearColor(0.f, 0.f, 0.f, 0.f));
	SetTextureBrush(RewindBrush, InArgs._RewindIcon, FLinearColor(0.f, 0.f, 0.f, 0.f));
	SetTextureBrush(ClapBrush, InArgs._ClapIcon, FLinearColor(0.f, 0.f, 0.f, 0.f));
	BlackBrush = FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 1.f));
	DimBrush = FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.6f));
	PanelBrush = FSlateColorBrush(FLinearColor(0.03f, 0.03f, 0.05f, 0.88f));

	ButtonStyle = FButtonStyle()
		.SetNormal(FSlateNoResource())
		.SetHovered(FSlateNoResource())
		.SetPressed(FSlateNoResource())
		.SetDisabled(FSlateNoResource());

	ChildSlot
	[
		SAssignNew(Switcher, SWidgetSwitcher)
		.WidgetIndex(0)
		+ SWidgetSwitcher::Slot() [ BuildMain() ]
		+ SWidgetSwitcher::Slot() [ BuildControls() ]
		+ SWidgetSwitcher::Slot() [ BuildLoading() ]
		+ SWidgetSwitcher::Slot() [ BuildPause() ]
	];
}

TSharedRef<SWidget> STDCMenuWidget::MakeButton(const FText& Label, const FSimpleDelegate& Action, TSharedPtr<SButton>* OutButton)
{
	// El botón "activo" (con foco de teclado/gamepad o con el mouse encima) se pinta en amarillo y con marca.
	TSharedRef<TWeakPtr<SButton>> Self = MakeShared<TWeakPtr<SButton>>();
	auto IsActive = [Self]() -> bool
	{
		const TSharedPtr<SButton> Button = Self->Pin();
		return Button.IsValid() && (Button->IsHovered() || Button->HasAnyUserFocus().IsSet());
	};

	TSharedRef<SButton> Button = SNew(SButton)
		.ButtonStyle(&ButtonStyle)
		.ContentPadding(FMargin(20.f, 6.f))
		.OnClicked_Lambda([Action]()
		{
			Action.ExecuteIfBound();
			return FReply::Handled();
		})
		.OnHovered_Lambda([Self]()
		{
			// El mouse mueve el foco: así teclado y mouse no quedan en botones distintos.
			if (const TSharedPtr<SButton> Hovered = Self->Pin())
			{
				FSlateApplication::Get().SetAllUserFocus(Hovered, EFocusCause::Mouse);
			}
		})
		[
			SNew(STextBlock)
			.Text_Lambda([Label, IsActive]()
			{
				return IsActive() ? FText::Format(LOCTEXT("ActiveButton", "> {0}"), Label) : Label;
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 34))
			.ColorAndOpacity_Lambda([IsActive]()
			{
				return FSlateColor(IsActive() ? AccentColor : TextColor);
			})
			.ShadowOffset(FVector2D(2.f, 2.f))
			.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
		];

	*Self = Button;
	if (OutButton)
	{
		*OutButton = Button;
	}
	return Button;
}

TSharedRef<SWidget> STDCMenuWidget::MakeBackground(const FSlateBrush* Brush)
{
	return SNew(SBorder)
		.BorderImage(&BlackBrush)
		.Padding(0.f)
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFill)
			[
				SNew(SImage).Image(Brush)
			]
		];
}

TSharedRef<SWidget> STDCMenuWidget::MakeControlRow(const FSlateBrush* Icon, const FText& Keys, const FText& What)
{
	TSharedRef<SWidget> IconWidget = SNullWidget::NullWidget;
	if (Icon)
	{
		IconWidget = SNew(SImage).Image(Icon);
	}

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 6.f, 16.f, 6.f)
		[
			SNew(SBox)
			.WidthOverride(56.f)
			.HeightOverride(56.f)
			[
				IconWidget
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 20.f, 0.f)
		[
			SNew(SBox)
			.MinDesiredWidth(170.f)
			.HeightOverride(52.f)
			[
				SNew(SBorder)
				.BorderImage(&KeyBrush)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(FMargin(14.f, 4.f))
				[
					SNew(STextBlock)
					.Text(Keys)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
					.ColorAndOpacity(FSlateColor(TextColor))
				]
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(What)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 24))
			.ColorAndOpacity(FSlateColor(TextColor))
		];
}

TSharedRef<SWidget> STDCMenuWidget::BuildMain()
{
	// El fondo ya trae el título arriba a la izquierda (title_screen); los botones van abajo a la izquierda,
	// sobre la zona oscurecida del arte.
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			MakeBackground(&TitleBrush)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(110.f, 0.f, 0.f, 120.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
			[
				MakeButton(LOCTEXT("Play", "Jugar"), OnPlay, &PlayButton)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
			[
				MakeButton(LOCTEXT("Controls", "Controles"),
					FSimpleDelegate::CreateSP(this, &STDCMenuWidget::ShowScreen, ETDCMenuScreen::Controls))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
			[
				MakeButton(LOCTEXT("Quit", "Salir"), OnQuit)
			]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.f, 0.f, 60.f, 40.f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MenuHint", "Enter / A: elegir      Esc / B: volver"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.75f, 0.8f, 0.8f)))
		];
}

TSharedRef<SWidget> STDCMenuWidget::BuildControls()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			MakeBackground(&TitleBrush)
		]
		+ SOverlay::Slot()
		[
			SNew(SBorder).BorderImage(&DimBrush)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(48.f, 36.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 20.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ControlsTitle", "CONTROLES"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 40))
					.ColorAndOpacity(FSlateColor(AccentColor))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeControlRow(nullptr, LOCTEXT("KeysMove", "A / D  o  stick"), LOCTEXT("WhatMove", "Moverse"))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeControlRow(nullptr, LOCTEXT("KeysJump", "Espacio / A"), LOCTEXT("WhatJump", "Saltar"))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeControlRow(&LightsBrush, LOCTEXT("KeysLights", "E / X"),
						LOCTEXT("WhatLights", "Luces: prender o apagar el reflector cercano"))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeControlRow(&RewindBrush, LOCTEXT("KeysRewind", "Q / LT (mantener)"),
						LOCTEXT("WhatRewind", "Rebobinar la toma (lo que está en cian)"))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeControlRow(&ClapBrush, LOCTEXT("KeysClap", "F / clic / B"),
						LOCTEXT("WhatClap", "Claquetazo: derriba o aturde extras"))
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeControlRow(nullptr, LOCTEXT("KeysPause", "Esc / Start"), LOCTEXT("WhatPause", "Pausa"))
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 24.f, 0.f, 0.f)
				[
					MakeButton(LOCTEXT("Back", "Volver"),
						FSimpleDelegate::CreateSP(this, &STDCMenuWidget::ShowScreen, ETDCMenuScreen::Main), &BackButton)
				]
			]
		];
}

TSharedRef<SWidget> STDCMenuWidget::BuildLoading()
{
	return MakeBackground(&LoadingBrush);
}

TSharedRef<SWidget> STDCMenuWidget::BuildPause()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SBorder).BorderImage(&DimBrush)
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(64.f, 40.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.f, 0.f, 0.f, 20.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PauseTitle", "PAUSA"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 44))
					.ColorAndOpacity(FSlateColor(AccentColor))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
				[
					MakeButton(LOCTEXT("Resume", "Continuar"), OnResume, &ResumeButton)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
				[
					MakeButton(LOCTEXT("Restart", "Reiniciar"), OnRestart)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f)
				[
					MakeButton(LOCTEXT("MainMenu", "Salir al menú"), OnMainMenu)
				]
			]
		];
}

void STDCMenuWidget::ShowScreen(ETDCMenuScreen Screen)
{
	Current = Screen;
	if (Switcher.IsValid() && Screen != ETDCMenuScreen::None)
	{
		Switcher->SetActiveWidgetIndex(IndexOf(Screen));
	}
	if (Screen != ETDCMenuScreen::None)
	{
		FocusDefault(); // None = jugando: el foco lo devuelve SetInputMode(GameOnly) al viewport
	}
}

TSharedPtr<SWidget> STDCMenuWidget::GetDefaultFocus() const
{
	switch (Current)
	{
	case ETDCMenuScreen::Main:     return PlayButton;
	case ETDCMenuScreen::Controls: return BackButton;
	case ETDCMenuScreen::Pause:    return ResumeButton;
	default:                       return nullptr;
	}
}

void STDCMenuWidget::FocusDefault()
{
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}
	TSharedPtr<SWidget> Target = GetDefaultFocus();
	if (!Target.IsValid())
	{
		Target = SharedThis(this); // carga: el foco queda en el menú para que nada llegue al juego
	}
	FSlateApplication::Get().SetAllUserFocus(Target, EFocusCause::SetDirectly);
}

FReply STDCMenuWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const bool bBack = Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Right;
	if (bBack)
	{
		if (Current == ETDCMenuScreen::Controls)
		{
			ShowScreen(ETDCMenuScreen::Main);
			return FReply::Handled();
		}
		if (Current == ETDCMenuScreen::Pause)
		{
			OnResume.ExecuteIfBound();
			return FReply::Handled();
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
