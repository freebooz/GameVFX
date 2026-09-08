# VFX Taxonomy Migration Implementation Plan

**Goal:** Implement the user's supplied taxonomy in the existing ModularGameVFX plugin and migrate actual assets. Latest user ruling: remove old paths after updating references.

**Architecture:** One Content/ModularGameVFXLibrary root contains 00_Core plus 01_Cast through 15_UI. Typed metadata extends the existing catalog without replacing PerformanceClass or the async subsystem. Unreal AssetTools performs renames; the manifest records every old and new package. Unvalidated legacy systems remain explicitly identified, never reported as completed effects.

**Tech Stack:** UE 5.8 C++, reflected enums/structs, GameplayTags, Unreal Python, Asset Registry, Niagara scalability.

**Spec:** User-provided attachment `C:/Users/Freebooz/.codex/attachments/15a4b073-1859-4404-8468-27d69357295d/pasted-text.txt`.

## Constraints

- Work in the user's explicitly named F:/game/MythicVFXLab project.
- H:/GameVFX/MythicVFXLab contains zero files; do not repeat the prior disk migration.
- Coordinate with the active VFX task: this task owns asset migration, runtime taxonomy, tags and performance; that task owns its existing document generator.
- Back up before mutation, never overwrite a destination asset, and preserve existing authored performance settings.
- No completed skill system in Core; no duplicated systems for color variants.
- Keep async soft references and portable core plugin boundaries. Remove old package and class compatibility redirects after resaving referencers; preserve mapping only in backup evidence.

## Execution

- [ ] Record file/asset inventory and back up Content, Config and Source for both plugins and host configuration.
- [ ] Add typed Category, Element, Form, Context and Lifecycle metadata; expose existing PerformanceClass as Priority without changing its serialized name. Verify enum reflection and the existing source contract; compile the editor modules.
- [ ] Derive a collision-free package/name manifest from real Asset Registry classes and known system roles. Use NS_/NE_/NM_/M_/MI_/MF_/T_/SM_/DA_/ET_/BP_ prefixes.
- [ ] Load local reference graph and migrate using AssetTools.rename_assets. Save affected packages and preserve redirectors until all references have been verified.
- [ ] Register category-first invocation tags and legacy redirects. Create/update the real default catalog with metadata and soft references to migrated assets.
- [ ] Create/reuse Critical, Combat, Cosmetic and Ambient Effect Types; retain per-system authored overrides and document the High/Medium/Low/VRMobile quality mapping.
- [ ] Verify target packages exist, old paths resolve, no forbidden core dependencies or duplicate destinations, metadata/tags agree, and current completed systems compile. Record legacy failures separately.
- [ ] Deliver taxonomy specification, migration manifest and verification report; synchronize the existing catalog document generator with actual paths.

## Verification cases

1. Every formal entry has a category matching its category-first tag and folder.
2. Petal color variants retain one Niagara System.
3. Frostbolt cast/projectile/impact occupy Cast/Projectile/Impact respectively; warning belongs to Indicator.
4. Enum additions retain existing serialized spawn and performance properties.
5. A missing source or pre-existing destination aborts preflight before any rename.
6. All target paths load correctly; no old package files or redirectors remain; core references no /Game or demo content.
7. The four performance templates expose distance, instance-count, significance and quality-level rules; authored effects retain their original budgets.
