# ModularGameVFX 1.0.0 VFX 效果目录清单

正式资源根目录：`/ModularGameVFX/ModularGameVFXLibrary`

资产迁移状态：**已迁移（migration_complete.json）；旧路径已移除**。

历史来源路径只用于追溯迁移映射，不是可用的兼容入口。正式使用标准目标路径和 GameplayTag。13 个 Catalog 系统仍为 Review，23 个旧库系统为 Prototype，ProductionReady 为 0；视觉与性能尚待人工评审和实机 Profile。旧库条目的 ET_VFX_* 表示建议策略，只有 13 个 Catalog 系统已实际绑定四个标准 Effect Type。

权威迁移映射：`F:/game/MythicVFXLab/Saved/TaxonomyMigration/manifest.json`，共 1059 个真实资产。只有检测到 `migration_complete.json` 后，清单才会标记迁移完成。

统一管理关系：**Folder + GameplayTag + Catalog Metadata + Niagara Effect Type + UModularGameVFXSubsystem**。

业务只能使用 GameplayTag 通过 `UModularGameVFXBlueprintLibrary` 调用；目录表示资源位置，Catalog 负责 Tag 到软 Niagara 引用的映射，Effect Type 负责 Cull、Scalability、实例数和性能预算。

## 一级分类规范

| 一级目录 | 分类名称 | 包含效果 | 归类规则 | 虚幻资产根路径 |
| --- | --- | --- | --- | --- |
| 00_Core | 基础视觉原子 | 17 个基础/共享资源目录 + EffectTypes、Catalog 两个管理目录；详见 Core原子表。 | 只存放可组合原子资源，不作为普通业务效果分类。 | /ModularGameVFX/ModularGameVFXLibrary/00_Core |
| 01_Cast | 施法 | 施法、蓄力、引导、释放 | 技能前摇、手部聚能、读条与释放瞬间。 | /ModularGameVFX/ModularGameVFXLibrary/01_Cast |
| 02_Projectile | 投射物 | 火球、箭矢、子弹、剑气、飞行体 | 具有弹道或明确飞行阶段的主体。 | /ModularGameVFX/ModularGameVFXLibrary/02_Projectile |
| 03_Trail | 拖尾 | 刀光、剑光、武器拖尾、冲刺残影 | 随运动持续生成的条带或残影。 | /ModularGameVFX/ModularGameVFXLibrary/03_Trail |
| 04_Beam | 光束 | 激光、雷链、治疗光束、牵引线 | 连续或瞬时连接起点与终点的效果。 | /ModularGameVFX/ModularGameVFXLibrary/04_Beam |
| 05_Impact | 命中 | 命中、受击、爆炸、冲击波、飞溅 | 接触目标或表面后的瞬时反馈。 | /ModularGameVFX/ModularGameVFXLibrary/05_Impact |
| 06_Area | 区域 | AOE、法阵、地面区域、持续区域、能量场 | 以空间范围为主体的持续或爆发效果。 | /ModularGameVFX/ModularGameVFXLibrary/06_Area |
| 07_Status | 状态 | 护盾、Buff、Debuff、Aura、持续治疗、隐身 | 附着于角色或区域并表达持续状态。 | /ModularGameVFX/ModularGameVFXLibrary/07_Status |
| 08_Space | 空间 | 传送、Portal、闪现、空间裂隙、黑洞、Warp | 改变或表达空间关系的效果。 | /ModularGameVFX/ModularGameVFXLibrary/08_Space |
| 09_Lifecycle | 生命周期 | 出生、召唤、变身、复活、死亡、消散、Dissolve | 实体生命周期节点的视觉反馈。 | /ModularGameVFX/ModularGameVFXLibrary/09_Lifecycle |
| 10_Character | 角色辅助 | 角色动作、武器附着、脚步、跳跃、落地、冲刺 | 依附角色动作和装备的辅助效果。 | /ModularGameVFX/ModularGameVFXLibrary/10_Character |
| 11_World | 世界环境 | 天气、雨雪、雾、风、水、环境烟尘、自然氛围 | 关卡与环境中的世界级效果。 | /ModularGameVFX/ModularGameVFXLibrary/11_World |
| 12_Surface | 表面残留 | Decal、弹孔、焦痕、裂纹、冰霜、脚印、魔法残留 | 贴附表面并保留一段时间的痕迹。 | /ModularGameVFX/ModularGameVFXLibrary/12_Surface |
| 13_Destruction | 破坏 | 碎裂、崩塌、爆炸残骸、Chaos、物理破坏 | 破坏与物理碎片反馈。 | /ModularGameVFX/ModularGameVFXLibrary/13_Destruction |
| 14_Indicator | 指示器 | 技能范围、攻击预警、锁定、瞄准、落点、Boss 红圈 | 在伤害或动作发生前传达玩法信息。 | /ModularGameVFX/ModularGameVFXLibrary/14_Indicator |
| 15_UI | 界面反馈 | UI 动画、屏幕反馈、受伤提示、低血量、PostProcess、Camera 反馈 | 屏幕空间、镜头和后期处理反馈。 | /ModularGameVFX/ModularGameVFXLibrary/15_UI |

## 00_Core 基础视觉原子、共享资源与管理目录

| Core 子目录 | 目录角色 | 效果名称 | 效果描述 | 虚幻资产根路径 | 业务调用规则 |
| --- | --- | --- | --- | --- | --- |
| Energy | 基础视觉原子 | 能量原子 | 通用能量核、能量流、脉冲与能量场 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Energy | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Fire | 基础视觉原子 | 火焰原子 | Flipbook 火苗、火舌、余烬与热扰动 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Fire | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Smoke | 基础视觉原子 | 烟雾原子 | 柔烟、浓烟、体积烟与消散烟 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Smoke | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Sparks | 基础视觉原子 | 火花原子 | 魔法火花、金属火花、星点与爆点 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Sparks | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Dust | 基础视觉原子 | 灰尘原子 | 落尘、地面扬尘、碎屑尘与环境尘 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Dust | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Lightning | 基础视觉原子 | 电弧原子 | 闪电分支、电弧与噪声驱动放电 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Lightning | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Glow | 基础视觉原子 | 辉光原子 | 光点、光斑、光晕与核心辉光 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Glow | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Shockwave | 基础视觉原子 | 冲击波原子 | 环形冲击波、折射波与速度线 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Shockwave | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Debris | 基础视觉原子 | 碎片原子 | 通用轻量碎片、冰屑、石屑与余烬碎片 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Debris | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Noise | 基础视觉原子 | 噪声原子 | 流动、溶解、扭曲、法线与遮罩 Noise | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Noise | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Ribbon | 基础视觉原子 | 条带原子 | Niagara Ribbon 基础材质、纹理与模块 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Ribbon | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Meshes | 基础视觉原子 | 基础网格体（Mesh） | 弹头、碎片、环、平面、圆柱及低模代理 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Meshes | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Materials | 基础视觉原子 | 通用材质 | 跨效果共享的主材质与标准实例模板 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Materials | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| MaterialFunctions | 共享技术资源 | 材质函数 | 跨效果共享的材质函数与节点封装 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/MaterialFunctions | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Textures | 基础视觉原子 | 通用纹理 | 跨效果共享的遮罩、序列帧、法线和颜色查找纹理 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Textures | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| NiagaraModules | 基础视觉原子 | Niagara 模块 | 共享 Spawn、Update、Force、Renderer 与参数模块 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/NiagaraModules | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| Curves | 共享技术资源 | 曲线资源 | 跨效果共享的颜色、尺寸、透明度与运动曲线 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Curves | 由业务效果组合使用；完整技能按功能归入 01_Cast 至 15_UI。 |
| EffectTypes | 管理目录 | Niagara Effect Type | 统一 Cull、Scalability、实例数和性能预算 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/EffectTypes | 供 Catalog 和性能治理使用，不作为可直接播放的完整业务效果。 |
| Catalog | 管理目录 | VFX Catalog | GameplayTag 到软 Niagara 引用及多维元数据的权威映射 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Catalog | 供 Catalog 和性能治理使用，不作为可直接播放的完整业务效果。 |

## 多维 Catalog Metadata

| 维度 | 用途 | 推荐值 |
| --- | --- | --- |
| Category | 主功能分类 | Cast / Projectile / Trail / Beam / Impact / Area / Status / Space / Lifecycle / Character / World / Surface / Destruction / Indicator / UI |
| Element | 元素属性 | Fire / Water / Ice / Lightning / Wind / Earth / Light / Dark / Nature / Arcane / Tech |
| Form | 视觉形态 | Explosion / Ring / Beam / Orb / Slash / Column / Cloud / Shockwave / Ribbon / Particle |
| Context | 使用场景 | Combat / Character / Weapon / World / UI / Cinematic |
| Lifecycle | 生命周期 | OneShot / Loop / Persistent / Attached |
| Priority | 性能优先级 | Critical / Combat / Cosmetic / Ambient |
| Effect Type | Niagara 性能策略 | ET_VFX_Critical / ET_VFX_Combat / ET_VFX_Cosmetic / ET_VFX_Ambient；控制 Cull、Scalability、实例数与预算。 |

## 演示法术目录

| VFX Catalog | VFX ID | Legacy VFX ID | 效果名称 | 效果描述 | GameplayTag | Element | Form | Context | Lifecycle | Priority | 演示按键 | Effect Type |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 01_Cast → 02_Projectile → 05_Impact | MGVFX-SPELL-ICE-FROSTBOLT | MVFX-FROST-001 | 寒冰箭 | 尖锐冰晶弹头、冷雾尾迹、命中碎冰与霜纹。 | VFX.Spell.Ice.Frostbolt | Ice | Projectile | Combat | OneShot | Combat | 1 | ET_VFX_Combat |
| 06_Area | MGVFX-SPELL-ICE-FROSTNOVA | MVFX-FROST-002 | 冰环术 | 瞬发外扩冰霜环、冰晶碎片与地面冻结纹；演示冻结怪物 5 秒。 | VFX.Spell.Ice.FrostNova | Ice | Ring | Combat | OneShot | Critical | 2 | ET_VFX_Critical |
| 07_Status | MGVFX-SPELL-NATURE-PETALBLOOM | MVFX-NATURE-001 | 绯樱花瓣魔法 | 粉色丝状光环、旋转花瓣、光裙和漂浮光点。 | VFX.Spell.Nature.PetalBloom | Nature | Ring | Character | Persistent | Cosmetic | 3 | ET_VFX_Cosmetic |
| 07_Status | MGVFX-SPELL-NATURE-HEALINGAURA | MVFX-NATURE-002 | 翡翠治疗魔法 | 青绿色地面治疗光环与上升花瓣，跟随玩家并保持玩家位于中心。 | VFX.Spell.Nature.HealingAura | Nature | Ring | Character | Attached | Combat | 4 | ET_VFX_Combat |
| 01_Cast → 02_Projectile → 05_Impact | MGVFX-SPELL-FIRE-FIREBALL | MVFX-FIRE-001 | 火球术 | Flipbook 高质量火苗、流动火核、余烬尾迹与爆燃命中。 | VFX.Spell.Fire.Fireball | Fire | Orb | Combat | OneShot | Combat | 5 | ET_VFX_Combat |
| 05_Impact | MGVFX-SPELL-FIRE-FIREBLAST | MVFX-FIRE-002 | 火焰冲击 | 目标处瞬间爆燃、翻卷火舌、冲击圈及余烬。 | VFX.Spell.Fire.FireBlast | Fire | Explosion | Combat | OneShot | Combat | 6 | ET_VFX_Combat |
| 14_Indicator → 06_Area | MGVFX-SPELL-FIRE-FLAMESTRIKE | MVFX-FIRE-003 | 烈焰风暴 | 地面预警后降下火柱，形成环形火冠与持续燃烧的序列帧火舌。 | VFX.Spell.Fire.Flamestrike | Fire | Column | Combat | Persistent | Critical | 7 | ET_VFX_Critical |
| 01_Cast → 02_Projectile → 05_Impact | MGVFX-SPELL-ARCANE-MISSILES | MVFX-ARCANE-001 | 奥术飞弹 | 紫青色等离子弹头、双螺旋尾迹与奥术冲击辉光。 | VFX.Spell.Arcane.Missiles | Arcane | Orb | Combat | OneShot | Combat | 8 | ET_VFX_Combat |

## 生产 Niagara 效果目录

| VFX Catalog | VFX ID | Legacy VFX ID | 效果名称 | 效果描述 | GameplayTag | Element | Form | Context | Lifecycle | Priority | Effect Type | 粒子预算上限 | 参数 | 历史来源路径 | 标准目标路径 | Catalog 状态 | 资产迁移状态 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 01_Cast | MGVFX-CAST-ICE-FROSTBOLT-CHARGE | MVFX-FROST-001-CAST | 寒冰箭·蓄力 | Ice Orb；Combat 场景；OneShot 生命周期。 | VFX.Cast.Ice.FrostboltCharge | Ice | Orb | Combat | OneShot | Combat | ET_VFX_Combat | 未单独记录 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Frost/Niagara/Systems/NS_Frostbolt_CastCharge | /ModularGameVFX/ModularGameVFXLibrary/01_Cast/NS_Ice_Frostbolt_Charge | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 02_Projectile | MGVFX-PROJECTILE-ICE-FROSTBOLT | MVFX-FROST-001-PROJ | 寒冰箭·弹体 | Ice Orb；Combat 场景；Attached 生命周期。 | VFX.Projectile.Ice.Frostbolt | Ice | Orb | Combat | Attached | Combat | ET_VFX_Combat | 未单独记录 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Frost/Niagara/Systems/NS_Frostbolt_Projectile | /ModularGameVFX/ModularGameVFXLibrary/02_Projectile/NS_Ice_Frostbolt_Projectile | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 05_Impact | MGVFX-IMPACT-ICE-FROSTBOLT | MVFX-FROST-001-HIT | 寒冰箭·命中 | Ice Burst；Combat 场景；OneShot 生命周期。 | VFX.Impact.Ice.Frostbolt | Ice | Burst | Combat | OneShot | Combat | ET_VFX_Combat | 未单独记录 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Frost/Niagara/Systems/NS_Frostbolt_Impact | /ModularGameVFX/ModularGameVFXLibrary/05_Impact/NS_Ice_Frostbolt_Impact | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 06_Area | MGVFX-AREA-ICE-FROSTNOVA | MVFX-FROST-002-BURST | 冰环术·爆发 | Ice Ring；Combat 场景；OneShot 生命周期。 | VFX.Area.Ice.FrostNova | Ice | Ring | Combat | OneShot | Combat | ET_VFX_Combat | 未单独记录 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Frost/Niagara/Systems/NS_FrostNova_Burst | /ModularGameVFX/ModularGameVFXLibrary/06_Area/NS_Ice_FrostNova_Burst | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 07_Status | MGVFX-STATUS-NATURE-PETAL-AURA | MVFX-NATURE-SHARED-AURA | 花瓣光环·粉色/翡翠治疗共用 | Nature Ring；Combat 场景；Persistent 生命周期。 | VFX.Status.Nature.PetalBloom | Nature | Ring | Combat | Persistent | Combat | ET_VFX_Combat | 未单独记录 | User.GlowColor / PetalColor / CoreColor / Intensity / SpellDuration | /ModularGameVFX/Effects/Nature/Niagara/Systems/NS_PetalBloom_Aura | /ModularGameVFX/ModularGameVFXLibrary/07_Status/NS_Nature_PetalBloom_Aura | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 01_Cast | MGVFX-CAST-FIRE-FIREBALL-CHARGE | MVFX-FIRE-001-CAST | 火球术·蓄力 | Fire Orb；Combat 场景；OneShot 生命周期。 | VFX.Cast.Fire.FireballCharge | Fire | Orb | Combat | OneShot | Combat | ET_VFX_Combat | 28 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Fireball_CastCharge | /ModularGameVFX/ModularGameVFXLibrary/01_Cast/NS_Fire_Fireball_Charge | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 02_Projectile | MGVFX-PROJECTILE-FIRE-FIREBALL | MVFX-FIRE-001-PROJ | 火球术·弹体 | Fire Orb；Combat 场景；Attached 生命周期。 | VFX.Projectile.Fire.Fireball | Fire | Orb | Combat | Attached | Combat | ET_VFX_Combat | 44 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Fireball_Projectile | /ModularGameVFX/ModularGameVFXLibrary/02_Projectile/NS_Fire_Fireball_Projectile | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 05_Impact | MGVFX-IMPACT-FIRE-BURST | MVFX-FIRE-SHARED-HIT | 火焰·共用命中爆燃 | Fire Explosion；Combat 场景；OneShot 生命周期。 | VFX.Impact.Fire.FireBurst | Fire | Explosion | Combat | OneShot | Combat | ET_VFX_Combat | 66 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Fire_ImpactBurst | /ModularGameVFX/ModularGameVFXLibrary/05_Impact/NS_Fire_Impact_Burst | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 14_Indicator | MGVFX-INDICATOR-FIRE-FLAMESTRIKE | MVFX-FIRE-003-WARNING | 烈焰风暴·范围预警 | Fire Circle；Combat 场景；OneShot 生命周期。 | VFX.Indicator.Fire.FlamestrikeWarning | Fire | Circle | Combat | OneShot | Critical | ET_VFX_Critical | 25 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Flamestrike_GroundWarning | /ModularGameVFX/ModularGameVFXLibrary/14_Indicator/NS_Fire_Flamestrike_Warning | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 06_Area | MGVFX-AREA-FIRE-FLAMESTRIKE | MVFX-FIRE-003-GROUND | 烈焰风暴·地面爆发 | Fire Column；Combat 场景；OneShot 生命周期。 | VFX.Area.Fire.Flamestrike | Fire | Column | Combat | OneShot | Combat | ET_VFX_Combat | 90 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Fire/Niagara/Systems/NS_Flamestrike_GroundBurst | /ModularGameVFX/ModularGameVFXLibrary/06_Area/NS_Fire_Flamestrike_Area | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 01_Cast | MGVFX-CAST-ARCANE-MISSILES-CHANNEL | MVFX-ARCANE-001-CHANNEL | 奥术飞弹·引导 | Arcane Orb；Combat 场景；OneShot 生命周期。 | VFX.Cast.Arcane.MissilesChannel | Arcane | Orb | Combat | OneShot | Combat | ET_VFX_Combat | 22 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Arcane/Niagara/Systems/NS_ArcaneMissiles_Channel | /ModularGameVFX/ModularGameVFXLibrary/01_Cast/NS_Arcane_Missiles_Channel | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 02_Projectile | MGVFX-PROJECTILE-ARCANE-MISSILE | MVFX-ARCANE-001-PROJ | 奥术飞弹·弹体 | Arcane Orb；Combat 场景；Attached 生命周期。 | VFX.Projectile.Arcane.Missiles | Arcane | Orb | Combat | Attached | Combat | ET_VFX_Combat | 55 | User.HeadAlpha | /ModularGameVFX/Effects/Arcane/Niagara/Systems/NS_ArcaneMissiles_Projectile | /ModularGameVFX/ModularGameVFXLibrary/02_Projectile/NS_Arcane_Missiles_Projectile | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |
| 05_Impact | MGVFX-IMPACT-ARCANE-MISSILE | MVFX-ARCANE-001-HIT | 奥术飞弹·命中 | Arcane Burst；Combat 场景；OneShot 生命周期。 | VFX.Impact.Arcane.Missiles | Arcane | Burst | Combat | OneShot | Combat | ET_VFX_Combat | 32 | Niagara 系统及材质实例参数 | /ModularGameVFX/Effects/Arcane/Niagara/Systems/NS_ArcaneMissiles_Impact | /ModularGameVFX/ModularGameVFXLibrary/05_Impact/NS_Arcane_Missiles_Impact | Registered_Review | 已迁移（migration_complete.json）；旧路径已移除 |

## 既有通用库迁移目录

| VFX Catalog | VFX ID | Legacy VFX ID | 效果名称 | 效果描述 | GameplayTag | Element | Form | Context | Lifecycle | Priority | Effect Type | 历史来源路径 | 标准目标路径 | Catalog 状态 | 资产迁移状态 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 00_Core | MGVFX-LIB-CORE-SPARKS-001 | MVFX-LIB-CH01-001 | 早期魔法火花原子 | Energy Spark；Combat 场景；OneShot 生命周期。 | VFX.Core.Energy.MagicSparkLegacy | Energy | Spark | Combat | OneShot | Cosmetic | ET_VFX_Cosmetic（建议策略，未绑定） | /ModularGameVFX/Niagara/CH01_MagicSpark/Systems/NS_VFX_CH01_MagicSpark | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Sparks/Legacy/NS_Energy_MagicSpark_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 00_Core | MGVFX-LIB-CORE-SPARKS-002 | MVFX-LIB-CH01-002 | 早期简式魔法火花原子 | Energy Spark；Combat 场景；OneShot 生命周期。 | VFX.Core.Energy.MagicSparkSimpleLegacy | Energy | Spark | Combat | OneShot | Cosmetic | ET_VFX_Cosmetic（建议策略，未绑定） | /ModularGameVFX/Niagara/CH01_MagicSpark/Systems/NS_VFX_MagicSpark_01 | /ModularGameVFX/ModularGameVFXLibrary/00_Core/Sparks/Legacy/NS_Energy_MagicSparkSimple_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 01_Cast | MGVFX-LIB-CORE-ENERGY-001 | MVFX-LIB-CH03-001 | 早期蓄能球 | Energy Orb；Combat 场景；OneShot 生命周期。 | VFX.Cast.Energy.EnergyOrbLegacy | Energy | Orb | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH03_EnergyOrb/Systems/NS_VFX_CH03_EnergyOrb | /ModularGameVFX/ModularGameVFXLibrary/01_Cast/Legacy/NS_Energy_ChargeOrb_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 02_Projectile | MGVFX-LIB-PROJECTILE-001 | MVFX-LIB-CH04-001 | 早期魔法弹体 | Energy Orb；Combat 场景；Attached 生命周期。 | VFX.Projectile.Energy.MagicProjectileLegacy | Energy | Orb | Combat | Attached | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH04_MagicProjectile/Systems/NS_VFX_CH04_MagicProjectile | /ModularGameVFX/ModularGameVFXLibrary/02_Projectile/Legacy/NS_Energy_Projectile_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 03_Trail | MGVFX-LIB-TRAIL-001 | MVFX-LIB-CH05-001 | 早期弹体尾迹 | Energy Trail；Combat 场景；Attached 生命周期。 | VFX.Trail.Energy.ProjectileTrailLegacy | Energy | Trail | Combat | Attached | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH05_ProjectileTrail/Systems/NS_VFX_CH05_ProjectileTrail | /ModularGameVFX/ModularGameVFXLibrary/03_Trail/Legacy/NS_Energy_ProjectileTrail_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 05_Impact | MGVFX-LIB-IMPACT-001 | MVFX-LIB-CH06-001 | 早期魔法命中 | Energy Burst；Combat 场景；OneShot 生命周期。 | VFX.Impact.Energy.MagicImpactLegacy | Energy | Burst | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH06_MagicImpact/Systems/NS_VFX_CH06_MagicImpact | /ModularGameVFX/ModularGameVFXLibrary/05_Impact/Legacy/NS_Energy_Impact_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 06_Area | MGVFX-LIB-AREA-001 | MVFX-LIB-CH07-001 | 早期地面范围 | Energy Field；Combat 场景；OneShot 生命周期。 | VFX.Area.Energy.GroundAOELegacy | Energy | Field | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH07_GroundAOE/Systems/NS_VFX_CH07_GroundAOE | /ModularGameVFX/ModularGameVFXLibrary/06_Area/Legacy/NS_Energy_GroundArea_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 06_Area | MGVFX-LIB-AREA-002 | MVFX-LIB-CH08-001 | 早期魔法阵 | Energy Circle；Combat 场景；OneShot 生命周期。 | VFX.Area.Energy.MagicCircleLegacy | Energy | Circle | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH08_MagicCircle/Systems/NS_VFX_CH08_MagicCircle | /ModularGameVFX/ModularGameVFXLibrary/06_Area/Legacy/NS_Energy_MagicCircle_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 09_Lifecycle | MGVFX-LIB-LIFECYCLE-001 | MVFX-LIB-CH09-001 | 早期召唤 | Energy Column；Combat 场景；OneShot 生命周期。 | VFX.Lifecycle.Energy.SummoningLegacy | Energy | Column | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH09_Summoning/Systems/NS_VFX_CH09_Summoning | /ModularGameVFX/ModularGameVFXLibrary/09_Lifecycle/Legacy/NS_Energy_Summoning_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 08_Space | MGVFX-LIB-SPACE-001 | MVFX-LIB-CH10-001 | 早期传送门 | Arcane Portal；Combat 场景；OneShot 生命周期。 | VFX.Space.Arcane.PortalLegacy | Arcane | Portal | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH10_PortalTeleport/Systems/NS_VFX_CH10_Portal | /ModularGameVFX/ModularGameVFXLibrary/08_Space/Legacy/NS_Arcane_Portal_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 08_Space | MGVFX-LIB-SPACE-002 | MVFX-LIB-CH10-002 | 早期瞬移 | Arcane Burst；Combat 场景；OneShot 生命周期。 | VFX.Space.Arcane.TeleportLegacy | Arcane | Burst | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH10_PortalTeleport/Systems/NS_VFX_CH10_Teleport | /ModularGameVFX/ModularGameVFXLibrary/08_Space/Legacy/NS_Arcane_Teleport_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 07_Status | MGVFX-LIB-STATUS-001 | MVFX-LIB-CH11-001 | 早期魔法护盾 | Energy Orb；Combat 场景；Persistent 生命周期。 | VFX.Status.Energy.ShieldLegacy | Energy | Orb | Combat | Persistent | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH11_MagicShield/Systems/NS_VFX_CH11_MagicShield | /ModularGameVFX/ModularGameVFXLibrary/07_Status/Legacy/NS_Energy_Shield_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 04_Beam | MGVFX-LIB-BEAM-001 | MVFX-LIB-CH12-001 | 早期魔法光束 | Energy Beam；Combat 场景；OneShot 生命周期。 | VFX.Beam.Energy.MagicBeamLegacy | Energy | Beam | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH12_MagicBeam/Systems/NS_VFX_CH12_MagicBeam | /ModularGameVFX/ModularGameVFXLibrary/04_Beam/Legacy/NS_Energy_Beam_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 04_Beam | MGVFX-LIB-BEAM-LIGHTNING-001 | MVFX-LIB-CH13-001 | 早期雷链 | Lightning Beam；Combat 场景；OneShot 生命周期。 | VFX.Beam.Lightning.ChainLegacy | Lightning | Beam | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH13_Lightning/Systems/NS_VFX_CH13_ChainLightning | /ModularGameVFX/ModularGameVFXLibrary/04_Beam/Legacy/NS_Lightning_Chain_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 04_Beam | MGVFX-LIB-BEAM-LIGHTNING-002 | MVFX-LIB-CH13-002 | 早期闪电 | Lightning Beam；Combat 场景；OneShot 生命周期。 | VFX.Beam.Lightning.BoltLegacy | Lightning | Beam | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH13_Lightning/Systems/NS_VFX_CH13_Lightning | /ModularGameVFX/ModularGameVFXLibrary/04_Beam/Legacy/NS_Lightning_Bolt_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 05_Impact | MGVFX-LIB-IMPACT-FIRE-001 | MVFX-LIB-CH14-001 | 早期火焰爆炸 | Fire Explosion；Combat 场景；OneShot 生命周期。 | VFX.Impact.Fire.ExplosionLegacy | Fire | Explosion | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH14_Fire/Systems/NS_VFX_CH14_Fire_Explosion | /ModularGameVFX/ModularGameVFXLibrary/05_Impact/Legacy/NS_Fire_Explosion_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 11_World | MGVFX-LIB-AREA-FIRE-001 | MVFX-LIB-CH14-002 | 早期持续火焰 | Fire Field；World 场景；Loop 生命周期。 | VFX.World.Fire.FireLoopLegacy | Fire | Field | World | Loop | Ambient | ET_VFX_Ambient（建议策略，未绑定） | /ModularGameVFX/Niagara/CH14_Fire/Systems/NS_VFX_CH14_Fire_Loop | /ModularGameVFX/ModularGameVFXLibrary/11_World/Legacy/NS_Fire_Loop_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 05_Impact | MGVFX-LIB-IMPACT-FIRE-002 | MVFX-LIB-CH14-003 | 早期熔岩喷发 | Fire Burst；Combat 场景；OneShot 生命周期。 | VFX.Impact.Fire.LavaBurstLegacy | Fire | Burst | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH14_Fire/Systems/NS_VFX_CH14_LavaBurst | /ModularGameVFX/ModularGameVFXLibrary/05_Impact/Legacy/NS_Fire_LavaBurst_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 06_Area | MGVFX-LIB-AREA-ICE-001 | MVFX-LIB-CH15-001 | 早期冰环 | Ice Ring；Combat 场景；OneShot 生命周期。 | VFX.Area.Ice.FrostNovaLegacy | Ice | Ring | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH15_FrostIce/Systems/NS_VFX_FrostNova | /ModularGameVFX/ModularGameVFXLibrary/06_Area/Legacy/NS_Ice_FrostNova_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 01_Cast | MGVFX-LIB-CAST-ICE-001 | MVFX-LIB-CH15-002 | 早期寒冰箭蓄力 | Ice Orb；Combat 场景；OneShot 生命周期。 | VFX.Cast.Ice.FrostboltChargeLegacy | Ice | Orb | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH15_FrostIce/Systems/NS_VFX_Frostbolt_Charge | /ModularGameVFX/ModularGameVFXLibrary/01_Cast/Legacy/NS_Ice_FrostboltCharge_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 05_Impact | MGVFX-LIB-IMPACT-ICE-001 | MVFX-LIB-CH15-003 | 早期寒冰箭命中 | Ice Burst；Combat 场景；OneShot 生命周期。 | VFX.Impact.Ice.FrostboltLegacy | Ice | Burst | Combat | OneShot | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH15_FrostIce/Systems/NS_VFX_Frostbolt_Impact | /ModularGameVFX/ModularGameVFXLibrary/05_Impact/Legacy/NS_Ice_FrostboltImpact_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 02_Projectile | MGVFX-LIB-PROJECTILE-ICE-001 | MVFX-LIB-CH15-004 | 早期寒冰箭弹体 | Ice Orb；Combat 场景；Attached 生命周期。 | VFX.Projectile.Ice.FrostboltLegacy | Ice | Orb | Combat | Attached | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/CH15_FrostIce/Systems/NS_VFX_Frostbolt_Projectile | /ModularGameVFX/ModularGameVFXLibrary/02_Projectile/Legacy/NS_Ice_FrostboltProjectile_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |
| 07_Status | MGVFX-LIB-STATUS-NATURE-001 | MVFX-LIB-PETALSAURA-001 | 早期花瓣光环 | Nature Ring；Combat 场景；Persistent 生命周期。 | VFX.Status.Nature.PetalsAuraLegacy | Nature | Ring | Combat | Persistent | Combat | ET_VFX_Combat（建议策略，未绑定） | /ModularGameVFX/Niagara/PetalsAura/NS_VFX_PetalsAura | /ModularGameVFX/ModularGameVFXLibrary/07_Status/Legacy/NS_Nature_PetalsAura_Legacy | Prototype_RequiresValidation | 已迁移（migration_complete.json）；旧路径已移除 |

## 真实资产迁移总览

| 资产类型 | 数量 | 资产迁移状态 |
| --- | --- | --- |
| Texture2D | 389 | 已迁移（migration_complete.json）；旧路径已移除 |
| MaterialInstanceConstant | 323 | 已迁移（migration_complete.json）；旧路径已移除 |
| Material | 155 | 已迁移（migration_complete.json）；旧路径已移除 |
| NiagaraScript | 118 | 已迁移（migration_complete.json）；旧路径已移除 |
| NiagaraSystem | 36 | 已迁移（migration_complete.json）；旧路径已移除 |
| NiagaraEffectType | 17 | 已迁移（migration_complete.json）；旧路径已移除 |
| StaticMesh | 11 | 已迁移（migration_complete.json）；旧路径已移除 |
| NiagaraEmitter | 9 | 已迁移（migration_complete.json）；旧路径已移除 |
| Blueprint | 1 | 已迁移（migration_complete.json）；旧路径已移除 |

完整的 1059 项资产映射写入同名 Excel 的“资产总表”工作表、JSON 的 `assetInventory` 数组，以及 `ModularGameVFX_1.0.0_Asset_Migration_Catalog.csv`。
