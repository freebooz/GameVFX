"""Run inside Unreal Editor Python after rebuilding ModularGameVFX.

These tests create transient catalogs only; they do not load or save VFX assets.
The round-trip guards serialized metadata and the existing priority property.
Validation cases guard migration compatibility and category-first invocation tags.
"""
from __future__ import annotations

import unittest

import unreal


class ModularGameVFXTaxonomyTests(unittest.TestCase):
    def setUp(self):
        for name in ("VFXCategory", "VFXElement", "VFXForm", "VFXContext", "VFXLifecycle"):
            self.assertTrue(hasattr(unreal, name), f"Missing reflected taxonomy enum: unreal.{name}")

    def test_legacy_entry_defaults_preserve_priority_and_defer_category_validation(self):
        entry = unreal.VFXCatalogEntry()
        self.assertEqual(entry.get_editor_property("category"), unreal.VFXCategory.UNSPECIFIED)
        self.assertEqual(entry.get_editor_property("element"), unreal.VFXElement.NONE)
        self.assertEqual(entry.get_editor_property("form"), unreal.VFXForm.UNSPECIFIED)
        self.assertEqual(entry.get_editor_property("context"), unreal.VFXContext.COMBAT)
        self.assertEqual(entry.get_editor_property("lifecycle"), unreal.VFXLifecycle.ONE_SHOT)
        self.assertEqual(entry.get_editor_property("performance_class"), unreal.VFXPriority.COMBAT)

    def test_catalog_metadata_and_existing_priority_survive_serialized_round_trip(self):
        entry = unreal.VFXCatalogEntry()
        values = {
            "category": unreal.VFXCategory.SPACE,
            "element": unreal.VFXElement.DARK,
            "form": unreal.VFXForm.FIELD,
            "context": unreal.VFXContext.COMBAT,
            "lifecycle": unreal.VFXLifecycle.PERSISTENT,
            "performance_class": unreal.VFXPriority.CRITICAL,
        }
        for key, value in values.items():
            entry.set_editor_property(key, value)
        restored = unreal.VFXCatalogEntry()
        self.assertTrue(restored.import_text(entry.export_text()))
        for key, value in values.items():
            self.assertEqual(restored.get_editor_property(key), value, key)

    def _validate(self, category, tag_text):
        entry = unreal.VFXCatalogEntry()
        # A non-null soft path exercises metadata validation without loading a system.
        self.assertTrue(entry.import_text(
            '(NiagaraSystem="/ModularGameVFX/Tests/NS_TransientReference.NS_TransientReference")'
        ))
        tag = unreal.GameplayTag()
        self.assertTrue(tag.import_text(f'(TagName="{tag_text}")'))
        self.assertIn(tag_text, tag.export_text(), "Fixture tag must be registered")
        entry.set_editor_property("vfx_tag", tag)
        entry.set_editor_property("category", category)
        catalog = unreal.new_object(unreal.VFXCatalog)
        catalog.set_editor_property("entries", [entry])
        validator = unreal.get_editor_subsystem(unreal.EditorValidatorSubsystem)
        return validator.is_object_valid(catalog, unreal.DataValidationUsecase.SCRIPT)

    def test_legacy_unspecified_category_accepts_old_invocation_tag(self):
        result, errors, _warnings = self._validate(unreal.VFXCategory.UNSPECIFIED, "VFX.Common.Hit")
        self.assertEqual(result, unreal.DataValidationResult.VALID, str(errors))

    def test_populated_category_rejects_legacy_non_category_tag(self):
        result, errors, _warnings = self._validate(unreal.VFXCategory.IMPACT, "VFX.Common.Hit")
        self.assertEqual(result, unreal.DataValidationResult.INVALID)
        self.assertTrue(any("category" in str(error).lower() for error in errors), str(errors))

    def test_project_character_namespace_accepts_other_primary_category(self):
        result, errors, _warnings = self._validate(
            unreal.VFXCategory.IMPACT, "VFX.Character.Tiger.Skill01.Hit")
        self.assertEqual(result, unreal.DataValidationResult.VALID, str(errors))


if __name__ == "__main__":
    result = unittest.TextTestRunner(verbosity=2).run(
        unittest.defaultTestLoader.loadTestsFromTestCase(ModularGameVFXTaxonomyTests))
    if not result.wasSuccessful():
        raise RuntimeError("ModularGameVFX taxonomy validation failed")
