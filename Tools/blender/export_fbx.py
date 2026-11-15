"""
Exporta el .blend abierto a FBX listo para Unreal.

  blender.exe -b Art/Source/<Asset>.blend --python Tools/blender/export_fbx.py -- --out Art/Export/<Asset>.fbx [--collection Export]

- Exporta la colección indicada (por defecto "Export"); si no existe, todos los meshes visibles.
- Aplica transforms, escala métrica -> centímetros de Unreal, eje -Y adelante / Z arriba.
"""

import argparse
import sys
from pathlib import Path

import bpy


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", required=True)
    parser.add_argument("--collection", default="Export")
    return parser.parse_args(argv)


def select_targets(collection_name):
    bpy.ops.object.select_all(action="DESELECT")
    collection = bpy.data.collections.get(collection_name)
    objects = collection.all_objects if collection else [o for o in bpy.context.scene.objects if o.visible_get()]
    targets = [o for o in objects if o.type in {"MESH", "ARMATURE", "EMPTY"}]
    for obj in targets:
        obj.select_set(True)
    if not targets:
        raise SystemExit(f"[TDC] Nada para exportar (colección '{collection_name}' vacía o sin meshes)")
    bpy.context.view_layer.objects.active = targets[0]
    return targets


def main():
    args = parse_args()
    out = Path(args.out).resolve()
    out.parent.mkdir(parents=True, exist_ok=True)

    targets = select_targets(args.collection)
    meshes = [o for o in targets if o.type == "MESH"]
    if meshes:
        with bpy.context.temp_override(selected_editable_objects=meshes, selected_objects=meshes, active_object=meshes[0]):
            bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)

    bpy.ops.export_scene.fbx(
        filepath=str(out),
        use_selection=True,
        apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_UNITS",
        axis_forward="-Y",
        axis_up="Z",
        mesh_smooth_type="FACE",
        use_tspace=True,
        add_leaf_bones=False,
        bake_anim=False,
    )
    print(f"[TDC] Exportado {len(targets)} objeto(s) -> {out}")


main()
