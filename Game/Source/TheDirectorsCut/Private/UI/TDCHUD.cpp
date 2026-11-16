#include "UI/TDCHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "CanvasItem.h"

// ---------------------------------------------------------------- mensajes

double UTDCHudMessages::NowSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetRealTimeSeconds() : 0.0; // tiempo real: el hit-stop y la pausa no congelan los avisos
}

void UTDCHudMessages::Post(int32 Key, const FString& Text, const FLinearColor& Color, float Duration, ETDCHudSlot Slot)
{
	const double Now = NowSeconds();
	FTDCHudMessage& Msg = Messages.FindOrAdd(Key);
	if (Msg.Text != Text)
	{
		Msg.PostTime = Now; // solo un texto nuevo reinicia la animación de entrada
	}
	Msg.Text = Text;
	Msg.Color = Color;
	Msg.Slot = Slot;
	Msg.ExpireTime = Duration < 0.f ? -1.0 : Now + Duration;
}

void UTDCHudMessages::Clear(int32 Key)
{
	Messages.Remove(Key);
}

void UTDCHudMessages::Prune()
{
	const double Now = NowSeconds();
	for (auto It = Messages.CreateIterator(); It; ++It)
	{
		if (It->Value.ExpireTime >= 0.0 && It->Value.ExpireTime < Now)
		{
			It.RemoveCurrent();
		}
	}
}

void TDC::HudMessage(const UObject* WorldContext, int32 Key, const FString& Text, const FLinearColor& Color, float Duration,
	ETDCHudSlot Slot)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (UTDCHudMessages* Hud = World ? World->GetSubsystem<UTDCHudMessages>() : nullptr)
	{
		Hud->Post(Key, Text, Color, Duration, Slot);
	}
}

void TDC::HudClear(const UObject* WorldContext, int32 Key)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (UTDCHudMessages* Hud = World ? World->GetSubsystem<UTDCHudMessages>() : nullptr)
	{
		Hud->Clear(Key);
	}
}

// ---------------------------------------------------------------- HUD

namespace
{
	const FLinearColor Ink(0.92f, 0.92f, 0.9f);
	const FLinearColor Dim(0.6f, 0.6f, 0.62f);
	const FLinearColor Plate(0.f, 0.f, 0.f, 0.5f);
	const FLinearColor Yellow(1.f, 0.82f, 0.25f);
	const FLinearColor Violet(0.62f, 0.38f, 1.f);
	const FLinearColor Cyan(0.35f, 0.85f, 1.f);
}

UTexture2D* ATDCHUD::LoadUi(const TCHAR* Name) const
{
	const TCHAR* Folders[] = { TEXT("/Game/UI/HUD"), TEXT("/Game/UI") };
	for (const TCHAR* Folder : Folders)
	{
		const FString Path = FString::Printf(TEXT("%s/%s.%s"), Folder, Name, Name);
		if (UTexture2D* Tex = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path, nullptr,
			LOAD_NoWarn | LOAD_Quiet)))
		{
			return Tex;
		}
	}
	return nullptr;
}

void ATDCHUD::BeginPlay()
{
	Super::BeginPlay();
	IconLights = LoadUi(TEXT("T_UI_icon_luces"));
	IconRewind = LoadUi(TEXT("T_UI_icon_rebobinar"));
	IconClap = LoadUi(TEXT("T_UI_icon_claquetazo"));
	KeyFrame = LoadUi(TEXT("T_UI_key_frame"));
}

void ATDCHUD::DrawPlate(float X, float Y, float W, float H, const FLinearColor& Color)
{
	FCanvasTileItem Tile(FVector2D(X, Y), FVector2D(W, H), Color);
	Tile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tile);
}

FVector2D ATDCHUD::TextSize(const FString& Text, UFont* Font, float Scale)
{
	float W = 0.f, H = 0.f;
	Canvas->TextSize(Font, Text, W, H, Scale, Scale);
	return FVector2D(W, H);
}

void ATDCHUD::DrawShadowedText(const FString& Text, float X, float Y, UFont* Font, float Scale, const FLinearColor& Color)
{
	FCanvasTextItem Item(FVector2D(X, Y), FText::FromString(Text), Font, Color);
	Item.Scale = FVector2D(Scale, Scale);
	Item.EnableShadow(FLinearColor(0.f, 0.f, 0.f, 0.85f), FVector2D(2.f, 2.f) * Scale);
	Canvas->DrawItem(Item);
}

void ATDCHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !bShowHUD)
	{
		return;
	}
	UTDCHudMessages* Hud = GetWorld() ? GetWorld()->GetSubsystem<UTDCHudMessages>() : nullptr;
	if (!Hud)
	{
		return;
	}
	Hud->Prune();
	const float Scale = FMath::Max(0.5f, Canvas->ClipY / 1080.f);
	if (Hud->WantsControls())
	{
		DrawControls(*Hud, Scale);
	}
	DrawMessages(*Hud, Scale);
}

void ATDCHUD::DrawControls(UTDCHudMessages& Hud, float Scale)
{
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font)
	{
		return;
	}
	struct FRow { const TCHAR* Key; const TCHAR* Label; UTexture2D* Icon; FLinearColor Accent; };
	const FRow Rows[] = {
		{ TEXT("A/D"), TEXT("moverse"), nullptr, Ink },
		{ TEXT("Esp"), TEXT("saltar"), nullptr, Ink },
		{ TEXT("F"), TEXT("claquetazo"), IconClap, Ink },
		{ TEXT("E"), TEXT("prender / apagar reflector"), IconLights, Yellow },
		{ TEXT("Q"), TEXT("mantener: rebobinar lo celeste"), IconRewind, Cyan },
	};
	const float Pad = 14.f * Scale;
	const float RowH = 52.f * Scale;
	const float IconS = 44.f * Scale;
	const float KeyW = 64.f * Scale;
	const float TextScale = 1.45f * Scale; // a 720p el medium font quedaba ilegible (T-0049)
	const float X0 = 24.f * Scale;
	const float Y0 = 24.f * Scale;
	const float W = 600.f * Scale;
	const float H = Pad * 2.f + RowH * UE_ARRAY_COUNT(Rows) + 30.f * Scale;
	DrawPlate(X0, Y0, W, H, Plate);
	DrawPlate(X0, Y0, 4.f * Scale, H, Yellow * FLinearColor(1.f, 1.f, 1.f, 0.9f)); // filete de "claqueta"

	float Y = Y0 + Pad;
	for (const FRow& Row : Rows)
	{
		float X = X0 + Pad;
		// tecla
		if (KeyFrame)
		{
			DrawTextureSimple(KeyFrame, X, Y, KeyW / KeyFrame->GetSizeX());
		}
		else
		{
			DrawPlate(X, Y + 2.f * Scale, KeyW, RowH - 8.f * Scale, FLinearColor(1.f, 1.f, 1.f, 0.12f));
		}
		const FVector2D KS = TextSize(Row.Key, Font, TextScale * 0.85f);
		DrawShadowedText(Row.Key, X + (KeyW - KS.X) * 0.5f, Y + (RowH - KS.Y) * 0.5f - 2.f * Scale, Font, TextScale * 0.85f, Row.Accent);
		X += KeyW + 10.f * Scale;
		// ícono
		if (Row.Icon)
		{
			FCanvasTileItem IconItem(FVector2D(X, Y + (RowH - IconS) * 0.5f), Row.Icon->GetResource(), FVector2D(IconS, IconS),
				FLinearColor::White);
			IconItem.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(IconItem);
		}
		X += IconS + 10.f * Scale;
		const FVector2D LS = TextSize(Row.Label, Font, TextScale);
		DrawShadowedText(Row.Label, X, Y + (RowH - LS.Y) * 0.5f - 2.f * Scale, Font, TextScale, Ink);
		Y += RowH;
	}
	// leyenda de colores
	float X = X0 + Pad;
	Y += 6.f * Scale;
	const struct { const TCHAR* Label; FLinearColor Color; } Legend[] = {
		{ TEXT("con luz"), Yellow }, { TEXT("a oscuras"), Violet }, { TEXT("rebobinable"), Cyan } };
	for (const auto& L : Legend)
	{
		DrawPlate(X, Y + 6.f * Scale, 12.f * Scale, 12.f * Scale, L.Color);
		X += 18.f * Scale;
		const FVector2D S = TextSize(L.Label, Font, TextScale * 0.8f);
		DrawShadowedText(L.Label, X, Y, Font, TextScale * 0.8f, Dim);
		X += S.X + 16.f * Scale;
	}
}

void ATDCHUD::DrawMessages(UTDCHudMessages& Hud, float Scale)
{
	UFont* Medium = GEngine ? GEngine->GetMediumFont() : nullptr;
	UFont* Large = GEngine ? GEngine->GetLargeFont() : nullptr;
	if (!Medium || !Large)
	{
		return;
	}
	const double Now = Hud.NowSeconds();
	const float CX = Canvas->ClipX * 0.5f;
	float BottomY = Canvas->ClipY - 110.f * Scale; // los toasts se apilan hacia arriba
	TArray<int32> Keys;
	Hud.GetMessages().GetKeys(Keys);
	Keys.Sort(); // orden estable

	for (const int32 Key : Keys)
	{
		const FTDCHudMessage& Msg = Hud.GetMessages()[Key];
		const float Age = static_cast<float>(Now - Msg.PostTime);
		const float Left = Msg.ExpireTime < 0.0 ? 10.f : static_cast<float>(Msg.ExpireTime - Now);
		const float Alpha = FMath::Clamp(FMath::Min(Age / 0.12f, Left / 0.3f), 0.f, 1.f);
		if (Alpha <= 0.f)
		{
			continue;
		}
		FLinearColor Color = Msg.Color;
		Color.A = Alpha;

		if (Msg.Slot == ETDCHudSlot::Banner)
		{
			// Barras de cine cian + texto que late: el tiempo está corriendo hacia atrás.
			const float Pulse = 0.75f + 0.25f * FMath::Sin(static_cast<float>(Now) * 8.f);
			const float Bar = 10.f * Scale;
			DrawPlate(0.f, 0.f, Canvas->ClipX, Bar, Color * FLinearColor(1.f, 1.f, 1.f, 0.7f * Pulse));
			DrawPlate(0.f, Canvas->ClipY - Bar, Canvas->ClipX, Bar, Color * FLinearColor(1.f, 1.f, 1.f, 0.7f * Pulse));
			const float S = 1.6f * Scale;
			const FVector2D TS = TextSize(Msg.Text, Large, S);
			DrawShadowedText(Msg.Text, CX - TS.X * 0.5f, 40.f * Scale, Large, S, Color * FLinearColor(1.f, 1.f, 1.f, Pulse));
			continue;
		}

		const bool bPrompt = Msg.Slot == ETDCHudSlot::Prompt;
		const float S = (bPrompt ? 1.7f : 1.5f) * Scale;
		const FVector2D TS = TextSize(Msg.Text, Medium, S);
		const float PadX = 18.f * Scale;
		const float PadY = 8.f * Scale;
		const float Rise = (1.f - FMath::Clamp(Age / 0.12f, 0.f, 1.f)) * 12.f * Scale; // entra subiendo
		const float Y = BottomY - TS.Y - PadY * 2.f + Rise;
		DrawPlate(CX - TS.X * 0.5f - PadX, Y, TS.X + PadX * 2.f, TS.Y + PadY * 2.f, Plate * FLinearColor(1.f, 1.f, 1.f, Alpha));
		if (bPrompt)
		{
			DrawPlate(CX - TS.X * 0.5f - PadX, Y + TS.Y + PadY * 2.f - 3.f * Scale, TS.X + PadX * 2.f, 3.f * Scale, Color);
		}
		DrawShadowedText(Msg.Text, CX - TS.X * 0.5f, Y + PadY, Medium, S, Color);
		BottomY = Y - 10.f * Scale;
	}
}
