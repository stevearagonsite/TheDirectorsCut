---
id: T-0032
titulo: Playtest automático de los beats (-TDCPlaytest) y replay de la corrida
estado: enviada
tipo: test
rama: t/T-0032-playtest
creada: 2026-11-15
---

# T-0032 — Playtest automático

## Objetivo
Pedido de Steve: "un agente para el testing, así no lo hago yo a mano". Un bot recorre los beats (luces, rebobinado del puente,
claquetazo a los extras y salida) con aserciones `[TDC-PLAYTEST] PASS/FAIL` y capturas por paso. Sin package.
Guía: [`docs/pipeline/playtest.md`](../pipeline/playtest.md).

## Contexto
- C++ nuevo: `ATDCPlaytestRunner` (modos `-TDCPlaytest`, `-TDCRecord`, `-TDCReplay`). Módulo `Json` en el Build.cs.
- `tdc.ps1 playtest | record | replay -Run`: corren el editor en `-game`, resumen las líneas `[TDC-*]`, copian capturas a
  `docs/evidence/<Task>/<cmd>/` y salen con código ≠ 0 si no hay `RESULT PASS`.
- La rama trae main con mejora 5. Si T-0033 (mejora 6) ya se mergeó, da igual: el playtest no depende de las animaciones.

## Pasos
0. **Reenvío:** 6a9ed7f corrige `TDCPlaytestRunner.cpp:309` (sobraba un `*Check.Name`) y se auditaron los demás Printf/UE_LOG.
1. `git fetch` y `git checkout t/T-0032-playtest` (verificá el sha). `.\Tools\scripts\tdc.ps1 stop`.
2. `.\Tools\scripts\tdc.ps1 build`. Si no compila, **pará** y reportá `fallo` con `archivo:línea`.
3. `.\Tools\scripts\tdc.ps1 playtest -Task T-0032`. Pegá en el Reporte todas las líneas `[TDC-PLAYTEST]` (PASS/FAIL por paso y RESULT).
4. Si dio `RESULT PASS`: `.\Tools\scripts\tdc.ps1 replay -Task T-0032 -Run` (usa el `best.json` que dejó el bot) y pegá su resumen.
5. Si algún paso dio FAIL, describí qué muestra la captura de ese paso (por ejemplo, el supuesto de que el puente se dispara
   teletransportándose a (4560, 100) no se verificó).
6. Commit de `docs/evidence/T-0032/**`, `Tools/playtest/runs/best.json` (si existe) y el Reporte. Push y `[TDC-REPORT]`.

## Criterios de aceptación
- [ ] Compila.
- [ ] `tdc.ps1 playtest` termina con un `RESULT` (PASS o FAIL con pasos identificados) y capturas por paso.

## Permite
- Editar `Tools/scripts/tdc.ps1` solo para que `playtest`/`replay` funcionen.

## No hacer
- No cambiar C++, mapas ni assets. No empaquetar. No tocar zips.

## Evidencia pedida
Líneas `[TDC-PLAYTEST]`, capturas por paso en `docs/evidence/T-0032/playtest/` y, si aplica, el resumen del replay.

---

## Reporte (lo completa Windows)
- estado: fallo
- sha: 267877f (base probada; el sha del reporte va en el `[TDC-REPORT]`)
- resumen: `tdc.ps1 build` no compila (1 error, en `TDCPlaytestRunner.cpp:309`). Según el paso 2 paré ahí: no se corrió
  `playtest` ni `replay`, no hay capturas ni `best.json`.
- errores:
  ```
  [1/4] Compile [x64] Module.TheDirectorsCut.cpp
  Game\Source\TheDirectorsCut\Private\Debug\TDCPlaytestRunner.cpp(309,21): error C7595:
    'UE::Core::Private::FormatStringSan::TCheckedFormatStringPrivate<FString::FmtCharType,const FString::ElementType *,
    const FString::ElementType *,const FString::ElementType *,const FString::ElementType *,const FString::ElementType *,
    int,int,int>::TCheckedFormatStringPrivate': call to immediate function is not a constant expression
      FString::Printf(TEXT("%s (%s) %s=%s: presente=%d (esperado %d), colision=%d"), *When, *Check.Name,
  FormatStringSanErrors.inl(41,1): note: X(DNeedsIntegerArg, "'%d' expects integral arg (eg. `char`, `int`, `long`, etc.)")
  Result: Failed (OtherCompilationError)
  ```
- evidencia: no hay (no llegó a correr el juego). Log completo local: `Game\Saved\Logs\tdc-build.log` (no versionado).
- notas / preguntas:
  - Causa: en `TDCPlaytestRunner.cpp:309-310` el formato tiene 4 `%s` + 3 `%d` (7) y se le pasan 8 argumentos:
    `*When, *Check.Name, *Check.Name, on/off, *P.Name, int, int, int`. `*Check.Name` está dos veces, así que `*P.Name`
    cae en el primer `%d`. Arreglo sugerido: sacar el `*Check.Name` repetido (o sumar un `%s` para la pieza).
  - Es el único error que mostró el compilador (unity build, cortó en el primero); puede haber otros detrás.
  - No toqué C++ (`No hacer`). `tdc.ps1 stop` no encontró editor del proyecto abierto; el build empaquetado de
    `Game\Saved\PackageTest` no se tocó.
  - Toolchain: MSVC 14.52.37029 (UBT avisa que no es la versión preferida, 14.50.35717); UE 5.8.
