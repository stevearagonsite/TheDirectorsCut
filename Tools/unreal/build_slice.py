"""
Construye (o reconstruye) el mapa greybox del vertical slice: /Game/Maps/L_Slice.

Correr en el editor de Unreal (Windows):
  - Output Log > Python:  py "<repo>/Tools/unreal/build_slice.py"
  - o por línea de comandos:  UnrealEditor.exe "<repo>/Game/TheDirectorsCut.uproject" -ExecutePythonScript="<repo>/Tools/unreal/build_slice.py"

Es idempotente: si el mapa existe, borra sus actores y lo vuelve a armar. El layout se edita ACÁ (lo mantiene la Mac);
los retoques visuales finos se hacen en el editor sobre otro mapa o en una tarea aparte.

Plano de juego: X = horizontal (derecha +), Z = vertical. Y = profundidad (cámara en +Y mirando a -Y).
Unidades: cm. El personaje salta ~190 cm de alto y ~600 cm de largo; huecos de 1200 son insalvables.

Beats:
  1. LUCES     - Pozo 1: plataforma amarilla (solo con luz). El reflector L1 empieza apagado -> prenderlo.
  2. SOMBRA    - Pared amarilla bloquea; más adelante una plataforma violeta (solo a oscuras). Un solo reflector L2
                 controla ambas -> apagarlo abre la pared Y hace aparecer la plataforma.
  3. TIEMPO    - Puente de tablas celestes (en toma) que se desploma al acercarse -> mantener Q para rebobinarlo.
  4. COMBO     - Salida alta. Caja celeste sobre un estante amarillo; escalón violeta. Apagar L3 tira la caja y
                 hace aparecer el escalón: caja -> escalón -> salida. Si la caja cae mal, rebobinar.

Extras (claquetazo, F): rojo = común (lo derriba), naranja = resistente (lo aturde y empuja),
negro con franja = de guion (inmune: en Beat 2 es una sombra que solo existe a oscuras -> prender L2 la saca de la toma).
"""

import unreal

MAP_PATH = "/Game/Maps/L_Slice"

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

SET_PIECE = unreal.load_class(None, "/Script/TheDirectorsCut.TDCSetPiece")
STAGE_LIGHT = unreal.load_class(None, "/Script/TheDirectorsCut.TDCStageLight")
FALL_TRIGGER = unreal.load_class(None, "/Script/TheDirectorsCut.TDCFallTrigger")
RESPAWN = unreal.load_class(None, "/Script/TheDirectorsCut.TDCRespawnVolume")
EXTRA = unreal.load_class(None, "/Script/TheDirectorsCut.TDCExtra")

ALWAYS = unreal.TDCLightPresence.ALWAYS
IN_LIGHT = unreal.TDCLightPresence.ONLY_IN_LIGHT
IN_DARK = unreal.TDCLightPresence.ONLY_IN_DARK

DEPTH = 400.0  # profundidad (Y) de pisos y plataformas


def rot(pitch=0.0, yaw=0.0, roll=0.0):
    r = unreal.Rotator()
    r.pitch, r.yaw, r.roll = pitch, yaw, roll
    return r


def spawn(cls, label, loc, folder, rotation=None):
    actor = eas.spawn_actor_from_class(cls, unreal.Vector(*loc), rotation or rot())
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    return actor


def box(label, x0, x1, z0, z1, folder, presence=ALWAYS, depth=DEPTH, record=False, simulate=False, safe=False, color=None):
    """Pieza de set como caja por extremos (cm). El cubo base mide 100."""
    center = ((x0 + x1) / 2.0, 0.0, (z0 + z1) / 2.0)
    piece = spawn(SET_PIECE, label, center, folder)
    piece.set_actor_scale3d(unreal.Vector((x1 - x0) / 100.0, depth / 100.0, (z1 - z0) / 100.0))
    piece.set_editor_property("presence", presence)
    piece.set_editor_property("record_time", record)
    piece.set_editor_property("starts_simulating", simulate)
    if color:
        piece.set_editor_property("color_override", unreal.LinearColor(*color))
    if safe:
        piece.set_editor_property("tags", [unreal.Name("TDC.Safe")])
    return piece


def stage_light(label, loc, radius, on, folder):
    light = spawn(STAGE_LIGHT, label, loc, folder)
    light.set_editor_property("radius", radius)
    light.set_editor_property("starts_on", on)
    return light


def extra(label, x, folder, vulnerability, patrol=200.0, presence=ALWAYS):
    """Extra fuera de guion parado sobre el piso (z=0); patrulla en X."""
    actor = spawn(EXTRA, label, (x, 0.0, 90.0), folder)
    actor.set_editor_property("vulnerability", vulnerability)
    actor.set_editor_property("presence", presence)
    actor.set_editor_property("patrol_distance", patrol)
    return actor


def floor(label, x0, x1, folder):
    return box(label, x0, x1, -100.0, 0.0, folder, safe=True)


def open_clean_map():
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        les.load_level(MAP_PATH)
        actors = [a for a in eas.get_all_level_actors() if not isinstance(a, unreal.WorldSettings)]
        if actors:
            eas.destroy_actors(actors)
    else:
        les.new_level(MAP_PATH)


def build():
    open_clean_map()

    # --- Ambiente: luz tenue para que los reflectores se lean ---
    sun = spawn(unreal.DirectionalLight, "Ambient_Sun", (0.0, 0.0, 2000.0), "Lighting", rot(pitch=-55.0, yaw=-70.0))
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_intensity(1.5)
    sun_comp.set_editor_property("atmosphere_sun_light", True)
    # Cielo y luz ambiente: sin esto el fondo queda negro. Tono de cine negro: cielo apagado + niebla baja.
    spawn(unreal.SkyAtmosphere, "Sky_Atmosphere", (0.0, 0.0, 0.0), "Lighting")
    sky = spawn(unreal.SkyLight, "Sky_Light", (0.0, 0.0, 1500.0), "Lighting")
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("real_time_capture", True)
    sky_comp.set_intensity(0.6)
    fog = spawn(unreal.ExponentialHeightFog, "Height_Fog", (0.0, 0.0, -200.0), "Lighting")
    fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_comp.set_editor_property("fog_density", 0.02)

    spawn(unreal.PlayerStart, "PlayerStart", (200.0, 0.0, 120.0), "Gameplay")

    # Fondo del pozo + respawn (una sola franja bajo los tres pozos)
    box("Pit_Floor", 1000.0, 6000.0, -900.0, -800.0, "Gameplay", depth=600.0)
    respawn = spawn(RESPAWN, "Pit_Respawn", (3500.0, 0.0, -700.0), "Gameplay")
    respawn.set_actor_scale3d(unreal.Vector(5.0, 1.0, 1.0))  # extent base 500 -> 2500 en X

    # --- Beat 1: LUCES ---
    f = "Beat1_Lights"
    floor("Floor_A", 0.0, 1000.0, f)
    box("P1_LightPlatform", 1450.0, 1750.0, -50.0, 0.0, f, presence=IN_LIGHT)
    stage_light("L1", (1600.0, -150.0, 350.0), 500.0, False, f)
    extra("X1_Common", 650.0, f, unreal.TDCVulnerability.COMMON, patrol=180.0)

    # --- Beat 2: SOMBRA ---
    f = "Beat2_Shadow"
    floor("Floor_B", 2200.0, 3000.0, f)
    box("W1_LightWall", 2800.0, 2860.0, 0.0, 400.0, f, presence=IN_LIGHT)
    stage_light("L2", (3050.0, -150.0, 350.0), 600.0, True, f)
    box("S1_DarkPlatform", 3350.0, 3650.0, -50.0, 0.0, f, presence=IN_DARK)
    # Sombra de guion: L2 la tapa al inicio; al apagar L2 para abrir la pared aparece. Prender L2 otra vez la elimina.
    extra("X2_ScriptedShadow", 2650.0, f, unreal.TDCVulnerability.SCRIPTED, patrol=80.0, presence=IN_DARK)  # siempre dentro del radio de L2

    # --- Beat 3: TIEMPO ---
    f = "Beat3_Time"
    floor("Floor_C", 4000.0, 4800.0, f)
    planks = []
    for i in range(4):
        x0 = 4800.0 + i * 300.0
        planks.append(box(f"Bridge_Plank_{i + 1}", x0 + 2.0, x0 + 298.0, -30.0, 0.0, f, depth=300.0, record=True))
    trigger = spawn(FALL_TRIGGER, "Bridge_FallTrigger", (4550.0, 0.0, 150.0), f)
    trigger.set_editor_property("targets", planks)

    # --- Beat 4: COMBO luces + tiempo ---
    f = "Beat4_Combo"
    floor("Floor_D", 6000.0, 7600.0, f)
    box("SH_LightShelf", 6500.0, 6600.0, 250.0, 300.0, f, presence=IN_LIGHT)
    box("C1_Crate", 6505.0, 6595.0, 302.0, 392.0, f, depth=90.0, record=True, simulate=True)
    box("P3_DarkStep", 6700.0, 7000.0, 200.0, 240.0, f, presence=IN_DARK)
    extra("X3_Resistant", 6250.0, f, unreal.TDCVulnerability.RESISTANT, patrol=150.0)
    stage_light("L3", (6750.0, -150.0, 550.0), 450.0, True, f)
    box("Exit_Ledge", 7200.0, 7600.0, 0.0, 380.0, f, safe=True)
    box("Exit_Marker", 7450.0, 7520.0, 380.0, 520.0, f, depth=70.0, color=(0.2, 0.9, 0.3, 1.0))

    world = unreal.EditorLevelLibrary.get_editor_world()
    settings = world.get_world_settings()
    settings.set_editor_property("kill_z", -1500.0)

    les.save_current_level()
    unreal.log("[TDC] L_Slice construido y guardado.")


build()

# Lanzado con -ExecutePythonScript (tdc.ps1 python -InEditor): cerrar el editor al terminar.
if "-executepythonscript" in unreal.SystemLibrary.get_command_line().lower():
    unreal.SystemLibrary.quit_editor()
