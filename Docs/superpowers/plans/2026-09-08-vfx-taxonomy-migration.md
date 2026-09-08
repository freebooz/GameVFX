# VFX Taxonomy Migration Implementation Plan

**Goal:** Implement the user's supplied taxonomy in the existing ModularGameVFX plugin and migrate actual assets. Latest user ruling: remove old paths after updating references.

**Architecture:** One Content/ModularGameVFXLibrary root contains 00_Core plus 01_Cast through 15_UI. Typed metadata extends the existing catalog without replacing PerformanceClass or the async subsystem. Unreal AssetTools performs renames; the manifest records every old and new package. Unvalidated legacy systems remain explicitly identified, never reported as completed effects.

**Tech Stack:** UE 5.8 C++, reflected enums/structs, GameplayTags, Unreal Python, Asset Registry, Niagara scalability.

**Spec:** User-provided attachment `C:/Users/Freebooz/.codex/attachments/15a4b073-1859-4404-8468-27d69357295d/pasted-text.txt`.

## Constraints

- Work in the user's explicitly named F:/game/ModularGameVFX project.
- H:/GameVFX/MythicVFXLab contains zero files; do not repeat the prior disk migration.
- Coordinate with the active VFX task: this task owns asset migration, runtime taxonomy, tags and performance; that task owns its existing document generator.
- Back up before mutation, never overwrite a destination asset, and preserve existing authored performance settings.
- No completed skill system in Core; no duplicated systems for color variants.
- Keep async soft references and portable core plugin boundaries. Remove old package and class compatibility redirects after resaving referencers; preserve mapping only in backup evidence.

## Execution

- [x] Record file/asset inventory and back up Content, Config and Source for both plugins and host configuration. SHA256 verified 1,240 backup files.
- [x] Add typed Category, Element, Form, Context and Lifecycle metadata; expose existing PerformanceClass as Priority without changing its serialized name. Compile actual editor modules and verify reflected metadata.
- [x] Derive and validate a collision-free manifest for 1,059 real packages.
- [x] Rename through AssetTools, resave 1,293 referencing packages, remove the six remaining redirectors and eight obsolete empty content branches after reference fixup.
- [x] Register category-first invocation tags and create the real 13-entry default Catalog. Remove obsolete package/class CoreRedirects as explicitly requested.
- [x] Create four Effect Types and retain authored system/emitter overrides. Quality indices: VRMobile=0, Low=1, Medium=2, High=3, Cinematic=4.
- [x] Fresh verification: all 1,059 new paths/classes valid; 1,065 total core assets; no old files/registry entries/redirectors or forbidden dependencies; Catalog valid; 13 curated systems compile. Report 12 failed legacy systems separately.
- [x] Synchronize schema 1.3 Catalog JSON/MD/CSV/XLSX with actual paths, preserving historical paths only as traceable records and keeping ProductionReady at zero.

## Final evidence, 2026-09-08

`Saved/TaxonomyMigration/verification.json` passed with zero acceptance failures. The native `ModularGameVFXDemo.Migration.NativeDefaultReferences` Automation test verified all 15 protected CDO references against the current DLL. Verification saved no asset files.

The H:/GameVFX/MythicVFXLab disk tree still contains 0 files and 1,057 empty directories. Its recursive deletion was explicitly approved by the user, but automatic approval policy rejected the command before execution. This external cleanup remains blocked; no alternate deletion mechanism was used.

## Verification cases

1. Every formal entry has a category matching its category-first tag and folder.
2. Petal color variants retain one Niagara System.
3. Frostbolt cast/projectile/impact occupy Cast/Projectile/Impact respectively; warning belongs to Indicator.
4. Enum additions retain existing serialized spawn and performance properties.
5. A missing source or pre-existing destination aborts preflight before any rename.
6. All target paths load correctly; no old package files or redirectors remain; core references no /Game or demo content.
7. The four performance templates expose distance, instance-count, significance and quality-level rules; authored effects retain their original budgets.
