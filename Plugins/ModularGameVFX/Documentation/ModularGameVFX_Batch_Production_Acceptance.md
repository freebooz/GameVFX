# ModularGameVFX 批量资产生产与质量验收规范

## 一、目标

用于批量建设：

```text
Fire
Water
Ice
Lightning
Wind
Earth
Light
Dark
Poison
Nature
Arcane
Tech
```

等基础元素 VFX 库。

AI 不应简单生成一批独立 Niagara，而应：

```text
先规划
↓
检查Core复用
↓
建立共享资产
↓
批量组合
↓
统一Catalog
↓
统一性能
↓
自动验收
```

最终形成可跨项目复用的完整 VFX Pack。

---

# 二、批量生产基本原则

批量生产时优先：

```text
共性先做
个性后做
```

例如 Fire 系列：

```text
Fire Projectile
Fire Trail
Fire Impact
Fire Area
Fire Status
Fire Cast
```

应首先识别可以共享的：

```text
Fire Material Instance
Fire Flipbook
Fire Noise
Fire Color Profile
Fire Sparks
Fire Smoke
Fire Ember
```

而不是每一个特效重新创建一套。

---

# 三、标准元素包结构

每个 Element Pack 建议至少覆盖：

```text
Cast
Projectile
Trail
Beam
Impact
Area
Status
Lifecycle
Surface
```

并根据元素特点选择是否增加：

```text
Space
Character
World
Destruction
Indicator
```

例如 Fire：

```text
Fire/
├─ Cast
├─ Projectile
├─ Trail
├─ Impact
├─ Area
├─ Status
├─ Lifecycle
├─ Surface
└─ World
```

---

# 四、基础批量资产矩阵

每个主要元素优先建立以下核心集合。

```text
01 Minor Cast
02 Heavy Cast

03 Small Projectile
04 Heavy Projectile

05 Weapon Trail
06 Projectile Trail

07 Light Impact
08 Heavy Impact

09 Small Explosion
10 Heavy Explosion

11 Ground Area
12 Persistent Field

13 Buff / Aura
14 Debuff

15 Spawn
16 Dissolve

17 Surface Residue
18 Environmental Variant
```

不要求每种元素机械拥有全部 18 个。

必须根据视觉语义裁剪。

---

# 五、Fire Pack 示例

建议：

```text
VFX.Cast.Fire.Charge

VFX.Projectile.Fire.Small
VFX.Projectile.Fire.Heavy

VFX.Trail.Fire.Weapon

VFX.Impact.Fire.Light
VFX.Impact.Fire.Heavy

VFX.Area.Fire.Burning

VFX.Status.Fire.Burning

VFX.Surface.Fire.Scorch

VFX.World.Fire.Embers
```

共享：

```text
MI_VFX_Fire_Additive
MI_VFX_Fire_Translucent

T_VFX_FB_Fire
T_VFX_FB_Smoke
T_VFX_Noise_Fire

NE_VFX_Sparks
NE_VFX_Smoke
NE_VFX_Embers
```

---

# 六、Water Pack 示例

核心视觉语言：

```text
流动
旋转
卷曲
折射
波纹
飞溅
水滴
泡沫
```

建议：

```text
VFX.Cast.Water.Charge

VFX.Projectile.Water.Orb

VFX.Trail.Water.Stream

VFX.Impact.Water.Splash

VFX.Area.Water.Pool

VFX.Status.Water.Flow

VFX.Surface.Water.Wet

VFX.World.Water.Splash
```

---

# 七、Ice Pack 示例

核心：

```text
晶体
尖锐
冻结
裂纹
碎裂
冷雾
```

建议：

```text
VFX.Cast.Ice.Charge

VFX.Projectile.Ice.Shard

VFX.Trail.Ice

VFX.Impact.Ice.Shatter

VFX.Area.Ice.FrozenField

VFX.Status.Ice.Frozen

VFX.Surface.Ice.Frost

VFX.Destruction.Ice.Shatter
```

---

# 八、Lightning Pack 示例

核心：

```text
瞬时
分叉
高速
跳跃
高亮
短生命周期
```

建议：

```text
VFX.Cast.Lightning.Charge

VFX.Projectile.Lightning.Bolt

VFX.Trail.Lightning

VFX.Beam.Lightning.Chain

VFX.Impact.Lightning.Heavy

VFX.Area.Lightning.Field

VFX.Status.Lightning.Electrified
```

避免：

```text
长时间大量透明电弧
```

雷电应以：

```text
短
快
亮
不规则
```

为主。

---

# 九、Wind Pack 示例

核心：

```text
流线
旋涡
空气扰动
尘埃
叶片
Ribbon
```

建议：

```text
VFX.Cast.Wind.Charge

VFX.Projectile.Wind.Blade

VFX.Trail.Wind

VFX.Impact.Wind.Burst

VFX.Area.Wind.Vortex

VFX.Status.Wind.Speed

VFX.World.Wind.Dust
```

---

# 十、Earth Pack 示例

核心：

```text
重量
岩石
地裂
尘土
碎块
低频运动
```

建议：

```text
VFX.Cast.Earth.Charge

VFX.Projectile.Earth.Rock

VFX.Impact.Earth.Heavy

VFX.Area.Earth.Quake

VFX.Status.Earth.Armor

VFX.Surface.Earth.Crack

VFX.Destruction.Earth.Rock
```

---

# 十一、Light / Holy Pack

核心：

```text
秩序
垂直
金白
Halo
光柱
符文
纯净
```

推荐：

```text
VFX.Cast.Light.HolyCharge

VFX.Beam.Light.Holy

VFX.Impact.Light.HolyBurst

VFX.Area.Light.Sanctuary

VFX.Status.Light.Blessing

VFX.Lifecycle.Light.Revive
```

---

# 十二、Dark Pack

核心：

```text
吸收
压缩
扭曲
暗色核心
少量高亮边缘
```

推荐：

```text
VFX.Cast.Dark.Charge

VFX.Projectile.Dark.Orb

VFX.Beam.Dark.Link

VFX.Impact.Dark.Heavy

VFX.Area.Dark.Void

VFX.Status.Dark.Curse

VFX.Space.Dark.Rift
```

---

# 十三、批量资产必须先建立共享层

每个 Element Pack 创建前，先判断需要哪些共享资产。

例如：

```text
FireShared/
├─ Materials
├─ MaterialInstances
├─ Textures
├─ Flipbooks
├─ Emitters
└─ Data
```

但这些共享资源如果具有跨元素复用价值：

优先放入：

```text
00_Core
```

例如：

```text
Shockwave Mesh
Radial Mask
Smoke Flipbook
Generic Sparks
```

不要复制进 Fire / Ice / Lightning 多份。

---

# 十四、颜色Profile

建议建立：

```text
FVFXColorProfile
```

或对应 Data Asset。

例如：

```text
Fire
Primary = OrangeRed
Secondary = Yellow
Accent = White

Ice
Primary = IceBlue
Secondary = Cyan
Accent = White

Lightning
Primary = ElectricBlue
Secondary = Violet
Accent = White
```

大量效果通过：

```text
Color Profile
+
User.PrimaryColor
+
User.SecondaryColor
```

驱动。

避免每个颜色变体复制 Niagara。

---

# 十五、批量复用目标

普通 Element Pack 建议达到：

```text
60%～85%
使用Core已有资产
```

包括：

```text
Niagara Module
Emitter Template
Material
Texture
Mesh
Curve
```

特色元素资产：

```text
15%～40%
```

例如：

```text
Lightning Arc
Ice Crystal
Fire Flipbook
Water Foam
```

才需要专属资源。

---

# 十六、批量创建顺序

推荐固定：

```text
1. Element视觉语言

2. Color Profile

3. Shared Material Instance

4. Shared Texture / Flipbook

5. Shared特殊Emitter

6. Cast

7. Impact

8. Projectile

9. Trail

10. Area

11. Status

12. Lifecycle

13. Surface

14. World Variant

15. Catalog注册

16. Scalability

17. Profile

18. 验收
```

优先完成：

```text
Impact
Cast
Projectile
```

因为复用价值最高。

---

# 十七、批量 Catalog 注册

禁止人工零散填写。

应该建立统一数据结构：

```text
GameplayTag
NiagaraSystem
Category
Element
Form
Lifecycle
Priority
Pooling
EffectType
```

例如：

```text
VFX.Impact.Fire.Light
VFX.Impact.Fire.Heavy
VFX.Impact.Ice.Light
VFX.Impact.Ice.Heavy
VFX.Impact.Lightning.Light
VFX.Impact.Lightning.Heavy
```

命名必须规律化。

---

# 十八、禁止生成机械变体

例如：

```text
Fire Impact Red
Fire Impact Blue
Fire Impact Green
```

如果仅颜色不同：

禁止创建三个 Niagara。

改为：

```text
NS_Fire_Impact
+
Color Parameter
```

同样：

```text
Small
Medium
Large
```

如果只是 Scale 和 Spawn Count 差异：

优先：

```text
User.Scale
User.Intensity
```

而不是复制 System。

---

# 十九、允许建立独立变体的条件

满足以下之一才允许：

```text
轮廓结构明显不同

Emitter组合明显不同

生命周期明显不同

Gameplay含义明显不同

材质模型明显不同

性能策略明显不同
```

例如：

```text
Fire LightImpact
```

与：

```text
Fire BossMeteorImpact
```

可以独立，因为视觉结构已经完全不同。

---

# 二十、质量等级矩阵

批量资产必须统一支持：

```text
High
Medium
Low
VRMobile
```

不要：

```text
Fire有4档
Ice只有2档
Lightning没有VR
```

导致整体库策略不统一。

---

# 二十一、批量性能检查

一次 Element Pack 完成后必须整体测试：

```text
单独播放

10个同时播放

30个同时播放

50个普通Hit

多个HeavyImpact同时播放

持续Area叠加

Projectile批量飞行
```

检查：

```text
GPU Time
CPU Time
Niagara Instances
Particle Count
Overdraw
Texture Memory
```

---

# 二十二、视觉一致性验收

同一个 Element Pack 应具有：

```text
统一主色语言

统一粒子运动特征

统一材质风格

统一亮度范围

统一结束方式
```

例如 Lightning 系列应该让玩家第一眼感觉：

```text
这些属于同一种雷元素体系
```

而不是每一个效果完全不同。

---

# 二十三、不同元素必须有辨识度

同时又必须避免：

```text
只是换颜色
```

例如：

```text
Fire
上升、翻卷、烟、火星

Ice
晶体、碎裂、冷雾

Lightning
分叉、瞬时、高速

Earth
低速、大块、尘土

Wind
流线、旋涡、空气感
```

所以元素差异应该来自：

```text
Shape
Motion
Timing
Material
Secondary FX
```

而不仅是颜色。

---

# 二十四、自动重复资产检查

批量完成后 AI 必须检查：

```text
相同功能Module是否重复

相同Material是否重复

相同Texture是否重复

相同Mesh是否重复

Niagara System是否仅颜色不同

Emitter是否可以参数化合并
```

发现重复：

优先合并。

---

# 二十五、依赖检查

任何批量资产必须确保：

```text
ModularGameVFX
不能依赖
Project Content
```

禁止引用：

```text
/Game/Characters
/Game/Maps
/Game/Skills
```

跨项目库只能依赖：

```text
自身插件
Engine
Niagara
必要基础插件
```

---

# 二十六、视觉质量自动评分

建议内部采用 100 分制。

```text
视觉中心        15
打击/力量感      15
层次感          10
时间节奏        10
元素辨识度      10
Gameplay可读性  15
资产复用率      10
性能设计        10
命名与规范       5
```

总分：

```text
90～100
Hero / Excellent

80～89
Production Ready

70～79
需要优化

<70
不允许进入正式库
```

---

# 二十七、性能评分

单独建立：

```text
Overdraw
Particle Count
Material Complexity
Instance Cost
Texture Memory
Scalability
Pooling
Cull
```

出现以下情况：

```text
大量全屏透明粒子

无Cull

无Instance Limit

高频VFX无Pooling

大量Particle Light

大量CPU Collision
```

直接判定：

```text
Performance Review Failed
```

---

# 二十八、正式入库条件

一个 VFX 只有同时满足：

```text
Visual Approved

Gameplay Approved

Performance Approved

Naming Approved

Catalog Registered

Dependency Approved
```

才能标记：

```text
Production Ready
```

---

# 二十九、资产状态

建议每个 VFX 增加：

```text
Prototype

Review

Optimized

ProductionReady

Deprecated
```

避免：

```text
测试资产
```

和：

```text
正式资产
```

混在一起。

---

# 三十、Deprecated处理

旧资产不要直接删除。

标记：

```text
Deprecated
```

并记录：

```text
Replacement VFX Tag
```

例如：

```text
VFX.Impact.Fire.Heavy.V1
→ Deprecated

Replacement:
VFX.Impact.Fire.Heavy
```

便于升级。

---

# 三十一、版本信息

正式 Pack 建议维护：

```text
PackName

Version

EngineVersion

MinimumFrameworkVersion

LastUpdated
```

例如：

```text
ModularGameVFX_ElementalPack

Version
1.2.0

Engine
5.8
```

---

# 三十二、批量 AI 执行提示词

```text
你现在需要为 Unreal Engine 5.8 的 ModularGameVFX
批量创建一个正式生产级元素VFX Pack。

元素：

{Fire / Water / Ice / Lightning / Wind / Earth / Light / Dark 等}

目标：

建立视觉语言统一、
高复用、
高品质、
性能可控、
跨项目使用的完整元素VFX资产包。

首先：

1. 分析该元素的核心视觉语言
2. 检查00_Core现有资产
3. 检查已有同类完整VFX
4. 建立共享Material / Texture / Emitter方案
5. 禁止创建功能重复资产

然后至少规划：

Cast
Projectile
Trail
Impact
Area
Status
Lifecycle
Surface

根据元素特性决定是否增加：

Beam
Space
World
Destruction。

每个VFX必须定义：

GameplayTag
Category
Element
Form
Context
Lifecycle
Priority
Niagara System
Emitter
Material
Texture
Mesh
User Parameters
Pooling
Effect Type
Scalability。

视觉品质要求：

AAA游戏正式生产级。

必须具有：

明确视觉核心
明确力量来源
明确运动方向
明确时间节奏
明确近中远空间层级。

视觉目标：

惊艳
震撼
宏大
史诗
强打击感
力量感
速度感
重量感
空间感
压迫感。

但不同等级VFX必须保持合理差异：

普通Hit不能做得像Boss大招。

禁止：

粒子堆砌
Bloom滥用
大量透明层
大量动态光
大量Distortion
大量CPU Collision
重复Material
重复Niagara Module。

元素差异不能只通过换颜色。

必须通过：

Shape
Motion
Timing
Material
Secondary FX

建立真正辨识度。

所有资源优先复用：

Core Niagara Module
Emitter Template
Master Material
Material Function
Texture
Mesh
Curve。

完成后执行：

重复资源检查

Dependency检查

Catalog检查

GameplayTag检查

Pooling检查

EffectType检查

High/Medium/Low/VRMobile检查

GPU/CPU性能检查

视觉一致性检查

元素辨识度检查。

最后输出：

1. Pack结构
2. 新建资产
3. 复用资产
4. GameplayTag清单
5. Catalog清单
6. Niagara结构
7. 共享Material
8. 共享Texture
9. 共享Emitter
10. Scalability
11. 性能风险
12. 优化结果
13. 质量评分
14. 未完成事项。
```

---

# 三十三、元素库推荐首批建设顺序

建议优先：

```text
第一批
Fire
Lightning
Energy

第二批
Ice
Water
Wind

第三批
Earth
Nature
Poison

第四批
Light
Dark
Arcane

第五批
Tech
Physical
```

因为第一批可以覆盖最多：

```text
技能
命中
弹道
Boss
武器
环境
```

使用场景。

---

# 三十四、最终批量生产体系

最终 AI 生产链：

```text
需求
↓
选择Element
↓
分析视觉语言
↓
扫描Core
↓
建立共享层
↓
批量创建基础VFX
↓
Catalog自动注册
↓
Effect Type
↓
Scalability
↓
批量压力测试
↓
自动质量评分
↓
Production Ready
```

这样 `ModularGameVFX` 就从“单个特效开发规范”进一步升级成：

```text
可批量扩展的VFX产品化生产体系
```
