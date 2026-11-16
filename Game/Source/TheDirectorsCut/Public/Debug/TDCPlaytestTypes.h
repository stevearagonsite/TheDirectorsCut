#pragma once

#include "CoreMinimal.h"

/** Acciones de juego que se graban y se reproducen (playtest automático, -TDCRecord / -TDCReplay / -TDCPlaytest). */
enum class ETDCPlayAction : uint8
{
	Move,     // eje horizontal sostenido (-1..1)
	Jump,     // 1 = apretar, 0 = soltar
	Lights,   // E: prende/apaga el reflector más cercano
	Rewind,   // Q: 1 = empezar a rebobinar, 0 = soltar
	Clap,     // F: claquetazo (Value 1 = se ejecutó, 0 = en cooldown)
	Teleport, // solo el bot guionado: salto de beat (Value = hacia dónde mira, +1/-1)
};

enum class ETDCPlaytestMode : uint8
{
	None,
	Record,   // -TDCRecord: graba lo que juega Steve
	Replay,   // -TDCReplay[=archivo]: reproduce una corrida grabada y verifica
	Scripted, // -TDCPlaytest: bot guionado por beats (genera best.json si no hay uno)
};
