import json
import unittest
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path


PLUGIN_ROOT = Path(__file__).resolve().parents[1]
DOCUMENTATION_ROOT = PLUGIN_ROOT / "Documentation"
CATALOG_BASE = DOCUMENTATION_ROOT / "ModularGameVFX_1.0.0_VFX_Catalog"


class ModularGameVFXCatalogDocumentTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = json.loads(Path(f"{CATALOG_BASE}.json").read_text(encoding="utf-8"))

    def test_canonical_outputs_exist(self):
        for suffix in (".json", ".csv", ".md", ".xlsx", ".xlsx.inspect.ndjson"):
            self.assertTrue(Path(f"{CATALOG_BASE}{suffix}").is_file(), suffix)
        self.assertTrue((DOCUMENTATION_ROOT / "ModularGameVFX_1.0.0_Asset_Migration_Catalog.csv").is_file())
        with zipfile.ZipFile(Path(f"{CATALOG_BASE}.xlsx")) as archive:
            workbook = ET.fromstring(archive.read("xl/workbook.xml"))
            ns = {"x": "http://schemas.openxmlformats.org/spreadsheetml/2006/main"}
            self.assertEqual(
                ["分类规范", "Core原子", "Metadata", "演示法术", "生产Niagara", "既有库迁移", "资产概览", "资产总表"],
                [sheet.attrib["name"] for sheet in workbook.findall("x:sheets/x:sheet", ns)],
            )

    def test_functional_taxonomy_is_complete_and_ordered(self):
        expected = ["00_Core"] + [
            "01_Cast", "02_Projectile", "03_Trail", "04_Beam", "05_Impact",
            "06_Area", "07_Status", "08_Space", "09_Lifecycle", "10_Character",
            "11_World", "12_Surface", "13_Destruction", "14_Indicator", "15_UI",
        ]
        self.assertEqual(expected, [row["一级目录"] for row in self.data["taxonomy"]])

    def test_core_primitive_taxonomy_is_complete(self):
        expected = {
            "Energy", "Fire", "Smoke", "Sparks", "Dust", "Lightning", "Glow",
            "Shockwave", "Debris", "Noise", "Ribbon", "Meshes", "Textures",
            "Materials", "MaterialFunctions", "NiagaraModules", "Curves",
            "EffectTypes", "Catalog",
        }
        self.assertEqual(expected, {row["Core 子目录"] for row in self.data["corePrimitives"]})
        management = {row["Core 子目录"] for row in self.data["corePrimitives"] if row["目录角色"] == "管理目录"}
        self.assertEqual({"EffectTypes", "Catalog"}, management)

    def test_catalog_records_have_precise_ids_and_metadata(self):
        rows = self.data["abilities"] + self.data["components"] + self.data["legacyLibrary"]
        required = {
            "VFX Catalog", "VFX ID", "Legacy VFX ID", "效果名称", "效果描述",
            "GameplayTag", "Element", "Form", "Context", "Lifecycle", "Priority",
            "Effect Type",
        }
        primary_ids = []
        for row in rows:
            self.assertTrue(required.issubset(row), row.get("VFX ID"))
            self.assertTrue(row["VFX ID"].startswith("MGVFX-"), row["VFX ID"])
            self.assertTrue(row["Legacy VFX ID"].startswith("MVFX-"), row["Legacy VFX ID"])
            self.assertTrue(row["效果名称"].strip(), row["VFX ID"])
            self.assertTrue(row["Effect Type"].startswith("ET_VFX_"), row["Effect Type"])
            self.assertFalse(row["Effect Type"].startswith("NET_MGVFX_"), row["Effect Type"])
            primary_ids.append(row["VFX ID"])
        self.assertEqual(len(primary_ids), len(set(primary_ids)))

    def test_asset_paths_use_modular_mount_and_declared_library_root(self):
        for row in self.data["components"] + self.data["legacyLibrary"]:
            self.assertNotIn("当前兼容路径", row)
            self.assertTrue(row["历史来源路径"].startswith("/ModularGameVFX/"))
            self.assertTrue(row["标准目标路径"].startswith(self.data["libraryRoot"] + "/"))
            self.assertNotIn("/MythicVFX/", row["历史来源路径"])
            self.assertNotIn("/MythicVFX/", row["标准目标路径"])

    def test_authoritative_manifest_inventory_is_complete_and_truthful(self):
        assets = self.data["assetInventory"]
        self.assertEqual(1059, len(assets))
        self.assertEqual(1059, len({row["历史来源路径"] for row in assets}))
        self.assertEqual(1059, len({row["标准目标路径"] for row in assets}))
        self.assertEqual(36, sum(row["资产类型"] == "NiagaraSystem" for row in assets))
        self.assertEqual(13, sum(row["正式效果"] == "是" for row in assets))
        marker = PLUGIN_ROOT.parents[1] / "Saved" / "TaxonomyMigration" / "migration_complete.json"
        self.assertEqual(marker.is_file(), self.data["migrationComplete"])
        self.assertEqual("1.3.0", self.data["schemaVersion"])
        self.assertEqual({"Review": 13, "Prototype": 23, "ProductionReady": 0, "visualReview": "NotTested", "gpuProfile": "NotRun"}, self.data["qualityState"])
        self.assertEqual(13, len(self.data["components"]))
        self.assertEqual(23, len(self.data["legacyLibrary"]))
        self.assertTrue(all("建议策略，未绑定" in row["Effect Type"] for row in self.data["legacyLibrary"]))
        self.assertTrue(all(row["Catalog 状态"] == "Prototype_RequiresValidation" for row in self.data["legacyLibrary"]))
        self.assertTrue(all("当前兼容路径" not in row for row in assets))
        if marker.is_file():
            self.assertTrue(all(row["Catalog 状态"] == "Registered_Review" for row in self.data["components"]))
            self.assertTrue(all("旧路径已移除" in row["资产迁移状态"] for row in assets))
        if not marker.is_file():
            self.assertTrue(all("未迁移" in row["资产迁移状态"] for row in assets))


if __name__ == "__main__":
    unittest.main()
