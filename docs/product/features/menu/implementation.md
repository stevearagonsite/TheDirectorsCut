# Menú inicial y pausa: implementation

**Updated:** 2026-11-15 · [Design](design.md) · [Testing](testing.md)

- `ATDCPlayerController` (`Core/TDCPlayerController`), fijado en `ATDCGameMode::PlayerControllerClass`.
  - En `BeginPlay`, si no está desactivado, muestra el menú y pausa el mundo con `UGameplayStatics::SetGamePaused`.
  - Lo desactivan los flags de línea de comandos o la opción de URL `?TDCNoMenu`, que es la que usa Reiniciar.
  - Esc / Start están enlazados con `BindKey` (`bExecuteWhenPaused`) y solo abren la pausa desde el juego.
  - "¡Acción!" (`vo_accion`) suena al empezar la toma; antes sonaba en el `BeginPlay` del personaje.
- `STDCMenuWidget` (`UI/TDCMenuWidget`): Slate puro con un `SWidgetSwitcher` de cuatro pantallas (principal, controles,
  carga y pausa). Se agrega al viewport con `AddViewportWidgetContent` y se saca en `EndPlay`, porque el contenido del
  viewport sobrevive al cambio de nivel.
- Texturas: `Tools/unreal/import_ui.py` importa `Art/Export/UI/` a `/Game/UI/T_UI_*`, en formato UserInterface2D, sin
  mips y en grupo UI. `/Game/UI` está en `DirectoriesToAlwaysCook`. El controlador las guarda en una `UPROPERTY` para
  que el GC no se las lleve mientras Slate las dibuja.
- Ícono del exe: `Game/Build/Windows/Application.ico`, copia de `Art/Export/UI/TheDirectorsCut.ico`.
- Módulos: `Slate` y `SlateCore` (privados).
