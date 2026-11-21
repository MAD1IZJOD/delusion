"""Run in Unreal Editor with DELUSION_ASSET_STEP=material or map.

Each invocation creates exactly one content file. Commit and push that file
before invoking the other step. Existing assets are never overwritten.
"""

import os
from pathlib import Path

import unreal


ROOT = Path(r"D:\personal projects\delusion")
if Path(unreal.Paths.project_dir()).resolve() != ROOT.resolve():
    raise RuntimeError("This script must run in the existing DELUSION project.")

step = os.environ.get("DELUSION_ASSET_STEP")
surface_path = "/Game/Materials/M_PrototypeSurface"
map_path = "/Game/Maps/Prototype"

if step == "material":
    if unreal.EditorAssetLibrary.does_asset_exist(surface_path):
        raise RuntimeError("The prototype material already exists; refusing to overwrite it.")
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_PrototypeSurface", "/Game/Materials", unreal.Material, unreal.MaterialFactoryNew()
    )
    if not material:
        raise RuntimeError("Unreal could not create the prototype material.")
    color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, -400, 0
    )
    color.set_editor_property("parameter_name", "Color")
    color.set_editor_property("default_value", unreal.LinearColor(0.4, 0.4, 0.4, 1.0))
    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 180
    )
    roughness.set_editor_property("r", 0.85)
    if not unreal.MaterialEditingLibrary.connect_material_property(
        color, "", unreal.MaterialProperty.MP_BASE_COLOR
    ):
        raise RuntimeError("Could not connect the surface color.")
    if not unreal.MaterialEditingLibrary.connect_material_property(
        roughness, "", unreal.MaterialProperty.MP_ROUGHNESS
    ):
        raise RuntimeError("Could not connect surface roughness.")
    unreal.MaterialEditingLibrary.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material):
        raise RuntimeError("Could not save the prototype material.")
    unreal.log("DELUSION_ASSET_CREATED: Content/Materials/M_PrototypeSurface.uasset")

elif step == "map":
    if unreal.EditorAssetLibrary.does_asset_exist(map_path):
        raise RuntimeError("The prototype map already exists; refusing to overwrite it.")
    surface = unreal.load_asset(surface_path)
    if not surface:
        raise RuntimeError("Create, commit, and push the prototype material first.")
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.new_level(map_path, False):
        raise RuntimeError("Could not create the non-partitioned prototype map.")
    district_class = unreal.load_class(None, "/Script/Delusion.DelusionNeighborhood")
    terminal_class = unreal.load_class(None, "/Script/Delusion.DelusionTestTerminal")
    if not district_class or not terminal_class:
        raise RuntimeError("Build the DELUSION editor module before creating the map.")
    district = actors.spawn_actor_from_class(district_class, unreal.Vector(0, 0, 0))
    district.set_actor_label("First District")
    district.set_editor_property("surface_material", surface)
    spawn = actors.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(-1800, -300, 105), unreal.Rotator(-10, 0, 0)
    )
    spawn.set_actor_label("DELUSION Spawn")
    terminal = actors.spawn_actor_from_class(terminal_class, unreal.Vector(-1300, -450, 90))
    terminal.set_actor_label("Interaction Terminal")
    if not levels.save_current_level():
        raise RuntimeError("Could not save the playable prototype map.")
    unreal.log("DELUSION_ASSET_CREATED: Content/Maps/Prototype.umap")

else:
    raise RuntimeError("Set DELUSION_ASSET_STEP to material or map; run one step at a time.")
