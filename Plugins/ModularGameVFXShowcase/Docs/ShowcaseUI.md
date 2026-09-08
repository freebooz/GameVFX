# Showcase workbench UI

`UVFXShowcaseWidget` builds the complete runtime UMG tree. Empty Widget Blueprint wrappers may derive from it; a default Designer Canvas Panel is replaced during `RebuildWidget`, before Slate renders the content. Set `Controller` before `AddToViewport`, or place one Showcase Controller in the world for automatic discovery.

The eight requested wrapper parent classes are in `VFXShowcaseWidget.h`: `VFXShowcaseWidget`, `VFXShowcaseCategoryMenu`, `VFXShowcaseEntryList`, `VFXShowcaseEntryItem`, `VFXShowcaseInspector`, `VFXShowcaseParameterPanel`, `VFXShowcasePerformancePanel`, and `VFXShowcaseReviewPanel`. Their module is `/Script/ModularGameVFXShowcase`. The root constructs these native panels automatically. The child wrappers are extension points; an empty child wrapper intentionally contains only its native content container until populated by the workbench.

## Browse and play

The left browser reads category progress and metadata rows from the controller. Category buttons are generated from `GetCategories`; there are no per-category click handlers. The controller's aggregate Unspecified row is displayed once as All, including its complete Catalog count. Both the category menu and entry list scroll vertically. Search, element, review status and sorting update the controller filter, so Previous, Next and Auto Preview follow the same filtered sequence. Entry captions use a defined wrap width and content padding so the button height includes every text line; long Inspector paths can wrap within an unbroken identifier.

Click an entry to stop the prior single preview and play the new gameplay tag through the controller and formal VFX Manager. The UI never receives a Niagara system reference and never spawns one. Entry cards show display name, tag, element, form, priority, favorite and review status. The inspector shows metadata, development asset path, structural information and validation issues.

Play, Replay, Stop, Stop Immediate, Loop, Auto Preview, Previous and Next dispatch to the controller. Select two entries with Set A and Set B, then Compare A / B. Stress buttons request 1, 5, 10, 20, 50 or 100 instances in the separate stress zone. Combat simulation dispatches the controller's catalog-based composition.

## Inspect and tune

The right side scrolls independently to keep long paths, parameter inputs and reviews inside the viewport. Environment controls switch real background, quality and playback speed. Preview distance changes the effect's target distance; Near, Medium and Far camera controls change the observation distance separately. Motion and surface buttons choose the controller's test motion or surface. VR preview toggles the controller's VR preview and quality behavior.

Parameter rows come from the selected resource's exposed parameter names. Supported numeric inputs include intensity, scale, lifetime, radius, width, length and progress. Supported primary and secondary colors expose editable linear RGBA channels, including HDR RGB values. All changes go through controller parameter setters; Reset reloads Catalog defaults. Resources with no exposed parameters are identified explicitly.

Live telemetry is refreshed four times a second from controller measurements. FPS, frame time, active and managed counts, Niagara count, quality and requested test count are shown. Niagara CPU, GPU and particle counts show N/A when a reliable measurement is unavailable. Acceptance still requires target-device profiling evidence.

## Review

Select the Visual, Gameplay, Performance, Naming, Catalog and Dependency verdicts, enter the reviewer name and evidence, then Save review. Evidence is attached to each changed gate. Previously recorded gate evidence, reviewer and timestamp remain visible in the inspector. Select a final status and a failure reason as appropriate. Notes support multiline text, including Chinese.

Final PASS requests Production Ready; it does not set any gate to Pass. The controller enforces the evidence requirements for the current asset version. Save + Next advances only after successful persistence. Favorite state is maintained by the controller's review storage.

This UI does not assign approval automatically from a valid tag, a successful spawn, an FPS reading or an empty validation-issues list.

## Integration checks

Compile the module with UMG, Slate and SlateCore dependencies. In the generated Showcase level, verify that a wrapper with a Designer-created Canvas root displays the native layout, a category click changes the matching rows, an entry click reaches VFX Manager, parameter reset restores the Catalog values, and a failed review save does not advance to another entry. Check wide and 1280 × 720 viewports; both side panels scroll and the central stage remains visible.
