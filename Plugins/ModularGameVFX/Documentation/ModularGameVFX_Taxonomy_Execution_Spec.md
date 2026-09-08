# ModularGameVFX 分类体系 AI执行提示词

你现在是一名资深 Unreal Engine 5.8 技术美术、Niagara VFX 架构师和 C++ 工程师。

请在当前 `ModularGameVFX` 插件中建立统一的 VFX 分类、命名、GameplayTag、Catalog Metadata 和性能管理规范。

目标：

```text
统一归档
统一搜索
统一调用
统一性能管理
支持跨项目复用
支持未来数百至数千个VFX资产
```

不要只输出说明，应根据当前项目实际结构直接实施可以自动完成的内容。

---

## 一、建立统一一级分类

使用以下一级分类：

```text
00_Core
01_Cast
02_Projectile
03_Trail
04_Beam
05_Impact
06_Area
07_Status
08_Space
09_Lifecycle
10_Character
11_World
12_Surface
13_Destruction
14_Indicator
15_UI
```

含义：

```text
00_Core
基础视觉原子：
Energy、Fire、Smoke、Spark、Dust、Lightning、
Glow、Shockwave、Debris、Noise等。

01_Cast
施法、蓄力、引导、释放。

02_Projectile
火球、箭矢、子弹、剑气、飞行能量体。

03_Trail
刀光、武器拖尾、冲刺残影、飞行拖尾。

04_Beam
激光、雷链、连接线、治疗光束、牵引线。

05_Impact
命中、受击、爆炸、冲击、飞溅。

06_Area
AOE、法阵、持续区域、地面能量场。

07_Status
Buff、Debuff、Shield、Aura、持续治疗等。

08_Space
Teleport、Portal、Warp、Rift、BlackHole。

09_Lifecycle
Spawn、Summon、Transform、Death、Dissolve、Revive。

10_Character
角色动作、武器附着、脚步、跳跃、落地、冲刺。

11_World
天气、环境、自然、雨雪、雾、风、水、氛围。

12_Surface
Decal、弹孔、焦痕、裂纹、脚印、冰霜、残留。

13_Destruction
碎裂、崩塌、Chaos、残骸、破坏。

14_Indicator
AOE范围、Boss预警、瞄准、锁定、落点、轨迹。

15_UI
UI反馈、屏幕反馈、Camera FX、PostProcess。
```

---

## 二、Core进一步整理

`00_Core` 建议建立：

```text
00_Core/
├─ Energy/
├─ Fire/
├─ Smoke/
├─ Sparks/
├─ Dust/
├─ Lightning/
├─ Glow/
├─ Shockwave/
├─ Debris/
├─ Noise/
├─ Ribbon/
├─ Meshes/
├─ Materials/
├─ MaterialFunctions/
├─ Textures/
├─ NiagaraModules/
└─ Curves/
```

Core 资源属于：

```text
可被多个完整VFX复用的基础视觉积木
```

不要把完整技能特效放入 Core。

---

## 三、禁止按元素建立一级目录

不要建立：

```text
Fire/
Water/
Lightning/
Ice/
```

作为顶层业务分类。

元素应该作为 Metadata / GameplayTag 使用。

例如：

```text
Category = Impact

Element = Fire
```

而不是：

```text
Fire/
└─ Impact/
```

---

## 四、建立多维分类 Metadata

每个正式 VFX 至少支持以下属性。

### Category

```text
Cast
Projectile
Trail
Beam
Impact
Area
Status
Space
Lifecycle
Character
World
Surface
Destruction
Indicator
UI
```

### Element

```text
None

Energy
Fire
Water
Ice
Lightning
Wind
Earth
Nature
Poison
Light
Dark
Arcane
Tech
Physical
```

### Form

```text
Flash
Explosion
Shockwave
Ring
Orb
Beam
Slash
Trail
Column
Cloud
Burst
Wave
Field
Circle
Portal
Spark
Smoke
Debris
```

允许后续扩展。

### Context

```text
Combat
Character
Weapon
World
UI
Cinematic
Interaction
```

### Lifecycle

```text
OneShot
Loop
Persistent
Attached
```

### Priority

```text
Critical
Combat
Cosmetic
Ambient
```

---

## 五、Catalog Entry增加Metadata

检查现有：

```cpp
FVFXCatalogEntry
```

如果尚未存在这些字段，在保持兼容的前提下增加：

```text
Category
Element
Form
Context
Lifecycle
Priority
```

优先使用：

```cpp
UENUM(BlueprintType)
```

或者 GameplayTag。

不要用随意字符串：

```cpp
FString Category;
```

避免拼写错误。

---

## 六、GameplayTag规范

业务调用 Tag 使用：

```text
VFX.<Category>.<SubCategory>.<Name>
```

例如：

```text
VFX.Cast.Energy.Charge

VFX.Projectile.Fire.Fireball

VFX.Trail.Weapon.Sword

VFX.Beam.Lightning.Chain

VFX.Impact.Fire.Heavy

VFX.Area.Fire.Burning

VFX.Status.Shield.Energy

VFX.Space.Portal

VFX.Lifecycle.Spawn.Energy

VFX.Character.Motion.Dash

VFX.World.Weather.Rain

VFX.Surface.Scorch.Fire

VFX.Destruction.Rock.Heavy

VFX.Indicator.AOE.Circle

VFX.UI.Feedback.Confirm
```

项目专属资源允许：

```text
VFX.Character.Tiger.Skill01.Hit
```

但项目专属 Tag 不能写死在 `ModularGameVFX` C++ 框架逻辑中。

---

## 七、完整技能不等于单个VFX

例如：

```text
火焰大招
```

应允许拆成：

```text
Cast
Projectile
Trail
Impact
Area
Surface
```

例如：

```text
VFX.Cast.Fire.HeavyCharge

VFX.Projectile.Fire.HeavyOrb

VFX.Trail.Fire.Heavy

VFX.Impact.Fire.HeavyExplosion

VFX.Area.Fire.BurningField

VFX.Surface.Scorch.Heavy
```

业务技能负责组合这些基础 VFX。

不要把所有逻辑永久塞进：

```text
NS_FireUltimate_Final
```

一个巨大 Niagara System。

---

## 八、资产命名规范

统一使用：

```text
NS_    Niagara System
NE_    Niagara Emitter
NM_    Niagara Module

M_     Material
MI_    Material Instance
MF_    Material Function

T_     Texture

SM_    Static Mesh

DA_    Data Asset

ET_    Niagara Effect Type

BP_    Blueprint
```

例如：

```text
NS_Fire_HeavyImpact

NE_Fire_Core
NE_Fire_Sparks
NE_Fire_Smoke

MI_VFX_Fire_Impact

T_VFX_Fire_Noise01
```

禁止：

```text
NewEffect
FinalEffect
Final2
TestFX
FX01
```

---

## 九、Niagara Emitter命名

Emitter 必须按视觉职责命名：

```text
Core
Flash
Shockwave
Sparks
Debris
Smoke
Trail
Ribbon
Ground
Residual
```

例如：

```text
NE_Impact_Core

NE_Impact_Flash

NE_Impact_Shockwave

NE_Impact_Debris
```

禁止：

```text
Emitter01
Emitter02
Emitter03
```

---

## 十、性能类型与分类分离

目录：

```text
Impact
Area
World
```

表示：

```text
VFX是干什么的
```

Effect Type：

```text
ET_VFX_Critical
ET_VFX_Combat
ET_VFX_Cosmetic
ET_VFX_Ambient
```

表示：

```text
VFX应该怎样进行性能管理
```

两者不得混用。

---

## 十一、推荐Effect Type

建立或复用：

```text
ET_VFX_Critical

ET_VFX_Combat

ET_VFX_Cosmetic

ET_VFX_Ambient
```

对应：

```text
Critical
玩家大招、Boss核心技能。

Combat
攻击、Hit、普通技能。

Cosmetic
装饰表现。

Ambient
环境表现。
```

并分别配置合理：

```text
Distance Culling
Instance Limit
Significance
Scalability
```

---

## 十二、质量等级

重要 VFX 至少考虑：

```text
High
Medium
Low
VRMobile
```

Hero级可增加：

```text
Cinematic
```

降级顺序：

```text
Secondary Particles
↓
Spawn Rate
↓
Smoke
↓
Ribbon Detail
↓
Distortion
↓
Particle Lights
↓
Mesh Particles
↓
Residual Lifetime
```

最后才允许减少：

```text
核心轮廓
主冲击
技能范围
```

---

## 十三、正式VFX创建检查

创建新 VFX 前必须判断：

```text
现有库是否已经有相同效果？

是否可以复用现有Niagara Module？

是否可以复用Master Material？

是否可以复用Texture？

是否可以通过参数变化实现，而不是新建资源？
```

例如：

```text
红色护盾
蓝色护盾
```

优先：

```text
NS_Common_Shield
+
User.PrimaryColor
```

禁止直接建立：

```text
NS_Shield_Red
NS_Shield_Blue
```

除非两者视觉结构确实不同。

---

## 十四、分类判断优先级

如果一个资源同时属于多个类别：

使用：

```text
“这个VFX最主要在游戏中承担什么职责？”
```

决定一级分类。

例如：

```text
雷电链命中爆炸
```

如果资源主体是：

```text
持续雷链
```

分类：

```text
Beam
```

如果资源主体是：

```text
最终命中爆炸
```

分类：

```text
Impact
```

通过：

```text
Element
Form
Context
```

表达其他属性。

禁止复制一份资源到两个目录。

---

## 十五、最终资产描述示例

例如：

```text
NS_Lightning_HeavyImpact
```

Metadata：

```text
GameplayTag
VFX.Impact.Lightning.Heavy

Category
Impact

Element
Lightning

Form
Explosion

Context
Combat

Lifecycle
OneShot

Priority
Combat
```

另一个：

```text
NS_Dark_BossBlackHole
```

Metadata：

```text
GameplayTag
VFX.Space.Dark.BlackHole

Category
Space

Element
Dark

Form
Field

Context
Combat

Lifecycle
Persistent

Priority
Critical
```

---

## 十六、最终体系

必须形成：

```text
文件夹
↓
解决资源放在哪里


GameplayTag
↓
解决业务如何调用


VFX Catalog
↓
解决Tag对应哪个实际资源


Metadata
↓
解决资源是什么类型


Effect Type
↓
解决性能、Cull、Scalability


VFX Manager
↓
解决加载、播放、停止和生命周期
```

---

## 十七、最终验收

完成后检查：

```text
是否建立统一15类目录

Core是否与完整VFX分离

元素是否采用Metadata而非一级目录

GameplayTag是否规范

Catalog是否具有分类Metadata

是否没有重复资源

是否建立Effect Type

是否支持性能等级

是否符合ModularGameVFX跨项目原则
```

最终目标：

建立一套即使拥有：

```text
1000+
Niagara Systems
```

仍然能够：

```text
快速查找

自动分类

方便复用

统一调用

统一性能管理

跨项目迁移
```

的 ModularGameVFX 视觉资产管理体系。
