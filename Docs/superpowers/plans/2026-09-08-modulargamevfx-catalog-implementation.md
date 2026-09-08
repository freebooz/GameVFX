# ModularGameVFX Catalog Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rename the reusable VFX plugin to `ModularGameVFX` and add a production runtime Catalog API that resolves Gameplay Tags to soft Niagara references, loads them asynchronously, and spawns them through one subsystem-owned path.

**Architecture:** `UModularGameVFXBlueprintLibrary` is the only public gameplay entry point. Every location/attachment wrapper creates an `FVFXPlayRequest` and forwards it to one `UModularGameVFXSubsystem::PlayVFX` path, which returns an `FVFXHandle` before asynchronous loading begins. The subsystem queries `UVFXCatalog`, coalesces loads by soft asset path, resolves Niagara with `FStreamableManager`, applies parameter overrides, and calls one private `SpawnResolvedVFX` function; the optional `ModularGameVFXDemo` plugin consumes this API but stays outside the reusable core.

**Tech Stack:** Unreal Engine 5.8 C++, UHT reflection, Niagara, GameplayTags, UGameInstanceSubsystem, UDeveloperSettings, AssetManager/StreamableManager, PrimaryDataAsset, Core Redirects.

**Spec:** `Docs/superpowers/specs/2026-09-08-modulargamevfx-rename-design.md` and the user-provided ModularGameVFX VFX Catalog specification attached on 2026-09-08.

## Global Constraints

- Core plugin, runtime module, API namespace, documentation, installer, and release identity use `ModularGameVFX`.
- Runtime content uses `/ModularGameVFX`; optional demo content uses `/ModularGameVFXDemo`.
- Gameplay code references `FGameplayTag`, never a concrete `UNiagaraSystem` asset path.
- `FVFXCatalogEntry::NiagaraSystem` is `TSoftObjectPtr<UNiagaraSystem>`.
- Catalog lookup uses a runtime `TMap<FGameplayTag, int32>` cache and reports duplicate tags.
- Niagara assets load only when requested; plugin initialization never synchronously loads the complete VFX library.
- `UModularGameVFXBlueprintLibrary` is the only public play/query API.
- `UModularGameVFXSubsystem` owns catalog lookup, async loading, lifecycle checks, error logging, and Niagara spawning.
- One private `SpawnResolvedVFX` function is the only call site for `UNiagaraFunctionLibrary::SpawnSystemAtLocation` and `SpawnSystemAttached`.
- Dedicated servers return safely without spawning VFX.
- Core plugin has no `/Game`, demo code, Character, Ability, damage, RPC, cooldown, or project-specific framework dependencies.
- Existing project-specific Tiger tags are data-only examples and never appear in framework C++.
- `UModularGameVFXSubsystem` is the single VFX Manager implementation; no parallel manager or direct-spawn API is introduced.
- Every Catalog entry records Critical, Combat, Cosmetic, or Ambient priority and a native Niagara pooling method.
- Current spell assets retain their authored Effect Type, fixed bounds, instance limits, culling, scalability, and particle budgets; the Catalog layer must not force-load or duplicate those controls.
- The plugin ships the user-provided `ModularGameVFX AI视觉开发全局策略与内置性能规范 V1.0` as its long-term visual and performance policy.
- Public callers keep `FVFXHandle`, never a raw `UNiagaraComponent*`; async completion reports the handle and success state.
- Concurrent requests for the same unloaded Niagara path share one `FStreamableHandle` and fan out only after that load completes.
- Stop, immediate stop, owner cleanup, tag cleanup, parameter changes, preload, and preload release all resolve through the same GameInstance subsystem.
- Existing legacy Niagara compilation failures must be reported honestly and cannot be attributed to this Catalog implementation.
- The formal asset root is `/ModularGameVFX/ModularGameVFXLibrary`, with exactly `00_Core` plus functional directories `01_Cast` through `15_UI`.
- `00_Core` preserves the requested visual primitives as `Energy`, `Fire`, `Smoke`, `Sparks`, `Dust`, `Lightning`, `Glow`, `Shockwave`, `Debris`, `Noise`, `Ribbon`, `Meshes` (the physical form of requested `Mesh`), `Textures`, `Materials`, and `NiagaraModules`; it also holds shared `MaterialFunctions` and `Curves`, plus the management-only `EffectTypes` and `Catalog` directories. Complete business effects are classified by function outside `00_Core`.
- Every Catalog record carries the primary functional category plus Element, Form, Context, Lifecycle, Priority, GameplayTag, Effect Type, precise `MGVFX-*` ID, and legacy ID where applicable.

---

### Task 1: Baseline, Backup, and Canonical Plugin Names

**Files:**
- Rename: `Plugins/MythicVFX` → `Plugins/ModularGameVFX`
- Rename: `Plugins/MythicVFX/MythicVFX.uplugin` → `Plugins/ModularGameVFX/ModularGameVFX.uplugin`
- Rename: `Plugins/MythicVFXDemo` → `Plugins/ModularGameVFXDemo`
- Rename: `Plugins/MythicVFXDemo/MythicVFXDemo.uplugin` → `Plugins/ModularGameVFXDemo/ModularGameVFXDemo.uplugin`
- Modify: `ModularGameVFX.uproject`
- Backup: `Saved/ModuleRelease/BackupBeforeModularGameVFXCatalog/<timestamp>/`

**Interfaces:**
- Consumes: current `MythicVFX` core plugin and `MythicVFXDemo` optional demo plugin.
- Produces: canonical plugin directories discoverable as `ModularGameVFX` and `ModularGameVFXDemo`.

- [ ] Stop PIE, save dirty packages, close the current Unreal Editor process normally, and verify no target-project Unreal process remains.
- [ ] Copy both plugin directories, the `.uproject`, `Config`, and `Source` into the timestamped backup without modifying the existing migration backup.
- [ ] Record pre-migration asset registry paths, plugin file hashes, current 13-system compile status, and current Git status.
- [ ] Rename directories and descriptors; set core `FriendlyName`, `CreatedBy`, and description to `ModularGameVFX`; set `CanContainContent=true` and add Runtime module `ModularGameVFX`.
- [ ] Update `.uproject` plugin entries to `ModularGameVFX` and `ModularGameVFXDemo` while keeping the host project/module name `MythicVFXLab`.

### Task 2: Runtime Types, Catalog Data Asset, and Validation

**Files:**
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/ModularGameVFX.Build.cs`
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Public/VFXTypes.h`
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Public/VFXCatalog.h`
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Private/VFXCatalog.cpp`
- Test: `Plugins/ModularGameVFX/Tests/Test_ModularGameVFX_Catalog.py`

**Interfaces:**
- Produces: `EVFXSpawnMode`, `EVFXPriority`, `FVFXParameterOverrides`, `FVFXCatalogEntry`, `FVFXPlayRequest`, `FVFXHandle`, `FOnModularVFXReady`, `UVFXCatalog::FindEntry`, `UVFXCatalog::FindEntryNative`, `UVFXCatalog::BuildRuntimeCache`.

- [ ] Add only `Core`, `CoreUObject`, `Engine`, `Niagara`, `GameplayTags`, `DeveloperSettings`, and `Projects`, assigning dependencies Public or Private according to exported headers.
- [ ] Define `EVFXSpawnMode { AtLocation, Attached }` and `EVFXPriority { Critical, Combat, Cosmetic, Ambient }` as Blueprint enums.
- [ ] Define `FVFXParameterOverrides` with Blueprint-editable float, vector, and linear-color maps keyed by standard Niagara `User.*` parameter names.
- [ ] Define `FVFXCatalogEntry` with `VFXTag`, soft Niagara reference, display text, description, spawn mode, default scale/offset/rotation/socket, auto-destroy, UE 5.8 `ENCPoolMethod`, priority, and default parameters.
- [ ] Define `FVFXPlayRequest` with tag, mode, transform, weakly consumed attach component/owner references, socket, explicit-default flags, auto-destroy/pooling override flags, and parameter overrides.
- [ ] Define `FVFXHandle` around `FGuid`, Blueprint `IsValid`, equality, and hashing; invalid handles use an invalid GUID.
- [ ] Implement `UVFXCatalog` as `UPrimaryDataAsset` with editable `TArray<FVFXCatalogEntry>` and transient `TMap<FGameplayTag, int32>`.
- [ ] Implement native O(1) lookup and a Blueprint-copy lookup; build cache after load and report invalid/duplicate/empty entries without silently overwriting the first valid entry.
- [ ] Override UE 5.8 `IsDataValid(FDataValidationContext&) const` under editor builds and emit exact errors for invalid tag, duplicate tag, or empty Niagara soft reference.
- [ ] Add an editor Python reflection test that verifies property types, soft-reference behavior, duplicate handling, and absence of hard Niagara references in the Catalog class.

### Task 3: Settings and GameplayTag Registration

**Files:**
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Public/ModularGameVFXSettings.h`
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Private/ModularGameVFXSettings.cpp`
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Private/ModularGameVFX.cpp`
- Create: `Plugins/ModularGameVFX/Config/Tags/ModularGameVFXTags.ini`
- Create: `Plugins/ModularGameVFX/Config/DefaultModularGameVFX.ini`

**Interfaces:**
- Produces: `UModularGameVFXSettings::DefaultCatalog` and module startup registration for plugin GameplayTag config and legacy package redirects.

- [ ] Implement `UModularGameVFXSettings` in Project Settings under Plugins → Modular Game VFX with `TSoftObjectPtr<UVFXCatalog> DefaultCatalog`.
- [ ] Register `Config/Tags` through `UGameplayTagsManager::AddTagIniSearchPath` using the plugin base directory found by `IPluginManager`.
- [ ] Add required base tags under `VFX`, `VFX.Common`, `VFX.Element`, `VFX.Character`, `VFX.World`, and `VFX.UI`.
- [ ] Add Tiger sample tags only to the data `.ini`; do not reference them from C++.
- [ ] Add real existing-spell tags for the 13 current spell systems so the default Catalog can reference actual Niagara assets.
- [ ] Register substring package redirects from `/MythicVFX` to `/ModularGameVFX` and keep host config redirects for portable compatibility.

### Task 4: One-Path Async Subsystem

**Files:**
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Public/ModularGameVFXSubsystem.h`
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Private/ModularGameVFXSubsystem.cpp`
- Test: `Plugins/ModularGameVFX/Tests/Test_ModularGameVFX_Subsystem.py`

**Interfaces:**
- Consumes: `UModularGameVFXSettings::DefaultCatalog`, `UVFXCatalog`, `FVFXCatalogEntry`, `FVFXPlayRequest`, `FVFXHandle`, `FOnModularVFXReady`.
- Produces: private library-friend methods `PlayVFX`, stop/parameter/preload operations, `FindVFXEntry`, one resolver, and one `SpawnResolvedVFX` choke point.

- [ ] Initialize the GameInstance subsystem by reading the default Catalog soft reference and requesting it asynchronously without loading any Niagara systems.
- [ ] Store the loaded Catalog with a GC-visible `TObjectPtr`, build its cache, and keep catalog, Niagara, and preload `FStreamableHandle` instances until their ownership ends.
- [ ] Allocate a valid `FVFXHandle` before load, store a runtime instance keyed by GUID, and index it by weak owner and GameplayTag.
- [ ] Implement the library-friend `PlayVFX` request method that validates world, net mode, tag, catalog state, attach component, and soft Niagara path.
- [ ] If the Catalog is still loading, continue the same request after Catalog load; if Niagara is already resident, continue immediately through the same resolver.
- [ ] Load only the requested Niagara soft path with `UAssetManager::GetStreamableManager().RequestAsyncLoad`; coalesce requests in `PendingLoads<FSoftObjectPath, WaitingHandles>` and fan out after one shared load.
- [ ] Capture world and attach component as weak object pointers; validate both again after async completion.
- [ ] Implement exactly one private `SpawnResolvedVFX` that applies Catalog defaults followed by request overrides, calls the correct UE 5.8 Niagara function with `bAutoDestroy` and `ENCPoolMethod`, applies standard parameters, binds completion, and records the component internally.
- [ ] Implement `StopVFX`, `StopVFXImmediate`, `StopAllVFXForOwner`, and `StopVFXByTag` without scanning the world; cancelling a pending handle prevents its eventual spawn.
- [ ] Implement float, vector, color, and batch parameter updates by handle; pending instances retain overrides and spawned instances update immediately.
- [ ] Implement `PreloadVFX`, `PreloadVFXTags`, and `ReleasePreloadedVFX` with per-tag reference counts and soft handles; never preload the complete Catalog automatically.
- [ ] Periodically prune destroyed weak owners/components through the subsystem lifecycle and remove all handles, delegates, pending requests, and preloads in `Deinitialize`.
- [ ] Define `LogModularGameVFX` and log actionable warnings for every safe-failure case without using `LogTemp`.
- [ ] Verify by source scan that Niagara spawn functions appear only inside `ModularGameVFXSubsystem.cpp`.

### Task 5: Blueprint and C++ Public API

**Files:**
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Public/ModularGameVFXBlueprintLibrary.h`
- Create: `Plugins/ModularGameVFX/Source/ModularGameVFX/Private/ModularGameVFXBlueprintLibrary.cpp`
- Test: `Plugins/ModularGameVFX/Tests/Test_ModularGameVFX_BlueprintAPI.py`

**Interfaces:**
- Produces: `PlayVFX`, location/attachment wrappers, handle lifecycle operations, parameter operations, preload operations, and `FindVFXEntry` as the only public Blueprint/C++ business API.

- [ ] Add static Blueprint-callable `PlayVFX` accepting `FVFXPlayRequest`, returning `FVFXHandle` immediately, and accepting a completion delegate that reports handle plus success without exposing a component.
- [ ] Add static Blueprint-callable `PlayVFXAtLocation` and `PlayVFXAttached` convenience functions that only build an `FVFXPlayRequest` and call `PlayVFX`.
- [ ] Add `StopVFX`, `StopVFXImmediate`, `StopAllVFXForOwner`, `StopVFXByTag`, `IsVFXHandleValid`, float/vector/color/batch setters, `PreloadVFX`, `PreloadVFXTags`, and `ReleasePreloadedVFX`; every function only resolves and forwards to the subsystem.
- [ ] Add static Blueprint-pure `FindVFXEntry` that copies an entry and never exposes a native pointer.
- [ ] Forward all three functions to the GameInstance subsystem; do not query a Catalog, load an asset, or call Niagara from the library.
- [ ] Add reflection tests verifying node names, World Context metadata, handle/delegate pins, input types, public API completeness, and return behavior for error tags.

### Task 6: Rename the Optional Demo Module and Migrate Asset Paths

**Files:**
- Rename: `Plugins/ModularGameVFXDemo/Source/MythicVFXDemo` → `Plugins/ModularGameVFXDemo/Source/ModularGameVFXDemo`
- Rename: `MythicVFXDemo.Build.cs` → `ModularGameVFXDemo.Build.cs`
- Rename: `MythicVFXDemoModule.cpp` → `ModularGameVFXDemoModule.cpp`
- Modify: demo public/private C++ files, descriptor, tests, docs, and configs.
- Modify: `Config/DefaultEngine.ini`

**Interfaces:**
- Consumes: public `ModularGameVFX` module and `/ModularGameVFX` assets.
- Produces: `/Script/ModularGameVFXDemo` and `/ModularGameVFXDemo/Maps/演示关卡`.

- [ ] Rename the runtime module class, API macro, Build.cs class, module declaration, descriptor module entry, and binary manifest identity.
- [ ] Replace demo hardcoded content paths with `/ModularGameVFX` and `/ModularGameVFXDemo` paths; do not refactor Character, GameMode, input, animation, combat, or AI behavior.
- [ ] Replace demo gameplay Niagara spawn/load call sites with GameplayTag requests through `UModularGameVFXBlueprintLibrary`, keeping existing timing, transforms, damage, AI, and animation behavior unchanged.
- [ ] Add package and class redirects from both legacy script package names to `/Script/ModularGameVFXDemo`.
- [ ] Move the plugin directories while the editor is closed, then open the project with the new modules and resave assets so serialized references use the canonical mount roots.
- [ ] Fix redirectors and audit hard/soft asset dependencies for forbidden legacy roots or `/Game` dependencies.

### Task 7: Default Catalog Asset and Existing Niagara Mappings

**Files:**
- Create via Unreal: `/ModularGameVFX/Data/DA_VFXCatalog_Default`
- Modify: project/plugin settings for `DefaultCatalog`.
- Test: `Plugins/ModularGameVFX/Tests/Test_ModularGameVFX_Runtime.py`

**Interfaces:**
- Consumes: compiled `UVFXCatalog` type and 13 existing production spell Niagara systems.
- Produces: real GameplayTag-to-soft-Niagara mappings with no nonexistent asset paths.

- [ ] Create the Primary Data Asset through Unreal Editor scripting after UHT exposes `UVFXCatalog`; never fabricate a binary `.uasset`.
- [ ] Add entries for the existing Frost, Fire, Arcane, and Nature systems with real soft object paths, readable names, descriptions, modes, pooling, auto-destroy, and priorities.
- [ ] Assign the created data asset to `UModularGameVFXSettings::DefaultCatalog` without a hardcoded C++ path.
- [ ] Verify Catalog loading does not load all Niagara systems, then request one unloaded system and confirm its callback marks the returned handle as spawned.
- [ ] Verify world-location spawn, socket-attached spawn, dynamic color/intensity, normal stop, immediate stop, stop during load, owner destruction, stop by tag, invalid tag, missing Catalog, destroyed-world/attach safety, dedicated-server early exit, pooling, preload/release, and many concurrent requests for one unloaded system.

### Task 8: Catalog Files, Installer, Release, and Validation

**Files:**
- Rename/update: `Plugins/ModularGameVFX/Documentation/ModularGameVFX_1.0.0_VFX_Catalog.*`
- Create: `Plugins/ModularGameVFX/Documentation/ModularGameVFX_AI_Visual_Performance_Policy_V1.0.md`
- Rename/update: `Install-ModularGameVFX.ps1`
- Update: release candidate builder, manifest, dependency audit, and standalone validation scripts.
- Produce: `F:/game/ModularGameVFX_Releases/1.0.0/`

**Interfaces:**
- Produces: installable core plugin, optional demo plugin, truthful VFX Catalog, installer, hash manifest, and validation evidence.

- [ ] Regenerate Excel, CSV, JSON, and Markdown Catalogs with `MGVFX-*` primary IDs and `Legacy VFX ID` mappings.
- [ ] Include the 16-directory taxonomy, 17 `00_Core` primitive/shared directories, two `00_Core` management directories, multidimensional metadata rules, all current effect names, current compatibility paths, standardized target paths, and migration status in every canonical Catalog source; Excel presents these as dedicated worksheets.
- [ ] Generate the Catalog from `Saved/TaxonomyMigration/manifest.json`, include all 1,059 asset mappings, use `ET_VFX_*` for documented priority Effect Types, and mark migration complete only when `Saved/TaxonomyMigration/migration_complete.json` exists.
- [ ] Store the supplied visual/performance policy unchanged in the plugin documentation and reference it from the installation and contribution guides.
- [ ] Update all asset paths, module names, filenames, descriptions, and install locations; remove old branding from canonical release files while preserving explicit migration documentation.
- [ ] Build `MythicVFXLabEditor Win64 Development` with modules `MythicVFXLab`, `ModularGameVFX`, and `ModularGameVFXDemo`; treat UHT or compile errors as blocking.
- [ ] Run Catalog reflection/data validation tests, one-path source scan, dependency audit, and current 13-system Niagara diagnostics.
- [ ] Run the `演示关卡` smoke test and manually verify keys 1–8 after the automated checks finish.
- [ ] Install only `ModularGameVFX` into an empty UE 5.8 C++ host, compile/load it, verify base GameplayTags, and exercise one Catalog request.
- [ ] Install core plus demo into the fixture, open `/ModularGameVFXDemo/Maps/演示关卡`, and confirm plugin independence from the original host project.
- [ ] Build a fresh versioned release directory, compute SHA-256 for every delivered file, verify the installer against the manifest, and record all legacy-system failures separately from the current production systems.
