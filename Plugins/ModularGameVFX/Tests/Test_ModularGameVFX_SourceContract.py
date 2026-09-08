from __future__ import annotations

import json
import pathlib
import re
import unittest


PLUGIN_ROOT = pathlib.Path(__file__).resolve().parents[1]
MODULE_ROOT = PLUGIN_ROOT / "Source" / "ModularGameVFX"


class ModularGameVFXSourceContractTests(unittest.TestCase):
    def test_runtime_module_and_required_files_exist(self) -> None:
        required = [
            PLUGIN_ROOT / "ModularGameVFX.uplugin",
            MODULE_ROOT / "ModularGameVFX.Build.cs",
            MODULE_ROOT / "Public" / "VFXTypes.h",
            MODULE_ROOT / "Public" / "VFXCatalog.h",
            MODULE_ROOT / "Public" / "ModularGameVFXSubsystem.h",
            MODULE_ROOT / "Public" / "ModularGameVFXBlueprintLibrary.h",
            MODULE_ROOT / "Public" / "ModularGameVFXSettings.h",
            MODULE_ROOT / "Private" / "ModularGameVFX.cpp",
            MODULE_ROOT / "Private" / "VFXCatalog.cpp",
            MODULE_ROOT / "Private" / "ModularGameVFXSubsystem.cpp",
            MODULE_ROOT / "Private" / "ModularGameVFXBlueprintLibrary.cpp",
            MODULE_ROOT / "Private" / "ModularGameVFXSettings.cpp",
        ]
        missing = [str(path.relative_to(PLUGIN_ROOT)) for path in required if not path.is_file()]
        self.assertEqual([], missing, f"Missing ModularGameVFX runtime files: {missing}")

        descriptor = json.loads((PLUGIN_ROOT / "ModularGameVFX.uplugin").read_text(encoding="utf-8-sig"))
        self.assertTrue(descriptor.get("CanContainContent"))
        self.assertIn(
            {"Name": "ModularGameVFX", "Type": "Runtime", "LoadingPhase": "Default"},
            descriptor.get("Modules", []),
        )

    def test_public_types_express_soft_catalog_handle_and_unified_request(self) -> None:
        source = (MODULE_ROOT / "Public" / "VFXTypes.h").read_text(encoding="utf-8")
        for token in [
            "EVFXSpawnMode",
            "EVFXPriority",
            "FVFXParameterOverrides",
            "FVFXCatalogEntry",
            "FVFXPlayRequest",
            "FVFXHandle",
            "TSoftObjectPtr<UNiagaraSystem>",
            "ENCPoolMethod",
            "FOnModularVFXReady",
        ]:
            self.assertIn(token, source)
        self.assertNotRegex(source, r"UPROPERTY[^;]*UNiagaraSystem\s*\*")

    def test_blueprint_library_is_the_only_public_gameplay_entry(self) -> None:
        header = (MODULE_ROOT / "Public" / "ModularGameVFXBlueprintLibrary.h").read_text(encoding="utf-8")
        for api in [
            "PlayVFX",
            "PlayVFXAtLocation",
            "PlayVFXAttached",
            "StopVFX",
            "StopVFXImmediate",
            "StopAllVFXForOwner",
            "StopVFXByTag",
            "SetVFXFloatParameter",
            "SetVFXVectorParameter",
            "SetVFXColorParameter",
            "SetVFXParameters",
            "PreloadVFX",
            "PreloadVFXTags",
            "ReleasePreloadedVFX",
            "IsVFXHandleValid",
            "FindVFXEntry",
        ]:
            self.assertIn(api, header)

        library_cpp = (MODULE_ROOT / "Private" / "ModularGameVFXBlueprintLibrary.cpp").read_text(encoding="utf-8")
        self.assertNotIn("UNiagaraFunctionLibrary", library_cpp)
        self.assertNotIn("RequestAsyncLoad", library_cpp)
        self.assertNotIn("FindEntryNative", library_cpp)

    def test_niagara_spawn_has_one_subsystem_owned_choke_point(self) -> None:
        source_files = list((PLUGIN_ROOT / "Source").rglob("*.cpp"))
        direct_spawn_files: list[pathlib.Path] = []
        for path in source_files:
            source = path.read_text(encoding="utf-8-sig")
            if "SpawnSystemAtLocation" in source or "SpawnSystemAttached" in source:
                direct_spawn_files.append(path)

        expected = MODULE_ROOT / "Private" / "ModularGameVFXSubsystem.cpp"
        self.assertEqual([expected], direct_spawn_files)
        subsystem_source = expected.read_text(encoding="utf-8")
        definitions = re.findall(
            r"UNiagaraComponent\s*\*\s*UModularGameVFXSubsystem::SpawnResolvedVFX\s*\(",
            subsystem_source,
        )
        self.assertEqual(1, len(definitions))
        self.assertIn("PendingLoads", subsystem_source)
        self.assertIn("TWeakObjectPtr", subsystem_source)
        self.assertIn("NM_DedicatedServer", subsystem_source)

    def test_framework_source_has_no_project_specific_logic(self) -> None:
        forbidden = re.compile(r"DivineBeasts|Tiger|Dragon|神兽联盟", re.IGNORECASE)
        violations: list[str] = []
        for path in MODULE_ROOT.rglob("*"):
            if path.suffix.lower() not in {".h", ".cpp", ".cs"}:
                continue
            if forbidden.search(path.read_text(encoding="utf-8-sig")):
                violations.append(str(path.relative_to(PLUGIN_ROOT)))
        self.assertEqual([], violations)


if __name__ == "__main__":
    unittest.main(verbosity=2)
