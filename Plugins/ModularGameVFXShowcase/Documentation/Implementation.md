# ModularGameVFXShowcase 实施与验收说明

本模块用于独立浏览 Catalog、调用正式 VFX Manager、调参、切换测试条件、运行压力测试并保存人工评审。它只依赖核心插件与引擎基础能力；删除 Showcase 不应影响核心运行库。

[用户原始执行规范](ModularGameVFX_Showcase_Execution_Spec.md) 按字节完整保留。原文 SHA256：`cf95710b7d078acb1cd15ae14b4ed7b1114231b88d97909407d9be8bd64e3aa0`。

## 工程入口与资产

模块名和挂载点分别为 `ModularGameVFXShowcase` 与 `/ModularGameVFXShowcase`。准备的 Python 创建脚本使用 Unreal 原生 Blueprint/WidgetBlueprint/DataAsset 工厂与地图保存 API，实际创建以下二进制资产；不能以脚本存在代替资产已生成。

| 目录 | 资产 |
| --- | --- |
| Maps | `L_VFX_Showcase` |
| Blueprints | `BP_VFX_ShowcaseController`、`BP_VFX_TestOrigin`、`BP_VFX_TestTarget`、`BP_VFX_TestCharacter`、`BP_VFX_TestProjectileTarget`、`BP_VFX_ShowcaseEnvironment` |
| UI | `WBP_VFX_Showcase`、`WBP_VFX_CategoryMenu`、`WBP_VFX_EntryList`、`WBP_VFX_EntryItem`、`WBP_VFX_Inspector`、`WBP_VFX_ParameterPanel`、`WBP_VFX_PerformancePanel`、`WBP_VFX_ReviewPanel` |
| Profiles | `DA_VFXShowcase_DefaultProfiles`，初始包含 13 个 curated Catalog Tag 的 Preview 条目 |

BP 和 WBP 是对应原生 Blueprintable 类的可编辑子类。UI 完整内容由原生 Widget 构建，wrapper 不是伪造的截图或无父类占位文件；主地图通过 `WidgetClass` 指向 `WBP_VFX_Showcase`。后续可以在 wrapper 上扩展界面。

Profile 是集合型 Data Asset，按 Tag 存储 PreviewMode、距离、时长、TargetType、默认镜头、LoopAllowed 与背景；正式 Runtime Catalog 不增加测试字段。初始 13 项来自实际迁移清单中 `formal=true` 的 NiagaraSystem，并要求这些 Tag 在实际核心 Catalog 中存在。早期 Prototype 不因文件存在而自动作为已审核效果展示；将新资源正式注册 Catalog 后，菜单由数据动态发现。

## 真实场景结构

地图保存 Controller 与 Environment 两个 Blueprint 实例，并绑定实际核心 Catalog、Preview Profile 和主 UI。地图显式使用引擎 `GameModeBase`，不继承宿主项目的 Demo GameMode 或角色。

Environment 原生构造相机、地面网格、Surface 测试墙、环境区和与展示区分离的压力测试区。BeginPlay 创建 Origin、Target、ProjectileTarget 与通用角色代理，设置相机观察场景。其余四个测试 Actor wrapper 保留为可复用资产，不在地图中额外复制一套由环境自动生成的对象。

角色是独立程序化代理，包含 `root`、`pelvis`、`spine`、`head`、`hand_l`、`hand_r`、`foot_l`、`foot_r`、`weapon` 命名组件。它不是已经制作并验收的 SkeletalMesh/动画资源。程序化轨迹可用于 Trail、Dash、Jump 等测试；真实角色骨骼、动画重定向和标准 Socket 动画表现仍须在目标项目核验。

Neutral、Bright、Dark、Complex 是测试光照/几何组合。Stone、Metal、Wood 等 Surface 入口目前用于代理表面与方向检查；不能据此声称完成真实材质、Physical Material 或所有贴花场景的认证。Screen 模式是独立相机附着预览层，不应被宣称为任意 UMG 材质或 VR 双眼显示已经验收。

## 数据与播放路径

分类、Element、搜索、评审状态和排序均通过 Controller 查询 Catalog DTO；Widget 不维护每个特效的硬编码按钮，也不直接调用 Niagara Spawn。正式播放路径保持：

```text
Widget 的 GameplayTag
→ Showcase Controller
→ ModularGameVFXBlueprintLibrary
→ 现有 VFX Manager/Catalog
→ Niagara
```

Preview Profile 可覆盖分类推导的播放方式。Cast/Status 使用附着代理，Projectile 沿测试通道，Area/Indicator 使用地面，其他模式使用各自测试区域。参数通过现有 Manager 传递；Reset 使用 Catalog 默认参数。Quality 影响真实 Niagara 配置，仍须用实际画面验证各档视觉降级是否合理。

五档质量索引统一为 `0=VRMobile`、`1=Low`、`2=Medium`、`3=High`（UE Epic 基础档）、`4=Cinematic`，与四个 Effect Type 的五个质量 mask 对齐。Preview Profile 本身只保存展示方式与时长距离，不另造一套质量索引。

Single、Compare、Stress 和组合测试的调用由 Controller 管理。压力测试支持 1～100 的边界，选择菜单为 1/5/10/20/50/100。播放、Replay、Stop、立即停止、Loop 与 Auto Preview 的生命周期必须在 Game/PIE 环境检验，不能只凭 Editor world 中创建了 Actor 判定通过。

## 评审门禁与证据

Visual、Gameplay、Performance 是人工主评审项；Naming、Catalog、Dependency 是技术准入项。只有六项均有与当前资产版本匹配的真实通过证据，才能标记 ProductionReady。快速三个 PASS 不能绕过已有六项生产门禁。

Review 保存的是独立评审数据，不修改正式 Niagara 资产。SaveReviews 是明确的持久化操作；模板评分与自动化测试中的合成证据不是用户真实评审。未取得实机 Profile 时保留“尚待实机 Profile”，CPU/GPU/粒子成本不能填假数。

## 构建、生成与检查

主任务统一构建并启用新模块后，在新的 Unreal Editor Python 进程依次运行：

1. `Saved/ShowcaseSetup/create_showcase_unreal.py`：校验真实类和 Catalog，创建/复用 6 个 BP、8 个 WBP、Profile、地图，逐阶段写 `setup_result.json`。
2. `Saved/ShowcaseSetup/verify_showcase_unreal.py`：实际加载 uasset 与生成类，在临时 Controller 上执行筛选和评审门禁行为测试，检查依赖与资产文件未保存，写 `verification.json`。
3. 在 Game/PIE 或原生 Automation 中执行 BeginPlay、Manager 播放/停止、压力边界与清理测试。实际结果以对应运行日志为准。
4. 在可见 Editor/目标设备中测试 UI 点击、布局、参数、真实 Niagara 画面、不同质量/背景与 VR 双眼；保存真实人工评审和 Profile 证据。

生成脚本重复运行时复用已有正确父类的 wrapper，保留已有 Profile 条目及自定义设置，只追加缺失 Tag；更新本脚本管理的场景绑定，不删除用户额外场景对象。已有同名资产或场景对象父类不匹配时失败并记录，避免静默覆盖。

两个脚本只保存新 Showcase 插件中明确负责的资产与自身报告，不保存核心/旧 Demo 内容，不使用 SaveAll，不伪造 `.uasset`。结构验收使用新建临时 Actor，合成六门禁测试不调用 SaveReviews，完成后销毁临时 Actor。

## 当前证据的解释

- 脚本语法通过只说明脚本可解析；实际生成成功由 `setup_result.json` 的 `success=true` 及真实资产支持。
- `verification.json` 的 `structural_checks_passed` 仅表示实际完成的结构/数据/门禁检查通过；它不等于 `production_ready`。
- NullRHI 可以用于反射、资源依赖和部分 Game world 生命周期验证。UI 交互、渲染效果、GPU 时间、Overdraw、目标设备与 VR 双眼必须分别记录真实测试；未测字段保持 NotTested/NotRun。
- 本次迁移旧位置及兼容 redirector 的清理由主迁移流程负责；Showcase 不恢复旧路径或反向依赖旧 Demo 内容。
