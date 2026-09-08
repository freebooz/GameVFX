# ModularGameVFX Showcase implementation

The user's September 8 Showcase attachment is the implementation specification.
Use the explicitly named project F:/game/MythicVFXLab. Existing asset migration
continues before this plugin is enabled or new editor processes run.

## Architecture and decisions

- A removable ModularGameVFXShowcase plugin depends on ModularGameVFX only for
  formal Catalog and Manager calls. Core has no dependency on Showcase or Demo.
- A native controller owns preview handles, filters, separate preview profiles,
  environment selection and persisted human review records.
- Native UMG classes build dynamic menus and controls. Real Blueprint wrappers
  expose the named WBP and BP assets for designer customization.
- A generated real L_VFX_Showcase map runs generic test geometry and named
  attachment points, with no host character dependency. Existing engine basic
  shapes are reused. Skeletal animation/VR hardware verification needs actual
  suitable assets/hardware; it cannot be asserted from commandlet output.
- Runtime metrics report measured frame time and world-scoped managed/active
  requests. GPU Niagara cost and particle counts are unavailable until measured.
- Ruling: Keep the previous six ProductionReady gates. The latest three human
  review buttons cover Visual/Gameplay/Performance; Naming/Catalog/Dependency
  remain technical gates. Never infer human PASS from compile success.
- Ruling: enum metadata seeds the fifteen categories; catalog rows dynamically
  supply counts and current entries. Empty categories are visible and disabled
  by their empty result, without hand-wiring one button event per category.

## Ownership and work

1. Root: finish asset migration/cleanup; add core diagnostics getter; coordinate
   build, integration tests, final reports and catalog synchronization.
2. showcase_backend: plugin/module, backend data/controller/test actors and
   runtime tests. Publish header contract to UI and setup agents early.
3. showcase_ui: UMG root and panels, dynamic entries and real controller actions.
4. taxonomy_metadata: UE Python generation of real wrappers, map and profiles;
   read-only verification script and setup documentation.

## Verification

- Compile core, showcase and generated reflected classes using actual UE5.8 UBT.
- Generate actual map, BP/WBP and profile uassets using editor factories.
- Load all generated assets in a fresh process; compile Blueprints where APIs
  permit; validate references, required classes, Catalog tags and dependency
  direction.
- Exercise filter/search, review gating, parameter reset, bounds on stress count,
  handle cleanup and unsupported preview states through reflected/runtime tests.
- Attempt actual game world preview and capture only if available; commandlet
  structure checks are not a substitute for human visual or GPU profiling.
- Document delivered functions and all unverified visual/hardware items.
