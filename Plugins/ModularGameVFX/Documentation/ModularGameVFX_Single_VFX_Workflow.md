# ModularGameVFX 单个 VFX 自动创建工作流

## 一、目标

以后用户只需要输入：

```text
创建一个史诗级雷电重击VFX
```

AI 应自动完成：

```text
需求理解
↓
VFX分类
↓
Core复用检查
↓
视觉设计
↓
Niagara结构
↓
GameplayTag
↓
VFX Catalog注册
↓
VFX Manager调用方案
↓
Effect Type
↓
Pooling
↓
Scalability
↓
性能检查
↓
测试
↓
验收
```

不得只输出视觉描述。

---

# 二、第一步：解析需求

首先提取：

```text
用途
元素
视觉等级
生命周期
使用位置
是否高频
是否持续
是否附着
是否需要Gameplay范围提示
目标平台
```

例如：

```text
史诗级雷电重击
```

自动推导：

```text
Category = Impact
Element = Lightning
Form = HeavyImpact
Context = Combat
Lifecycle = OneShot
Priority = Critical
```

---

# 三、第二步：判断是否可以复用已有VFX

创建前必须搜索：

```text
完整Niagara System
Emitter
Niagara Module
Master Material
Material Instance
Texture
Mesh
Curve
Effect Type
```

优先级：

```text
已有完整System
↓
参数化复用

已有Emitter
↓
组合

已有Module
↓
组合

已有材质和纹理
↓
复用

确实缺失
↓
创建新资产
```

禁止：

```text
未搜索Core
→
直接创建新资产
```

---

# 四、第三步：确定GameplayTag

按照：

```text
VFX.<Category>.<Element>.<Name>
```

生成。

例如：

```text
VFX.Impact.Lightning.Heavy
```

如果属于项目专属技能：

```text
VFX.Character.Tiger.Skill01.Hit
```

Catalog 中仍可以记录：

```text
Category = Impact
Element = Lightning
```

业务 Tag 和资源分类允许不同维度并存。

---

# 五、第四步：建立视觉核心

任何 VFX 首先明确：

```text
视觉中心是什么？
```

例如雷电重击：

```text
中心：
高亮雷电核心

主方向：
攻击方向向前

主冲击：
命中点瞬间爆发

空间扩散：
径向冲击环

Secondary：
电弧、火花、碎片

Residual：
短时间电流残留
```

如果无法用一句话描述视觉核心：

重新设计。

---

# 六、第五步：建立时间轴

重要 Combat / Critical VFX 默认：

```text
Anticipation
↓
Primary Impact
↓
Secondary Motion
↓
Residual
```

例如雷电重击：

```text
0.00s
短暂能量压缩

0.03s
白蓝强闪

0.05s
雷电核心爆发

0.08s
冲击环扩散

0.10～0.30s
电弧、火花、碎片飞散

0.30～0.80s
地面电流残留

0.80s后
快速消失
```

---

# 七、第六步：建立近中远空间层级

标准结构：

```text
Near
核心高亮、电弧、碎片

Mid
主要冲击波、能量主体

Far
大型低频冲击环、环境响应
```

原则：

```text
Near负责细节
Mid负责主体
Far负责尺度
```

---

# 八、第七步：Niagara System设计

例如：

```text
NS_Lightning_HeavyImpact
```

建议：

```text
NE_Flash
NE_Core
NE_Shockwave
NE_LightningArc
NE_Sparks
NE_Debris
NE_GroundResidual
```

每个 Emitter 必须说明职责。

例如：

```text
NE_Flash
负责命中瞬间亮度峰值

NE_Core
负责主要雷电爆发主体

NE_Shockwave
负责大尺度力量传播

NE_LightningArc
负责雷元素识别

NE_Sparks
负责高频打击细节

NE_Debris
负责重量反馈

NE_GroundResidual
负责结束后的短暂残留
```

---

# 九、第八步：复用Core模块

优先检查：

```text
NM_VFX_RadialBurst
NM_VFX_DirectionalBurst
NM_VFX_ScaleOverLife
NM_VFX_ColorOverLife
NM_VFX_FadeInOut
NM_VFX_RandomRotation
NM_VFX_GroundRing
```

能够参数化解决的：

禁止重新创建。

例如：

```text
NE_Sparks
=
NE_VFX_Sparks
+
Lightning Material Instance
+
BlueWhite Color
```

而不是重新创建一个全新的 Sparks Emitter。

---

# 十、第九步：材质方案

优先使用：

```text
M_VFX_Additive_Master
M_VFX_Translucent_Master
M_VFX_Distortion_Master
M_VFX_Decal_Master
```

然后建立：

```text
MI_VFX_Lightning_HeavyImpact
```

主要参数：

```text
PrimaryColor
SecondaryColor
EmissiveIntensity
UVSpeed
DistortionStrength
Opacity
```

原则：

```text
优先Material Instance
禁止复制Master Material
```

---

# 十一、第十步：纹理和Mesh

优先复用：

```text
Noise
Gradient
Radial Mask
Lightning Mask
Shockwave Mesh
Ring Mesh
Debris Mesh
```

只有视觉语言确实缺失时才创建新资源。

例如：

```text
SM_VFX_Shockwave
```

可以服务：

```text
Fire
Lightning
Holy
Dark
Energy
```

主要通过 Material 改变表现。

---

# 十二、第十一步：统一User Parameters

至少检查是否需要：

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

例如：

```text
User.PrimaryColor = 电蓝

User.SecondaryColor = 白

User.Intensity = 1.4
```

不要创建：

```text
User.LightningBlue
User.BossColor
```

这类项目专用参数。

---

# 十三、第十二步：Catalog注册

创建或更新：

```text
VFX Catalog
```

例如：

```text
GameplayTag
VFX.Impact.Lightning.Heavy

NiagaraSystem
NS_Lightning_HeavyImpact

Category
Impact

Element
Lightning

Form
HeavyImpact

Context
Combat

Lifecycle
OneShot

Priority
Critical

SpawnMode
AtLocation

Pooling
AutoRelease

AutoDestroy
true
```

必须使用：

```text
Soft Reference
```

禁止 Catalog 硬引用整个视觉库。

---

# 十四、第十三步：VFX Manager调用

业务最终只允许：

```text
Play VFX At Location
```

输入：

```text
VFXTag =
VFX.Impact.Lightning.Heavy

Location =
HitResult.Location

Rotation =
ImpactDirection

PrimaryColor =
BlueWhite

Intensity =
1.4
```

业务不得直接引用：

```text
NS_Lightning_HeavyImpact
```

---

# 十五、第十四步：Pooling策略

判断：

```text
是否高频？
是否一次性？
是否持续？
```

规则：

```text
高频OneShot
→ AutoRelease

持续Buff
→ ManualRelease

环境长期循环
→ 根据Manager生命周期管理
```

普通 Hit / Impact 默认优先：

```text
AutoRelease
```

---

# 十六、第十五步：Effect Type

根据优先级选择：

```text
Critical
→ ET_VFX_Critical

Combat
→ ET_VFX_Combat

Cosmetic
→ ET_VFX_Cosmetic

Ambient
→ ET_VFX_Ambient
```

不要每个 VFX 新建 Effect Type。

---

# 十七、第十六步：Scalability

所有重要 VFX 至少设计：

```text
High
Medium
Low
VRMobile
```

例如雷电重击：

## High

```text
全部Emitter
完整电弧
完整碎片
少量Distortion
```

## Medium

```text
减少Secondary Arc
减少Sparks
减少Debris
```

## Low

```text
保留：
Core
Flash
Shockwave

明显减少：
Arc
Sparks
Residual
```

## VRMobile

```text
保留：
Core
Flash
简单Shockwave

关闭或显著减少：
Distortion
Debris
高频Ribbon
大型透明粒子
```

---

# 十八、第十七步：性能检查

必须检查：

```text
System Instance Count

Emitter Count

Particle Count

CPU/GPU Simulation

Quad Overdraw

Shader Complexity

Transparent Layers

Distortion

Dynamic Light

Collision

Ribbon

Mesh Particle

Texture Memory

Fixed Bounds
```

任何正式 VFX 不允许直接写：

```text
性能良好
```

必须基于实际 Profile 或明确标记：

```text
尚待实机Profile
```

---

# 十九、第十八步：视觉检查

必须判断：

```text
第一眼是否有明确视觉中心？

是否具有强打击感？

是否能感受到攻击方向？

是否有明显速度变化？

是否有Near/Mid/Far层级？

是否存在视觉高潮？

结束是否干净？

是否存在无意义粒子？
```

如果视觉只靠：

```text
Bloom
+
大量Particles
```

重新设计。

---

# 二十、第十九步：Gameplay Readability检查

必须验证：

```text
是否遮挡敌人？

是否遮挡玩家角色？

是否看得到命中位置？

是否看得到技能范围？

是否会误判敌我？

是否残留过久？
```

Gameplay信息优先。

---

# 二十一、第二十步：命名与目录

例如：

```text
05_Impact/
└─ Lightning/
    └─ Heavy/
        ├─ NS_Lightning_HeavyImpact
        └─ MI_VFX_Lightning_HeavyImpact
```

但不要为了元素建立过深目录。

基础资产继续放：

```text
00_Core
```

---

# 二十二、单VFX完整验收

创建完成必须满足：

```text
有GameplayTag

有Catalog Entry

通过VFX Manager调用

没有业务Hard Reference

优先复用Core

命名规范

Effect Type正确

Pooling正确

Scalability存在

性能风险明确

可在其他项目复用
```

---

# 二十三、AI自动执行主提示词

以后可以直接使用以下提示词：

```text
请严格遵循当前项目中的：

ModularGameVFX分类规范
ModularGameVFX Core基础积木库规范
ModularGameVFX视觉质量规范
ModularGameVFX性能规范
VFX Catalog规范
VFX Manager规范。

现在创建：

【VFX】
{填写特效名称}

首先自动判断：

Category
Element
Form
Context
Lifecycle
Priority

然后搜索当前ModularGameVFX资产库。

必须优先复用：

已有Niagara System
Emitter
Niagara Module
Master Material
Material Function
Texture
Mesh
Curve
Effect Type。

如果可以参数化复用，禁止创建重复资产。

然后完成：

1. GameplayTag设计
2. 视觉核心设计
3. 时间轴设计
4. Near/Mid/Far空间设计
5. Niagara System设计
6. Emitter结构
7. Core模块复用
8. Material设计
9. Texture需求
10. Mesh需求
11. User Parameters
12. Catalog Entry
13. VFX Manager调用方式
14. Pooling
15. Effect Type
16. High/Medium/Low/VRMobile
17. Fixed Bounds
18. Instance Limit
19. 性能风险
20. 优化方案
21. 测试方案
22. 最终验收。

视觉品质要求：

惊艳、
震撼、
宏大、
史诗、
强打击感、
力量感、
重量感、
速度感、
空间感、
压迫感。

但禁止：

单纯增加粒子数量、
Bloom滥用、
透明层滥用、
大量动态光、
大量Distortion、
大量CPU Collision、
无意义Emitter。

如果性能和视觉发生冲突：

优先保留：

核心轮廓、
主冲击、
运动方向、
Gameplay范围。

优先减少：

Secondary Particle、
Smoke、
Debris、
额外Ribbon、
Distortion、
Particle Light、
Residual。

必须真实在Unreal Engine 5.8 Niagara中可实现。

如果当前AI环境能够操作UE项目：

直接创建和修改资产或代码。

如果无法创建二进制.uasset：

不要伪造资产。

请明确输出需要在UE编辑器人工完成的步骤。

最后给出：

创建资产列表
复用资产列表
GameplayTag
Catalog配置
Niagara结构
性能等级
测试结果
尚未完成事项。
```

---

# 二十四、超简版调用模板

如果全局规范已经写入 AI Context，以后只需要：

```text
按 ModularGameVFX 全局规范创建：

VFX名称：
雷电重击

用途：
角色重型攻击命中

视觉等级：
Critical

要求：
史诗、震撼、强打击感、强压迫感。

目标平台：
PC + VRMobile

请自动完成分类、Core复用、Niagara设计、Catalog注册、Manager调用、性能配置和测试。
```

即可。

---

# 二十五、AI自动生产链最终形态

最终希望达到：

```text
用户输入
“创建一个史诗级雷电重击”
            ↓
AI理解需求
            ↓
分类和Metadata
            ↓
搜索Core
            ↓
复用已有资产
            ↓
创建缺失部分
            ↓
创建Niagara
            ↓
GameplayTag
            ↓
Catalog注册
            ↓
Effect Type
            ↓
Pooling
            ↓
Scalability
            ↓
Manager调用验证
            ↓
Profile
            ↓
验收
```

整个流程不再依赖人工逐项提醒。
