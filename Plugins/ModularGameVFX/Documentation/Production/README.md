# ModularGameVFX 生产规范入口

本目录把用户提供的单个效果工作流与批量生产验收规范落成可复用登记格式。它不创建 Niagara 资产、不扩大生产范围，也不代表当前库已通过生产验收。

## 规范与文件

- [单个 VFX 自动创建工作流原文](../ModularGameVFX_Single_VFX_Workflow.md)
- [批量资产生产与质量验收规范原文](../ModularGameVFX_Batch_Production_Acceptance.md)
- [当前分类与操作规范](../ModularGameVFX_Taxonomy_Operations.md)
- [标准资产模板原文](../ModularGameVFX_Standard_Asset_Templates.md)
- [需求登记模板](Templates/Production_Intake.template.json)
- [验收登记模板](Templates/Production_Acceptance.template.json)
- [现有 36 个系统的初始质量登记](VFX_Quality_Register.json)
- [原文来源与完整性记录](Source_Provenance.json)

规范原文保留用户输入，包括说明性示例；下列适用规则解决示例与当前工程实现之间的差异。

## 范围与当前实现

1. 只有用户实际要求的效果、元素、数量和平台进入生产范围。规范中的 12 元素、18 项矩阵、Fire/Water 等示例，以及未来首批建设顺序，均不构成创建整套资产的授权。未明确指定的项目保持空白或注明推断来源，不把模板示例加入生产队列。
2. 物理业务目录继续按主要职责使用 `01_Cast` 至 `15_UI`；共享积木进入 `00_Core`。批量规范中 `Fire/Cast` 等树形示例表示逻辑 Pack 视图，不建立元素顶层业务目录。
3. 使用现有 `EVFXCategory`、`EVFXElement`、`EVFXForm`、`EVFXContext`、`EVFXLifecycle` 和 `EVFXPriority`。原文 `Form=HeavyImpact` 是视觉描述示例；当前 Form 枚举没有此值，应按主体选择 `Explosion`、`Burst` 或 `Shockwave`，把 HeavyImpact 记录在描述或名称。`Holy` 的当前 Element 使用 `Light`。确需扩展枚举时作为独立实现决定处理，不能把未知字符串写入 Catalog。
4. Catalog 中 Priority 的序列化字段仍为 `PerformanceClass`，Python 名为 `performance_class`；编辑器显示为 Priority。Catalog 使用软引用。正式业务经现有 `UModularGameVFXBlueprintLibrary` 调用，不另造 Manager，也不把当前 Demo 的直接引用误写成已经完成统一入口验收。
5. 当前迁移由独立流程执行。登记中的目标路径来自迁移清单，单独注明尚未在该登记中重新验证；路径存在、Niagara 编译成功和 Catalog 注册都不能证明视觉、Gameplay 或实机性能合格。

## 使用方式

复制需求模板为本次工作记录，先填写用户范围及需求来源，再完成分类、复用检索和设计。批量工作先列实际授权矩阵与共享资产，逐项引用单个效果需求记录；不为颜色、比例或 Spawn Count 差异机械复制 System。

复用检索至少覆盖完整 System、Emitter、Niagara Module、Master Material、Material Instance、Material Function、Texture、Mesh、Curve、Effect Type。记录查询范围、匹配资产、证据和决策；未检索不能宣称没有可复用资源。60%～85% 是参考目标，实际复用率必须说明分子、分母和计量口径。

每个效果设计明确一句话视觉核心、方向、时间轴、Near/Mid/Far 层次以及每个 Emitter 的职责。根据需要选择职责层，不要求每个效果都具备所有层。User Parameters 采用可复用含义；参数化复用优先于复制系统或 Master Material。

实施并测试后填写验收模板，链接真实证据和版本。模板是结构化登记格式，不是自动完成验收的工具；没有证据的项保持 `NotAssessed`，测量值和评分保持 JSON `null`。`NotApplicable` 必须说明理由；正式六项准入门禁不能以 `NotApplicable` 代替通过。

## ProductionReady 的六项门禁

只有以下六项均为 `Approved`，且分别关联真实证据、验收人或执行工具、验收时间和对应资产版本，才能标记 `ProductionReady`：

| 门禁 | 最少需要的证据 |
| --- | --- |
| Visual Approved | 实际画面或录像及评审记录：视觉中心、节奏、层次、元素辨识、结束方式、包内一致性。 |
| Gameplay Approved | 实际调用与场景检查：命中点、范围、方向、敌我辨识和遮挡；Tag → Catalog → Manager 调用及生命周期符合用途。 |
| Performance Approved | 目标设备实测 Profile：硬件、平台、分辨率、质量档、构建版本、测试负载、预算、CPU/GPU/实例/粒子/Overdraw/纹理内存数据与结论。 |
| Naming Approved | 实际资产类型前缀、职责目录、Emitter 命名、Tag 规范、重复资产检查结果。 |
| Catalog Registered | 实际 Catalog 条目、软引用、强类型 Metadata、Tag 唯一性和对应资源加载/调用检查。 |
| Dependency Approved | 实际硬软依赖扫描和跨项目依赖边界验证；不得依赖主项目 `/Game` 或 Demo/宿主脚本。 |

编译零错误只是技术证据的一部分。配置 Effect Type、Pooling、Fixed Bounds、实例限制与质量档只证明性能设计存在，不能替代 Profile。未实测时必须写“尚待实机 Profile”，不能写“性能良好”“性能达标”或“AAA 正式生产级已验收”。

批量负载计划覆盖单独播放、10 个、30 个、50 个普通 Hit、多个 HeavyImpact、持续 Area 叠加和 Projectile 批量飞行；不适用场景给出理由。重要效果覆盖 High、Medium、Low、VRMobile，必要时增加 Cinematic。Showcase 与性能模板统一使用五档索引：0=VRMobile、1=Low、2=Medium、3=High（采用 UE Epic 基础档）、4=Cinematic。档位配置不证明实际 VR 设备、双眼渲染或移动平台已经通过测试。

视觉建议评分权重为：视觉中心 15、打击/力量感 15、层次感 10、时间节奏 10、元素辨识 10、Gameplay 可读性 15、复用率 10、性能设计 10、命名规范 5，总分 100。仅在各项具有评审证据时计算总分。90～100 对应 Hero/Excellent、80～89 对应 Production Ready 建议区间、70～79 需优化、低于 70 不进入正式库；评分等级不能代替六项门禁。

性能风险单独登记 Overdraw、Particle Count、Material Complexity、Instance Cost、Texture Memory、Scalability、Pooling、Cull。大量全屏透明粒子、缺少 Cull/Instance Limit、高频效果无 Pooling、大量粒子光或 CPU Collision 应进入失败/整改记录，不能仅用视觉分数抵消。

## 状态与本次迁移

- `Prototype`：初始或实验效果，尚未完成评审。
- `Review`：等待完整评审的候选效果；不等于 ProductionReady。
- `Optimized`：有明确优化与验证记录，仍需逐项通过正式门禁。
- `ProductionReady`：六项门禁均有真实通过证据。
- `Deprecated`：未来确实淘汰的逻辑效果，记录替代 VFX Tag、原因和升级说明。

初始质量登记严格按当前迁移清单映射：`formal=true` 的 13 个 System 为 `Review`，其余 23 个为 `Prototype`。所有评分、实机测量和六项验收结论均未填实测值。此映射是待评审清单初始化，不能由 formal 字段推导生产质量。

用户最新要求移除本次迁移的旧路径。此次只清理已备份的旧迁移位置、兼容资产 redirector 及旧 CoreRedirect 配置；映射留在备份证据中。未来的 `Deprecated` 流程不要求保留此次旧迁移目录或兼容入口，也不扩大为删除与本次迁移无关的资产。
