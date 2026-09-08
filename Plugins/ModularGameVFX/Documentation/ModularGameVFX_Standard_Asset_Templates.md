# ModularGameVFX 标准资产模板与 AI 执行规范

## 一、总体原则

所有正式 VFX 均遵循：

```text
GameplayTag
↓
VFX Catalog
↓
VFX Manager
↓
Niagara System
↓
Emitter
↓
Material / Texture / Mesh
```

每个 VFX 至少明确：

```text
用途
视觉核心
生命周期
性能等级
GameplayTag
Niagara System
User Parameters
Pooling
Effect Type
Scalability
```

禁止仅创建 Niagara 资源而没有 Catalog、性能和调用规范。

---

# 二、统一 Niagara System 模板

建议所有正式 Niagara System 根据需要从以下功能层选择：

```text
Core
核心主体

Flash
瞬间亮度/命中闪光

Trail
运动轨迹

Shockwave
冲击波

Sparks
火花/高频粒子

Debris
碎片

Smoke
烟雾/尘埃

Ground
地面反馈

Secondary
二级细节

Residual
残留
```

不是所有特效都必须包含全部 Emitter。

原则：

```text
只保留有明确视觉职责的Emitter。
```

---

# 三、01_Cast 施法模板

适用：

```text
施法
蓄力
引导
释放前摇
能量聚集
```

推荐结构：

```text
NS_Cast_xxx
│
├─ NE_Core
├─ NE_Attraction
├─ NE_Ring
├─ NE_Sparks
└─ NE_Residual
```

视觉节奏：

```text
低强度
↓
粒子向中心聚集
↓
核心亮度提升
↓
释放前达到视觉峰值
```

常用参数：

```text
User.PrimaryColor
User.SecondaryColor
User.Intensity
User.LifetimeScale
User.Scale
```

性能等级：

```text
Combat
或
Critical
```

---

# 四、02_Projectile 投射物模板

适用：

```text
火球
能量球
子弹
剑气
魔法弹
飞行技能
```

推荐结构：

```text
NS_Projectile_xxx
│
├─ NE_Core
├─ NE_OuterGlow
├─ NE_Trail
├─ NE_Sparks
└─ NE_Residual
```

核心要求：

```text
主体轮廓明确
运动方向明确
高速时仍可识别
```

推荐参数：

```text
User.PrimaryColor
User.Intensity
User.Direction
User.Scale
```

通常：

```text
Attached
```

到 Projectile Actor。

---

# 五、03_Trail 拖尾模板

适用：

```text
刀光
剑光
冲刺残影
武器拖尾
高速运动轨迹
```

推荐：

```text
NS_Trail_xxx
│
├─ NE_PrimaryRibbon
├─ NE_SecondaryRibbon
├─ NE_Sparks
└─ NE_Fade
```

核心：

```text
方向
宽度变化
速度变化
透明度变化
```

低质量模式优先删除：

```text
Secondary Ribbon
Sparks
```

保留：

```text
Primary Ribbon
```

---

# 六、04_Beam 光束模板

适用：

```text
激光
雷链
治疗光束
能量连接
牵引线
```

推荐：

```text
NS_Beam_xxx
│
├─ NE_PrimaryBeam
├─ NE_Core
├─ NE_ElectricDetail
├─ NE_Source
└─ NE_Target
```

核心参数：

```text
User.SourceLocation
User.TargetLocation
User.PrimaryColor
User.Intensity
User.Width
```

Beam 与 Hit 分离。

Beam 只负责：

```text
连接过程
```

命中效果调用独立：

```text
Impact VFX
```

---

# 七、05_Impact 命中模板

这是最重要、复用率最高的一类。

适用：

```text
普通攻击
技能命中
爆炸
枪击
刀击
重击
魔法命中
```

推荐：

```text
NS_Impact_xxx
│
├─ NE_Flash
├─ NE_Core
├─ NE_Shockwave
├─ NE_DirectionalSparks
├─ NE_Debris
└─ NE_Residual
```

时间节奏：

```text
0ms
命中

0~100ms
Flash + Core

50~250ms
Shockwave + Sparks

200~800ms
Debris

300~1200ms
Residual
```

高频 Hit 必须：

```text
短生命周期
Pooling
Instance Limit
低Overdraw
```

默认建议：

```text
Pooling = AutoRelease

EffectType = ET_VFX_Combat
```

---

# 八、06_Area / AOE 模板

适用：

```text
技能范围
持续伤害区域
法阵
火焰地面
毒圈
治疗区域
能量场
```

推荐：

```text
NS_Area_xxx
│
├─ NE_GroundBase
├─ NE_Boundary
├─ NE_Core
├─ NE_AmbientParticles
└─ NE_Pulse
```

必须清楚表达：

```text
中心
边界
范围
危险区域
```

Gameplay Readability 优先于视觉复杂度。

参数：

```text
User.Radius
User.PrimaryColor
User.Intensity
User.LifetimeScale
```

---

# 九、07_Status 状态模板

适用：

```text
Buff
Debuff
护盾
Aura
中毒
燃烧
冰冻
治疗
无敌
```

推荐：

```text
NS_Status_xxx
│
├─ NE_Core
├─ NE_Orbit
├─ NE_BodyFX
└─ NE_Pulse
```

多数属于：

```text
Persistent
Attached
```

必须支持：

```text
StopVFX
StopAllVFXForOwner
```

不要依靠 AutoDestroy 管理长期状态。

---

# 十、08_Space 空间模板

适用：

```text
Teleport
Portal
Warp
Rift
BlackHole
空间裂缝
```

推荐：

```text
NS_Space_xxx
│
├─ NE_Core
├─ NE_Ring
├─ NE_Distortion
├─ NE_Particles
├─ NE_Volume
└─ NE_Residual
```

视觉原则：

```text
收缩
扭曲
旋转
吸附
空间深度
```

Distortion：

```text
PC / Cinematic
允许

VRMobile
优先关闭或简化
```

---

# 十一、09_Lifecycle 生命周期模板

适用：

```text
出生
召唤
变身
死亡
消散
复活
```

推荐拆成：

```text
Spawn

Transform

Death

Dissolve

Revive
```

典型结构：

```text
NS_Lifecycle_xxx
│
├─ NE_Core
├─ NE_BodyTransition
├─ NE_Particles
├─ NE_Ground
└─ NE_Residual
```

必须与角色逻辑分离。

VFX 只负责表现，不负责：

```text
Spawn Actor
Destroy Actor
状态切换
```

---

# 十二、10_Character 模板

适用：

```text
Footstep
Land
Jump
Dash
Weapon Attachment
Eye Glow
Body Energy
Weapon Enchant
```

推荐大量使用通用资产。

例如：

```text
VFX.Character.Motion.Land
VFX.Character.Motion.Dash
VFX.Character.Footstep.Dust
VFX.Weapon.Enchant.Fire
```

避免每个角色重新制作：

```text
Tiger_Land
Dragon_Land
Monkey_Land
```

如果视觉差异仅是颜色：

使用：

```text
User.PrimaryColor
```

解决。

---

# 十三、11_World 环境模板

适用：

```text
Rain
Snow
Fog
Dust
Leaves
Pollen
Waterfall
Fire
Wind
```

Environment 优先：

```text
GPU Simulation
距离裁剪
实例限制
低Overdraw
```

环境 VFX 默认：

```text
Priority = Ambient
```

必须积极 Cull。

禁止：

```text
整个地图所有环境粒子始终完整运行
```

---

# 十四、12_Surface 模板

适用：

```text
弹孔
焦痕
裂纹
脚印
冰霜
魔法残留
```

推荐：

```text
Decal
+
少量Niagara
```

而不是所有效果全部通过粒子实现。

例如：

```text
Impact
↓
瞬时火花
↓
Surface
↓
焦痕Decal
```

长期 Surface 必须：

```text
Fade Out
```

并设置：

```text
最大实例数量
```

---

# 十五、13_Destruction 模板

适用：

```text
碎裂
岩石崩塌
建筑破坏
木材破碎
爆炸残骸
```

推荐组合：

```text
Chaos
+
Niagara
```

职责：

```text
Chaos
负责真实碎块物理

Niagara
负责灰尘
火花
小碎片
烟雾
冲击波
```

避免 Niagara 模拟大量真正应该由 Chaos 管理的大型物理碎块。

---

# 十六、14_Indicator 模板

适用：

```text
Boss红圈
AOE范围
瞄准
落点
锁定
轨迹预测
危险区域
```

最高原则：

```text
可读性 > 华丽程度
```

推荐结构：

```text
NS_Indicator_xxx
│
├─ NE_Base
├─ NE_Border
├─ NE_Direction
└─ NE_Pulse
```

参数：

```text
User.Radius
User.Angle
User.Length
User.PrimaryColor
User.Progress
```

必须在：

```text
复杂战斗背景
亮环境
暗环境
```

仍清晰可见。

---

# 十七、15_UI 模板

适用：

```text
Button FX
HUD FX
Screen Damage
Low Health
Level Up
Reward
Skill Ready
PostProcess
```

必须区分：

```text
World Space
Screen Space
```

VR 项目谨慎使用：

```text
全屏闪白
强Vignette
强Blur
剧烈屏幕Distortion
```

避免造成不适。

---

# 十八、统一 Niagara User Parameter

正式库优先统一：

```text
User.PrimaryColor

User.SecondaryColor

User.Intensity

User.Scale

User.LifetimeScale

User.Direction

User.TargetLocation

User.TeamColor
```

可按类型扩展：

```text
User.Radius

User.Width

User.Length

User.SourceLocation

User.Progress
```

禁止随意创建同义参数。

---

# 十九、统一 VFX Catalog Entry

每个正式 VFX 至少配置：

```text
VFXTag

NiagaraSystem

DisplayName

Description

Category

Element

Form

Context

Lifecycle

Priority

SpawnMode

DefaultSocket

DefaultScale

PoolingMethod

AutoDestroy
```

---

# 二十、统一 Effect Type

正式库统一使用：

```text
ET_VFX_Critical

ET_VFX_Combat

ET_VFX_Cosmetic

ET_VFX_Ambient
```

避免每一个特效创建新的：

```text
ET_xxx
```

---

# 二十一、统一质量等级

所有重要资产至少定义：

```text
High
Medium
Low
VRMobile
```

Hero资产可以：

```text
Cinematic
High
Medium
Low
VRMobile
```

原则：

```text
降级Secondary FX

保留Primary FX
```

---

# 二十二、AI创建新VFX时的固定流程

AI 每创建一个新特效必须执行：

```text
1. 判断Category

2. 判断是否已有类似资源

3. 判断哪些Core资源可以复用

4. 定义GameplayTag

5. 定义视觉核心

6. 定义时间节奏

7. 定义Near/Mid/Far结构

8. 设计Niagara System

9. 列出Emitter及职责

10. 定义Material

11. 定义Texture

12. 定义Mesh

13. 定义User Parameters

14. 配置Catalog

15. 配置Effect Type

16. 配置Pooling

17. 配置Scalability

18. Profile

19. 优化

20. 验收
```

---

# 二十三、AI正式执行提示词

以后创建任何 VFX，都在任务前加入以下规则：

```text
你正在为 Unreal Engine 5.8 跨项目视觉特效库
ModularGameVFX
开发正式生产级VFX。

首先根据 ModularGameVFX 分类规范判断该资源属于：

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

中的哪一类。

然后检查现有：

Core Niagara Module
Master Material
Texture
Mesh
Niagara System

是否可以复用。

禁止创建功能重复资产。

视觉品质要求：

惊艳
震撼
宏大
史诗
具有强打击感
力量感
重量感
速度感
空间感
压迫感

但禁止通过单纯增加粒子数量制造视觉复杂度。

正式重要VFX必须具有：

明确视觉核心
明确运动方向
明确时间节奏
明确Primary FX
明确Secondary FX

重要战斗VFX采用：

蓄势
→
主爆发
→
二级冲击
→
余波
→
残留

空间采用：

Near
+
Mid
+
Far

三级结构。

所有VFX必须通过：

GameplayTag
→
VFX Catalog
→
Soft Reference
→
VFX Manager
→
Niagara

调用。

不得让业务硬引用Niagara。

必须设置：

Category
Element
Form
Context
Lifecycle
Priority

并指定：

Pooling
Effect Type
Scalability
High/Medium/Low/VRMobile

性能不足时优先删除Secondary Detail。

不得先删除：

核心轮廓
主冲击
技能方向
Gameplay范围。

完成后必须检查：

GPU Cost
CPU Cost
Particle Count
System Count
Quad Overdraw
Shader Complexity
Transparency
Distortion
Dynamic Light
Collision
Ribbon
Mesh Particle
Texture Memory

最后输出完整：

GameplayTag
Catalog配置
Niagara结构
Emitter结构
Material
Texture
Mesh
User Parameters
Pooling
Effect Type
Scalability
资产命名
目录
UE5.8实现步骤
性能风险
优化结果。
```

---

# 二十四、最终目标

最终 `ModularGameVFX` 应形成：

```text
分类标准
+
Core基础积木库
+
标准Niagara模板
+
统一GameplayTag
+
VFX Catalog
+
VFX Manager
+
Effect Type
+
Scalability
+
AI自动开发规范
```

以后无论创建：

```text
火焰
雷电
爆炸
技能
角色
Boss
VR
科幻
魔法
现代战争
```

都遵循同一套资产结构和工程规范。

最终达到：

```text
视觉高品质
+
结构统一
+
快速生产
+
高复用率
+
低重复资产
+
性能可控
+
跨项目迁移
```

的目标。
