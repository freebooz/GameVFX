# ModularGameVFX 代码性能审查报告

- **审查日期**：2026-09-09
- **目标工程**：Unreal Engine 5.8，`F:\game\GameVFX`
- **审查范围**：`ModularGameVFX` 核心运行时、`ModularGameVFXDemo` 战斗演示、`ModularGameVFXShowcase` 验收工具、宿主模块与相关配置
- **代码口径**：排除 `Intermediate` 和 UHT 生成代码，共 38 个手写 C++/C#/头文件，约 5,376 行
- **当前内容规模**：插件内容目录 1,065 个文件，约 104 MiB；按资产命名统计含 36 个 Niagara System、389 个纹理、479 个材质类资产；质量登记表含 36 个 VFX 条目
- **方法**：静态代码阅读、调用链追踪、复杂度与分配模式检查、全工程模式扫描、配置检查
- **证据边界**：本报告确认的是代码路径、复杂度和阻塞/分配行为。尚未在目标硬件上采集 Unreal Insights、GPU Profile、粒子数、透明过绘或纹理驻留数据，因此不宣称任何效果已经通过性能门禁，也不把风险严重度解释为已测得的毫秒数。

## 结论

当前核心插件已经具备正确的基础：Catalog 使用软引用和标签索引，Subsystem 合并同一资源的异步加载请求，支持预加载引用计数、Niagara 组件池、弱引用生命周期清理，并过滤 Dedicated Server。主要性能风险来自两处边界被破坏：

1. Showcase 在游戏线程上同步载入并检查整个 Catalog，还递归遍历和哈希每个系统的依赖；这个重操作在启动时会执行两次。当前 36 个系统已经可能造成冷启动停顿，Catalog 扩展到数百或数千条目后不可持续。
2. Demo 战斗代码有 12 处绕过 `UModularGameVFXBlueprintLibrary → UModularGameVFXSubsystem → UVFXCatalog` 的直接 Niagara 生成，并通过构造函数硬引用资产。它绕过异步加载、统一句柄、生命周期统计、未来预算控制和跨项目解耦。

在修复这两个问题之前，不建议把当前版本标记为性能通过或 Production Ready。建议先完成 P0，再做实例预算与遥测、投射物/点光优化，最后处理 Showcase 的数据结构和 UI 扩展性。

## 风险总览

| ID | 严重度 | 范围 | 发现 | 主要影响 | 证据状态 |
| --- | --- | --- | --- | --- | --- |
| PERF-P0-01 | Critical | Showcase | 启动时全量同步加载、递归依赖哈希，并重复刷新两次 | 冷启动卡顿、游戏线程阻塞、软引用资产被整体拉入内存 | 静态确认 |
| PERF-P0-02 | Critical | Demo / 架构 | 业务代码硬引用并直接生成 Niagara，绕过唯一调用链 | 同步载入、生命周期与预算失控、模块迁移能力下降 | 静态确认 |
| PERF-P1-03 | High | Showcase / Core | 每帧遍历全局 Niagara UObject，再线性扫描全部受管实例 | 测量逻辑自身成为 Game Thread 开销，随对象数增长 | 静态确认 |
| PERF-P1-04 | High | Core | `PerformanceClass` 未参与任何运行时准入、限流或降级 | 请求洪峰可无限创建句柄、等待项和 Niagara 实例 | 静态确认 |
| PERF-P1-05 | High | Demo | 每枚投射物使用独立 Actor、Tick、碰撞查询和部分独立点光 | CPU、GC、Primitive/Light 与碰撞开销按弹量线性增长 | 静态确认 |
| PERF-P1-06 | High | Demo | 单只傀儡由 15 个静态网格组件组成，并逐帧更新多块网格 | Draw Call、组件遍历、渲染状态更新随怪物数放大 | 静态确认 |
| PERF-P1-07 | High | Showcase UI | 非虚拟化条目列表在每个搜索字符上全量过滤、排序和重建 Widget | Catalog 扩大后出现输入卡顿、Slate/GC 压力和内存抖动 | 静态确认 |
| PERF-P2-08 | Medium | Showcase | 标签可见性每帧执行投影和 `SlatePrepass` | 验收场景基线被 UI 测量污染 | 静态确认 |
| PERF-P2-09 | Medium | Demo | AI、治疗地面跟随、奥术引导和飞弹在逐帧路径上重复查询/射线 | 多怪、多持续区域、多飞弹时 Game Thread 扩展性差 | 静态确认 |
| PERF-P2-10 | Medium | Demo HUD | Canvas HUD 每帧重建多组字符串和 `FText`，并重复执行翻译替换 | 持续分配、文本布局与 Canvas 提交开销 | 静态确认 |
| PERF-P2-11 | Medium | Core | 每次播放多次复制参数/Catalog 结构，清理 Ticker 空闲时仍常驻扫描 | 高频生成下产生容器分配与复制，空场景也有固定轮询 | 静态确认 |
| PERF-P2-12 | Medium | Showcase 数据层 | 多处线性查找组合成 O(N²)，排序比较器反复 `FText::ToString` | 评审数据和 Catalog 增长后刷新、保存、搜索变慢 | 静态确认 |
| PERF-P3-13 | Low | 打包 / 配置 | Demo/Showcase 是 Runtime 模块；工程还选择 Full Ray Tracing 与多个着色目标 | 若生产工程沿用，可增加模块、Cook/Shader 和潜在 GPU 成本 | 需按目标构建确认 |

## 详细发现

### PERF-P0-01 — Showcase 在启动阶段同步加载并递归处理整个 Catalog，而且执行两次

**证据**

- `AVFXShowcaseController::BeginPlay` 在创建界面前调用 `RefreshCatalog()`：`Plugins/ModularGameVFXShowcase/Source/ModularGameVFXShowcase/Private/VFXShowcaseController.cpp:90-100`。
- `UVFXShowcaseWidget::NativeConstruct` 又调用自己的 `RefreshCatalog()`，继而再次调用 Controller：`Plugins/ModularGameVFXShowcase/Source/ModularGameVFXShowcase/Private/VFXShowcaseWidget.cpp:246-252`、`:404-419`。
- Controller 同步载入 Catalog：`VFXShowcaseController.cpp:170-175`。
- 对每一条目同步载入 Niagara System 并读取 Emitter、Effect Type、Fixed Bounds 和参数：`VFXShowcaseController.cpp:182-214`，关键同步调用位于 `:195`。
- `VersionFor` 对每个条目递归获取全部非 Engine 依赖、读取包文件并计算 MD5，之后排序并再次拼接整个依赖集合：`VFXShowcaseController.cpp:53-78`。文件哈希只在一次刷新内按包缓存；依赖图遍历、排序和序列化仍会为每个条目重复，第二次刷新又从空缓存开始。

**影响**

- 这些操作位于 Game Thread 的 `BeginPlay/NativeConstruct` 路径，会直接形成冷启动长帧。
- `TSoftObjectPtr` 的价值被 Showcase 的全量 `LoadSynchronous` 抵消；36 个系统及其材质、纹理、网格和模块依赖会被提前装入。
- Catalog 规模扩大时，成本近似为“条目数 × 依赖图遍历”，并叠加同步磁盘读取。共享依赖只能减少重复哈希，不能消除重复图遍历。

**修复建议**

1. 只保留 Controller 的一次初始化；Widget 订阅“Catalog 元数据就绪”事件并读取现有结果，不再主动触发全量刷新。
2. Catalog 本身继续异步加载。列表首屏只读取软路径和 Catalog 元数据，不载入 Niagara System。
3. Emitter、Effect Type、Bounds 和公开参数改为选中条目时异步检查，或在 Editor/Commandlet 阶段生成轻量诊断清单。
4. 资产版本在保存/Cook/CI 阶段生成。运行时优先使用 Asset Registry 的包版本、Cooked Hash 或预生成 manifest；如果仍需文件哈希，按绝对路径、大小和修改时间做跨刷新缓存，并放到后台任务。
5. 用冷缓存 PIE/Standalone 记录 `LoadTime`、Game Thread 最大帧、载入包数量和进程驻留内存，比较修复前后。

### PERF-P0-02 — Demo 绕过唯一规定的 Catalog/Subsystem 调用链

**证据**

- 核心要求的合法出口只有 Subsystem 内两处：`ModularGameVFXSubsystem.cpp:344-368`。
- Demo 另有 12 处 `UNiagaraFunctionLibrary::SpawnSystemAtLocation/Attached`：
  - 冰霜：`Plugins/ModularGameVFXDemo/Source/ModularGameVFXDemo/Private/FrostMageGameplay.cpp:133`、`:168`、`:173`、`:259`、`:309`。
  - 火焰：`Plugins/ModularGameVFXDemo/Source/ModularGameVFXDemo/Private/FireMageGameplay.cpp:35`、`:87`、`:94`、`:112-113`。
  - 奥术：`Plugins/ModularGameVFXDemo/Source/ModularGameVFXDemo/Private/ArcaneMageGameplay.cpp:30`、`:54`。
- Demo 通过 `ConstructorHelpers::FObjectFinder` 或其 `FM::Asset/FireMage::Asset` 包装器，在 Actor 构造路径中加载 Niagara、材质、网格和动画：`FrostMageGameplay.cpp:35`、`:79-98`、`:157-161`、`:179-185`、`:213-231`；`FireMageGameplay.cpp:14`；`ArcaneMageGameplay.cpp:13-17`。
- Frost Impact 的两处直接生成没有显式指定池化方法：`FrostMageGameplay.cpp:168`、`:173`。

**影响**

- Demo 类加载或实例构造会同步解析资产；投射物类还在频繁 Spawn 路径中运行构造代码。
- 这些 VFX 不进入 Subsystem 的句柄表、Owner/Tag 索引、异步加载合并、预加载引用计数和清理统计。
- 后续加入优先级预算、质量降级或统一遥测时，Demo 仍会绕过它们，导致验收关卡不能代表插件真实业务调用性能。
- 硬引用把 Demo 与具体资源路径绑定，降低插件独立安装、升级和迁移能力。

**修复建议**

1. Demo 只保存 `FGameplayTag`；所有视觉生成统一调用 `UModularGameVFXBlueprintLibrary`。
2. 在战斗关卡加载或 Pawn 初始化时通过 `PreloadVFXTags` 预加载 1–8 号键所需效果，退出关卡时对称释放。
3. 保存返回的 `FVFXHandle`，取消施法、目标死亡、Actor EndPlay 时通过统一 API 停止。
4. 投射物 Actor 只负责 Gameplay 轨迹/碰撞；视觉通过 Attached 请求挂到其 Root，命中视觉通过 AtLocation 请求生成。
5. 加一条静态 CI 规则：除 `ModularGameVFXSubsystem.cpp` 外，项目代码出现 `UNiagaraFunctionLibrary::SpawnSystem*` 或业务类硬引用 `UNiagaraSystem` 即失败。

### PERF-P1-03 — Showcase 每帧扫描全局 Niagara UObject 和全部受管实例

**证据**

- `AVFXShowcaseController::Tick` 每帧调用 `GetVFXRuntimeCounts`，随后执行 `TObjectIterator<UNiagaraComponent>`：`VFXShowcaseController.cpp:484-490`。
- `TObjectIterator` 遍历进程中所有已加载的 Niagara Component UObject，之后才用 `GetWorld()` 过滤；成本并不只与当前世界活动实例数相关。
- `UModularGameVFXSubsystem::GetVFXRuntimeCounts` 又线性遍历 `RuntimeInstances`：`Plugins/ModularGameVFX/Source/ModularGameVFX/Private/ModularGameVFXSubsystem.cpp:424-435`。
- Widget 实际仅每 0.25 秒刷新一次文本：`VFXShowcaseWidget.cpp:719-725`，因此 Controller 的逐帧全量采样没有显示收益。

**影响**

验收工具本来用于观察性能，却在每帧引入与全局 UObject 数和受管实例数相关的额外 Game Thread 成本。编辑器中残留对象多、压力测试实例多或多个 PIE World 并存时偏差更明显。

**修复建议**

- Subsystem 在 Spawn、Finish、Stop、World 切换时维护每世界的 Managed/Active 原子计数或普通 Game Thread 计数。
- Showcase 以 2–4 Hz 读取这些计数；只有显式“诊断全局组件”操作才运行对象遍历。
- 遥测采样代码放入独立 Trace Channel，Shipping 默认关闭，并用 Unreal Insights Counter 展示。

### PERF-P1-04 — 核心没有运行时实例预算，`PerformanceClass` 当前只是展示字段

**证据**

- `EVFXPriority PerformanceClass` 定义于 `Plugins/ModularGameVFX/Source/ModularGameVFX/Public/VFXTypes.h:123-134`。
- 全工程运行时代码中，该字段只被 Showcase 复制到列表行：`VFXShowcaseController.cpp:187`；Subsystem 不读取它。
- 每个合法请求都会立即创建 GUID、`RuntimeInstances` 记录、Owner/Tag 索引，并可能加入 Catalog 或资源等待队列：`ModularGameVFXSubsystem.cpp:111-154`。
- 同资源请求会合并异步加载，这是正确的；但等待句柄数量、每世界活动数量、每 Tag 速率和优先级没有上限：`ModularGameVFXSubsystem.cpp:266-310`。

**影响**

任何高频 Gameplay、网络重放、错误循环或压力测试都可持续增长受管记录和 Niagara 实例。Niagara Effect Type 可以限制部分系统行为，但不能替代请求入口的待加载队列、Owner/Tag 速率和优先级准入控制。

**修复建议**

- 在 `UModularGameVFXSettings` 增加每世界、每 Tag、每 Owner、Pending Load 和总实例预算。
- 使用 `Critical > Combat > Cosmetic > Ambient` 做拒绝、替换或最远实例淘汰；记录拒绝原因和计数。
- 对同一帧的大量请求分批生成，避免单帧集中创建组件。
- 保留 Niagara Effect Type 的距离、实例数和质量档控制，让入口预算与 Niagara 内部预算同时生效。

### PERF-P1-05 — Gameplay 投射物采用 Actor-per-projectile，并叠加 Tick、碰撞和点光

**证据**

- Frost、Fire、Arcane 投射物都是可 Tick Actor：`FrostMageGameplay.cpp:157-176`、`FireMageGameplay.cpp:23-50`、`ArcaneMageGameplay.cpp:13-48`。
- Arcane 一次引导生成 5 个 Actor：`ArcaneMageGameplay.cpp:57-67`；每枚每帧做贝塞尔计算、球形 Sweep 和 Transform 更新：`:37-47`。
- Fire 每帧做 Sweep、Transform 更新和点光强度更新：`FireMageGameplay.cpp:38-49`。
- Frost/Fire 投射物各带一个 Movable Point Light：`FrostMageGameplay.cpp:161`、`FireMageGameplay.cpp:28-29`；Fire Area 另带半径 530 cm 的点光：`FireMageGameplay.cpp:53-56`。
- Actor 命中或失效后频繁 `Destroy`，没有 Gameplay Actor 池：`FrostMageGameplay.cpp:168-176`、`FireMageGameplay.cpp:35-49`、`ArcaneMageGameplay.cpp:27-47`。

**影响**

弹量增加时，Actor/UObject 分配与销毁、GC 扫描、Tick 调度、碰撞查询、Primitive 更新和重叠动态光都会线性增加。阴影已关闭能降低成本，但不能消除每光源的光照 pass、剔除和渲染线程工作。

**修复建议**

- 复用轻量 Projectile Pool，或用集中式 Projectile Simulation/MovementComponent 管理弹道；休眠实例关闭 Tick。
- Gameplay 碰撞与 VFX 生命周期分开；只对近距、关键投射物保留精确 Sweep。
- 普通投射物使用 Emissive、Bloom 和少量共享角色/命中光。动态光按质量档、距离和屏幕占比设置硬上限。
- 把 `FVector::Size/Distance` 的仅比较用途改为平方距离，减少高频开方；先用距离/视锥粗筛，再做 Sweep/LOS。

### PERF-P1-06 — 单只傀儡使用 15 个独立静态网格组件并逐帧改 Transform

**证据**

- 构造时创建 13 个石块和 2 个眼球静态网格组件：`FrostMageGameplay.cpp:86-98`。
- 活动时每帧更新 BodyRoot，并对索引 5 之后的 8 个石块分别设置 Location 和 Rotation：`FrostMageGameplay.cpp:139-154`，关键更新在 `:151-152`。
- 选中状态每帧提交一个 64 段 Debug Circle：`FrostMageGameplay.cpp:154`。

**影响**

每只怪物至少带来 15 个 Primitive Component、相应 Draw Call/剔除工作和多次 Transform 脏标记。增加 AI 数量后，这部分会与怪物数直接相乘，并污染 VFX 性能测试基线。

**修复建议**

- 正式 Demo 使用单一 Skeletal Mesh；低复杂度方案可使用一个 HISM/ISM 加 Per Instance Custom Data。
- 动画只更新骨骼或实例矩阵，远距降至 15/10 Hz 并设置可见性 Tick 策略。
- 目标圈改用一个 Decal/Niagara Indicator，或至少降低段数并只在状态变化/固定低频更新。

### PERF-P1-07 — Catalog 列表没有虚拟化，并在每次输入变化时重建全部 Widget

**证据**

- `SearchChanged` 对每个文本变化立即调用 `FilterChanged`：`VFXShowcaseWidget.cpp:459-480`。
- 随后清空类别和条目容器，再为每个可见条目创建 `UVFXShowcaseEntryItem`、Button、TextBlock 和 Slot：`VFXShowcaseWidget.cpp:422-457`。
- `GetEntries` 返回完整结构体副本数组，过滤后排序；比较器反复执行 `FText::ToString()`：`VFXShowcaseController.cpp:241-254`。
- 搜索匹配对每行重复生成 DisplayName、Tag、Enum 和中文标签字符串：`Plugins/ModularGameVFXShowcase/Source/ModularGameVFXShowcase/Private/VFXShowcaseTypes.cpp:18-31`。

**影响**

当前 36 个条目问题有限；设计目标为数百、数千 Niagara System 时，每输入一个字符都会产生大量 UObject/Slate Widget、字符串转换、排序和布局工作。`ScrollBox + 全量子控件` 不会像 `ListView/TileView` 那样复用可见行。

**修复建议**

- 使用 `UListView/UTileView` 或原生 `SListView`，只创建视口内行并复用 Entry Widget。
- 搜索输入去抖 100–200 ms；拼音/中文等索引在 Catalog 刷新时预计算。
- Controller 缓存过滤后的 Tag/索引数组；只有筛选、排序、评审或 Catalog 变化时重新计算。
- 排序键、规范化搜索文本和枚举显示字符串写入轻量 ViewModel，比较器只比较缓存值。

### PERF-P2-08 — Showcase 标签每帧执行投影和 Slate 预布局

**证据**

- `AVFXShowcaseEnvironment::Tick` 每帧获取 PlayerController、Viewport Scale 和尺寸，并遍历全部 HUD Label：`Plugins/ModularGameVFXShowcase/Source/ModularGameVFXShowcase/Private/VFXShowcaseEnvironment.cpp:162-186`。
- 每个可见候选都调用 `ProjectWorldLocationToScreen`，并对其 Slate Widget 主动执行 `SlatePrepass()`：`:172-182`。

**影响**

文字没有逐帧变化，Desired Size 也基本稳定；重复 Prepass 和投影属于不必要的固定成本。由于这个世界用于性能验收，它会直接抬高并扰动测量基线。

**修复建议**

- 创建或文字/分辨率/DPI 变化时缓存 Desired Size。
- 以 5–10 Hz 更新可见性，或只在镜头 Transform、Viewport 和 UI 面板尺寸发生变化时更新。
- 缓存 PlayerController；无 Label、窗口后台或使用战斗世界时关闭 Actor Tick。

### PERF-P2-09 — AI、治疗跟随与奥术引导在逐帧路径中重复查询和射线

**证据**

- 每只怪物每帧 `GetPlayerCharacter`，游荡分支又 `GetPlayerPawn`，并执行多次非平方距离计算；攻击条件再做 LOS：`FrostMageGameplay.cpp:116-154`。
- 跟随玩家的治疗地面效果在 6 秒生命周期内每帧向下 Line Trace：`FrostMageGameplay.cpp:196-210`。
- 玩家每帧调用 `TickArcaneChannel`，其中每帧验证距离与 `LineOfSightTo`：`FrostMageGameplay.cpp:326-337`、`ArcaneMageGameplay.cpp:57-68`。
- 每枚 Arcane 飞弹每帧 Sphere Sweep；一次施法最多并发 5 枚：`ArcaneMageGameplay.cpp:42-47`、`:59-67`。
- Fire Area 虽已设为 0.05 秒 Tick，但伤害路径遍历所有怪物并逐个做遮挡查询：`FireMageGameplay.cpp:53-74`。

**影响**

单怪演示下通常可接受；怪物、持续区域或并发飞弹增加后，Controller/Pawn 查询、碰撞场景查询和开方会明显放大。

**修复建议**

- AI 缓存玩家弱指针；移动保持逐帧，目标选择、距离分档和 LOS 以 5–10 Hz Service 更新。
- 治疗地面只有玩家跨过一定 XY/Z 阈值或地面单元变化时 Trace；平面区域直接继承角色 XY。
- Arcane 的目标有效性/LOS 在发射节拍或 10 Hz 检查；碰撞先用粗距离/Overlap 缩小候选。
- Area 使用 Sphere Overlap 或维护在场怪物集合，只对候选做必要遮挡测试。

### PERF-P2-10 — Demo Canvas HUD 每帧产生字符串、FText 和绘制提交

**证据**

- `AFMFrostHUD::DrawHUD` 每帧创建多组 `FString::Printf`、`FString::FromInt`、`FText::FromString` 和 `FCanvasTextItem`：`FrostMageGameplay.cpp:447-489`。
- 8 个技能卡每帧绘制背景/冷却矩形及两段文字：`:461-475`。
- 状态文字显示期间，`ChineseStatus` 每帧依次执行 17 组 `ReplaceInline`：`FrostMageGameplay.cpp:45-61`、`:488`。

**影响**

该 HUD 是 Demo 限定，但会持续产生小对象/字符串分配、字体布局和 Canvas Draw Item，降低用同一关卡测量 VFX CPU 与帧时间时的可信度。

**修复建议**

- 状态变化时完成本地化并缓存 `FText`；冷却显示以 10 Hz 更新。
- 用 Retainer/Invalidation 支持的 UMG HUD，静态标签只建一次；数字变化只修改对应文本。
- 性能采样提供“隐藏 HUD”基线，并分别记录显示/隐藏两组数据。

### PERF-P2-11 — Core 高频播放存在重复结构复制，清理扫描始终开启

**证据**

- 每次播放复制完整参数映射：`ModularGameVFXSubsystem.cpp:111-126`。
- 解析 Tag 后把完整 `FVFXCatalogEntry` 复制进运行时记录，其中含两个 `FText` 和默认参数 Maps：`:236-253`、`VFXTypes.h:75-135`。
- 生成时再次复制默认参数并 Append 请求参数：`ModularGameVFXSubsystem.cpp:377-379`。
- Catalog/资源等待列表使用线性 `AddUnique`：`:153`、`:231`、`:268-270`。GUID 已由本次请求唯一生成，重复检查通常没有收益。
- Subsystem 初始化后无论是否存在实例，都以 0.25 秒周期扫描整个 `RuntimeInstances`：`:13-20`、`:758-774`。

**影响**

单次播放成本不高；在一帧内生成几十到几百个短生命周期效果时，Map/文本复制、容器扩容与线性查重会叠加。空世界仍保留 Cleanup Ticker。

**修复建议**

- 运行时记录保存 Catalog 索引/Tag 和生成所需的轻量字段；Catalog 已由 Subsystem 强引用，可安全保持条目来源。
- 默认参数只构建一次最终集合；高频标准参数可采用固定结构或小型内联数组，扩展参数再落入 TMap。
- 对天然唯一的句柄直接 `Add`；确需去重的等待集合改 `TSet` 或并行 Array+Set。
- 仅当第一个运行时实例出现时挂载 Cleanup Ticker，最后一个实例结束后移除；进一步用 Owner/World EndPlay 事件替代大部分轮询。

### PERF-P2-12 — Showcase 多处线性查找组合为二次复杂度

**证据**

- `FindRow` 线性扫描 `AllEntries`：`VFXShowcaseController.cpp:238-239`，并在选择、参数、评审、保存等路径重复调用。
- `UVFXShowcaseProfile::Find` 线性扫描 Profile：`VFXShowcaseTypes.cpp:33-36`；Catalog 刷新对每个条目至少调用 `ResolveProfile`，还单独再次调用 `Profile->Find`：`VFXShowcaseController.cpp:189-193`。
- `UpdateReviewRows` 对每个条目线性搜索 Reviews：`VFXShowcaseController.cpp:547-558`，形成 O(Entries × Reviews)。
- `SaveReviews` 对每条 Review 调用线性 `FindRow`：`:796-805`；`LoadReviews` 逐条用 `ContainsByPredicate` 查重：`:816-834`。
- `GetCategories` 对每条目遍历全部类别：`:256-267`；`GetElements` 使用 `AddUnique`：`:269-273`。

**影响**

36 条目下不构成主要瓶颈；当库达到数百或数千条目时，刷新、载入评审、保存和筛选会出现明显二次增长，并与 PERF-P0-01 的同步加载叠加。

**修复建议**

- 为 Showcase 建立 `Tag → EntryIndex`、`Tag → ReviewIndex`、`Tag → ProfileIndex` 三个 TMap；Catalog 变更时一次重建。
- 分类/元素进度在 Entry 或 Review 变化时增量更新。
- Blueprint API 如需返回副本可保留，但 C++ Widget 内部使用只读视图、Tag 数组或指针，避免复制整行。

### PERF-P3-13 — 生产构建需排除验收模块，并核实图形配置

**证据**

- Demo 和 Showcase 均声明为 Runtime 模块；虽然 `.uplugin` 默认关闭，但当前宿主工程明确启用二者。
- Showcase Runtime 依赖 AssetRegistry、Json、UMG、Slate、Niagara 调试/资产检查代码；这些功能不是核心播放 API 必需。
- `Config/DefaultEngine.ini:28-37` 同时配置多个 D3D 着色目标并选择 `RayTracingMode=Full`。

**影响**

核心插件跨项目打包时，如果宿主沿用当前启用项和配置，可能携带不需要的 Demo/Showcase 模块、资产和着色变体。Full Ray Tracing 是否实际产生运行成本还取决于 RHI、项目渲染开关和目标硬件，因此本项只记录为待确认风险。

**修复建议**

- 发布包把 `ModularGameVFX`、`ModularGameVFXDemo`、`ModularGameVFXShowcase` 分别验证；生产宿主只启用 Core。
- 把资产哈希、完整依赖审计和验收写盘逻辑拆到 Editor/Developer 模块；如需 packaged showcase，仅保留轻量 Runtime 浏览与播放部分。
- 针对实际交付平台精简 shader formats；对 Ray Tracing On/Off 分别 Profile，再决定默认项。

## 已确认的良好实现

以下实现应保留，并作为后续优化的基础：

- Catalog 建立 `FGameplayTag → EntryIndex` 缓存，运行时查找为平均 O(1)：`Plugins/ModularGameVFX/Source/ModularGameVFX/Private/VFXCatalog.cpp:5-36`。
- 默认 Catalog 和 Niagara System 使用 Streamable Manager 异步加载：`ModularGameVFXSubsystem.cpp:160-181`、`:266-310`。
- 同一 Niagara 软路径的并发请求合并到一个 `FStreamableHandle`：`ModularGameVFXSubsystem.cpp:266-281`。
- Dedicated Server 在分配实例前被拒绝：`:82-95`。
- Niagara 生成前先计算 Catalog 默认 Transform，参数在 Activate 前写入，并使用 Catalog Pooling Method：`:313-388`。
- Owner、World、AttachComponent 与 Niagara Component 使用弱引用，并在 Finish/Stop/周期清理中移除句柄：`:714-774`。
- 预加载按 Tag/资产路径维护引用计数，多个 Tag 可共享同一资产句柄：`:593-701`。
- Demo 的 Fire Area 已使用 0.05 秒 Tick，而不是无条件 60/120 Hz：`Plugins/ModularGameVFXDemo/Source/ModularGameVFXDemo/Private/FireMageGameplay.cpp:53`。
- Demo 动态点光均关闭阴影；这降低了最昂贵的一部分光照成本，但仍需要数量和质量档预算。

## 修复顺序

### P0：先消除结构性阻塞

1. 把 Showcase 初始化改为一次异步 Catalog 元数据加载。
2. 把递归依赖哈希和系统内省迁移到 Editor/Commandlet 生成的 manifest；运行时按需读取。
3. 把 Demo 的 12 处直接 Niagara 生成全部迁移到 Blueprint Library，加入进入战斗时预加载和退出时释放。
4. 增加 CI 规则，确保业务代码不再产生第二条生成路径。

### P1：控制并发与逐帧增长

1. 在 Subsystem 增加每世界/Tag/Owner/Priority 预算、拒绝计数和分帧生成队列。
2. 用事件计数替代 Showcase 的 `TObjectIterator` 和每帧 Runtime Map 扫描。
3. 池化 Projectile Actor，限制动态光数量；重做傀儡为单骨骼网格或实例网格。
4. Showcase 条目列表改为虚拟化并加入筛选去抖。

### P2：降低固定成本并提高测量可信度

1. AI、LOS、地面 Trace、HUD 数字和环境 Label 降频/事件化。
2. 为 Showcase 的 Entry/Profile/Review 建立 Tag 索引。
3. 减少 Core 的 Catalog/参数复制，并按需启停清理 Ticker。
4. 提供无 HUD、无调试圈、无 Showcase 遥测扫描的纯 VFX 基线模式。

## 实测验证计划

修复后应在同一机器、分辨率、构建配置和镜头下保存原始 Trace，至少覆盖以下场景：

| 场景 | 负载 | 必采数据 |
| --- | --- | --- |
| 冷启动 | 首次进入 Demo/Showcase，磁盘缓存冷/热各一次 | 首帧前总耗时、最长 Game Thread 帧、Sync Load 次数、加载包数、Working Set |
| 单效果 | 每类代表效果各 1 个 | Game/Render/RHI Thread、GPU、Emitter/Particle、Draw Call、Overdraw、纹理驻留 |
| 常规战斗 | 1 个 AI，冰/火/奥术连续施法 | Projectile Tick、Collision Query、动态光、GC Alloc/Mark、帧时间 P50/P95/P99 |
| 多 AI | 10、30 个怪物追击与反击 | AI/LOS/Movement、Primitive 数、Shadow/Light cost、Game Thread 扩展曲线 |
| 批量 VFX | 10、30、50 个普通 Hit；持续 Area 叠加；Projectile 批量飞行 | Subsystem Dispatch、Pending/Active/Rejected、Niagara Tick、GPU、Overdraw |
| 上限测试 | Showcase 100 实例 | 单帧生成峰值、内存峰值、回收时间、Pool 命中、GC 峰值；只用于找上限 |
| 质量档 | High、Medium、Low、VRMobile | 每档实例/粒子/光/材质降级是否真实生效，画面可读性与性能差异 |

建议工具与输出：

- Unreal Insights：CPU、Load Time、Asset Loading、Object/Memory、Counters、File Activity。
- Niagara Debugger / Niagara Overview：System/Emitter 实例、CPU/GPU Sim、粒子数、Tick 时间、Scalability/Cull 原因。
- `profilegpu`、GPU Visualizer、Shader Complexity、Quad Overdraw、Light Complexity。
- `stat game`、`stat slate`、`stat niagara`、`stat gpu`、`memreport -full`。
- 每次报告记录硬件、驱动、平台、分辨率、RHI、是否开启 Ray Tracing、质量档、构建 Commit/资产版本、测试负载和采样时长。

## 验收判定

本轮只能判定“代码性能审查已完成，存在待整改项”。以下结论目前不能成立：

- “性能良好”或“达到电影级性能预算”；
- “VRMobile 已通过”；
- “100 实例压力测试通过”；
- “Production Ready”。

只有在 P0/P1 主要问题整改后，按上表取得目标设备的 CPU、GPU、实例、粒子、Overdraw、纹理内存和稳定帧时间证据，才能填写插件生产规范中的 Performance Approved。
