# ModularGameVFX Showcase 测试关卡与人工验收系统

## 1. 建设目标

建立一个独立的 VFX 测试与展示环境：

```text
ModularGameVFXShowcase
```

用于：

```text
浏览全部VFX
按分类筛选
搜索VFX
选择具体技能/特效
点击菜单立即施放
重复播放
停止特效
修改颜色/强度/缩放
切换质量等级
测试不同背景和光照
查看性能信息
人工核验
记录通过/失败
批量压力测试
```

最终形成：

```text
VFX Catalog
      ↓
自动读取分类
      ↓
生成分类菜单
      ↓
选择分类
      ↓
生成该分类下的技能/VFX菜单
      ↓
点击VFX
      ↓
VFX Manager
      ↓
测试场景施放
      ↓
人工视觉核验
      ↓
性能核验
      ↓
PASS / FAIL
```

---

# 2. 模块划分

推荐：

```text
Plugins/
│
├─ ModularGameVFX/
│
│   ├─ VFX Catalog
│   ├─ VFX Manager
│   ├─ Niagara
│   └─ Runtime API
│
└─ ModularGameVFXShowcase/
    │
    ├─ Maps/
    ├─ UI/
    ├─ Blueprints/
    ├─ TestActors/
    ├─ Profiles/
    └─ Data/
```

依赖关系：

```text
ModularGameVFXShowcase
        ↓
ModularGameVFX
```

禁止：

```text
ModularGameVFX
        ↓
ModularGameVFXShowcase
```

Showcase 是可删除的开发验证模块。

---

# 3. 测试关卡

创建：

```text
L_VFX_Showcase
```

建议整体布局：

```text
┌─────────────────────────────────────────────┐
│                VFX测试控制UI                 │
├─────────┬────────────┬──────────────────────┤
│ 分类菜单 │ 技能/VFX菜单 │      参数控制         │
│         │            │                      │
│ Impact  │ Heavy Hit  │ Color                │
│ Beam    │ Fire Hit   │ Intensity            │
│ Area    │ Ice Hit    │ Scale                │
│ ...     │ ...        │ Quality              │
├─────────┴────────────┴──────────────────────┤
│                                             │
│                 VFX测试场                   │
│                                             │
│     Player → Spawn Point → Target Dummy     │
│                                             │
├─────────────────────────────────────────────┤
│      播放 / Stop / Loop / 压测 / 验收        │
└─────────────────────────────────────────────┘
```

---

# 4. 测试场景结构

至少创建：

```text
BP_VFX_ShowcaseController

BP_VFX_TestOrigin

BP_VFX_TestTarget

BP_VFX_TestCharacter

BP_VFX_TestProjectileTarget

BP_VFX_ShowcaseEnvironment
```

核心测试点：

```text
Origin
=
特效起点

Target
=
目标点

Ground
=
地面命中点

Character
=
角色附着测试

WeaponSocket
=
武器/手部Socket测试
```

---

# 5. 测试距离

场景设置几个标准测试点：

```text
Near
约2～3米

Medium
约10米

Far
约30～50米
```

用于观察：

```text
近距离细节

中距离核心轮廓

远距离可读性

距离Cull
```

---

# 6. 场景背景切换

至少准备：

```text
Neutral
中性灰背景

Bright
明亮户外

Dark
暗环境

Complex
复杂战斗背景
```

UI提供：

```text
Background
```

切换按钮。

目的是避免：

```text
黑背景看起来很好
进入真实地图以后完全看不清
```

---

# 7. 分类菜单

左侧建立：

```text
WBP_VFX_CategoryMenu
```

自动从 Catalog Metadata 读取分类。

一级菜单固定显示：

```text
All

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

可增加：

```text
Favorites
Recent
Failed
Unreviewed
```

---

# 8. 分类菜单不得手工维护

禁止在 Widget 中写：

```text
Cast Button
Impact Button
Beam Button
...
```

然后逐个绑定。

应该：

```text
VFX Catalog
      ↓
读取Category
      ↓
去重
      ↓
排序
      ↓
动态创建分类菜单
```

这样以后增加：

```text
VFX.Category.NewType
```

测试系统能够自动识别。

---

# 9. 技能/VFX菜单

创建：

```text
WBP_VFX_EntryList
```

当用户点击：

```text
Impact
```

右侧自动显示：

```text
Impact
│
├─ Fire Light Impact
├─ Fire Heavy Impact
├─ Ice Shatter Impact
├─ Lightning Heavy Impact
├─ Physical Sword Impact
└─ ...
```

点击：

```text
Beam
```

变为：

```text
Beam
│
├─ Lightning Chain
├─ Energy Beam
├─ Healing Beam
└─ Dark Link
```

---

# 10. 菜单数据必须来自Catalog

筛选逻辑：

```text
Selected Category
        ↓
VFX Catalog
        ↓
Filter
        ↓
Matching Entries
        ↓
VFX Entry List
```

例如：

```text
Category == Impact
```

返回所有：

```text
FVFXCatalogEntry
```

中 Category 为 Impact 的资源。

---

# 11. 技能菜单条目

创建：

```text
WBP_VFX_EntryItem
```

显示：

```text
Display Name

GameplayTag

Element

Form

Priority

Review Status
```

例如：

```text
雷电重击

VFX.Impact.Lightning.Heavy

Lightning
HeavyImpact

Critical

Unreviewed
```

菜单尽量简洁。

详细信息放到 Inspector。

---

# 12. 搜索功能

增加：

```text
Search
```

支持按：

```text
DisplayName

GameplayTag

Element

Form
```

查找。

例如输入：

```text
Lightning
```

显示全部雷元素效果。

输入：

```text
Impact
```

显示全部命中特效。

---

# 13. Element二级筛选

分类之外增加：

```text
Element Filter
```

例如：

```text
All

Fire
Water
Ice
Lightning
Wind
Earth
Light
Dark
Poison
Energy
Tech
Physical
```

例如：

```text
Category = Impact
Element = Lightning
```

最终菜单只显示：

```text
雷电命中类VFX
```

---

# 14. 点击技能立即施放

点击：

```text
WBP_VFX_EntryItem
```

后：

```text
GameplayTag
      ↓
BP_VFX_ShowcaseController
      ↓
ModularGameVFX Manager
      ↓
PlayVFX
      ↓
测试场景
```

严禁测试界面直接：

```text
SpawnSystemAtLocation(NS_xxx)
```

测试系统也必须经过正式：

```text
GameplayTag
→ Catalog
→ VFX Manager
```

调用链。

这样才能同时验证真正运行架构。

---

# 15. 根据VFX类型自动选择播放方式

Showcase Controller 根据 Catalog Metadata 判断。

例如：

## Impact

```text
TargetLocation
```

播放。

---

## Cast

```text
TestCharacter
hand_r
```

附着。

---

## Projectile

```text
Origin
↓
Target
```

飞行。

---

## Trail

```text
播放测试攻击动画
```

或：

```text
移动测试武器
```

---

## Beam

```text
Origin
↓
Target
```

---

## Area

```text
GroundCenter
```

---

## Status

```text
Attach TestCharacter
```

---

## Space

```text
Portal Origin
或
Teleport Origin → Target
```

---

## World

```text
Environment Test Zone
```

---

## UI

```text
Screen Preview Layer
```

---

# 16. 推荐增加Preview Mode

Catalog 或 Showcase Profile 增加：

```text
PreviewMode
```

例如：

```text
AtLocation

Attached

Projectile

Beam

Area

Screen

Environment
```

Showcase 根据 PreviewMode 自动决定测试方式。

---

# 17. 不建议把测试数据全部塞进Runtime Catalog

推荐另外建立：

```text
UVFXShowcaseProfile
```

保存：

```text
VFXTag

PreviewMode

PreviewDistance

PreviewDuration

DefaultCamera

TargetType

LoopAllowed

RecommendedBackground
```

Catalog负责：

```text
正式运行数据
```

Showcase Profile负责：

```text
测试展示数据
```

避免 Runtime Catalog 被大量开发字段污染。

---

# 18. 播放控制区

提供：

```text
Play

Replay

Stop

Stop Immediate

Loop

Auto Replay
```

其中：

```text
Replay
```

执行：

```text
Stop Previous
↓
重新Play
```

便于反复人工观察。

---

# 19. Slow Motion

增加：

```text
Playback Speed
```

建议：

```text
0.1x

0.25x

0.5x

1.0x
```

用于检查：

```text
Hit Flash

Shockwave

Debris

Trail

时间节奏
```

尤其适合检查 50～200ms 的瞬时 Hit。

---

# 20. 参数控制面板

创建：

```text
WBP_VFX_ParameterPanel
```

至少提供：

```text
Primary Color

Secondary Color

Intensity

Scale

Lifetime Scale
```

根据资源支持情况显示：

```text
Radius

Width

Length

Progress
```

所有参数通过：

```text
VFX Manager
```

传入。

---

# 21. 参数Reset

必须有：

```text
Reset
```

恢复：

```text
Catalog Default
```

避免测试人员调过参数以后误以为是原始效果。

---

# 22. Quality切换

增加：

```text
Quality
```

菜单：

```text
Cinematic
High
Medium
Low
VRMobile
```

切换以后重新播放当前 VFX。

用于直观看：

```text
Secondary Particle是否合理减少

核心轮廓是否保持

低质量模式是否仍可识别
```

Niagara 本身具有质量等级和 Scalability 配置能力，因此这部分应该验证真实 Niagara 降级，而不是使用几张模拟截图。UE5.8 Emitter Scalability 支持按平台设置 Spawn Count Scale 等参数。

---

# 23. Inspector信息面板

选择 VFX 后显示：

```text
DisplayName

GameplayTag

Asset

Category

Element

Form

Context

Lifecycle

Priority

Pooling

EffectType

SpawnMode

ReviewStatus
```

同时显示：

```text
Niagara System Path
```

但只用于开发者查看。

游戏业务仍不能依赖该路径。

---

# 24. VFX结构信息

如果可以获得：

显示：

```text
Emitter Count

Simulation Target

Fixed Bounds

Effect Type
```

用于快速检查规范。

---

# 25. 性能面板

创建：

```text
WBP_VFX_PerformancePanel
```

至少提供当前测试状态：

```text
FPS

Frame Time

Active VFX Count

Managed VFX Count

Active Niagara Systems

Current Quality

Current Test Count
```

如果可以可靠获取：

继续显示：

```text
CPU Niagara Cost

GPU Niagara Cost

Particle Count
```

不能可靠获得的数据：

不要伪造。

---

# 26. 单特效模式

默认：

```text
Single
```

每次只播放一个效果。

切换资源时：

```text
Stop Previous
↓
Clear
↓
Play Selected
```

适合视觉验收。

---

# 27. Compare模式

增加：

```text
Compare
```

允许选择：

```text
VFX A
VFX B
```

并排播放。

例如比较：

```text
Fire Heavy Impact V1

Fire Heavy Impact V2
```

用于版本选择。

---

# 28. Stress Test模式

增加：

```text
Stress Test
```

提供：

```text
1

5

10

20

50

100
```

实例按钮。

例如选择：

```text
VFX.Impact.Fire.Light
```

点击：

```text
50
```

则在测试区域按网格或随机点生成 50 次。

---

# 29. Stress Test不能污染视觉测试

压力测试区域与单特效展示区分离。

推荐：

```text
ShowcaseZone
```

和：

```text
StressTestZone
```

分别存在。

避免测试完 100 个 Impact 后影响普通人工观察。

---

# 30. Combat Simulation

增加：

```text
Combat Simulation
```

自动生成：

```text
Projectile
Impact
Trail
Status
Area
```

组合。

用于验证：

```text
真实战斗中是否视觉过载
```

而不是只看一个特效单独播放。

---

# 31. 人工核验面板

建立：

```text
WBP_VFX_ReviewPanel
```

提供：

```text
PASS

FAIL

Needs Optimization

Needs Visual Rework

Not Tested
```

---

# 32. 人工核验内容

人工检查：

```text
视觉中心

视觉冲击

打击感

力量感

速度感

空间层次

颜色

轮廓

时间节奏

Gameplay可读性

结束反馈

性能
```

---

# 33. 快速验收按钮

推荐：

```text
Visual PASS

Performance PASS

Gameplay PASS

Final PASS
```

只有：

```text
Visual PASS
+
Performance PASS
+
Gameplay PASS
```

全部通过：

最终状态才能：

```text
Production Ready
```

---

# 34. Fail原因

点击 FAIL 后支持选择：

```text
视觉平淡

缺少打击感

层次不足

颜色问题

过度Bloom

遮挡严重

生命周期过长

Overdraw高

粒子过多

性能过高

VR版本问题

分类错误

Catalog错误

其他
```

并支持填写：

```text
Review Notes
```

---

# 35. Review数据不要直接修改正式Niagara

测试UI不应该直接改变：

```text
Niagara资产
```

保存的只是：

```text
ReviewResult
```

例如：

```text
VFXTag
Status
VisualResult
PerformanceResult
GameplayResult
Notes
Date
```

---

# 36. 建立Review Data

可以创建：

```text
UVFXReviewData
```

或开发环境中的等效数据结构。

保存：

```text
GameplayTag

ReviewStatus

VisualScore

PerformanceScore

GameplayScore

Notes
```

---

# 37. 自动跳转下一个

验收完成：

```text
PASS
```

或者：

```text
FAIL
```

后提供：

```text
Next
```

自动进入当前分类的下一个 VFX。

实现：

```text
逐个快速验收
```

---

# 38. Auto Preview模式

增加：

```text
Auto Preview
```

例如：

```text
Impact分类
↓
Fire Light
↓
播放
↓
3秒
↓
Fire Heavy
↓
播放
↓
3秒
↓
Ice Impact
↓
...
```

适合整体浏览视觉库。

---

# 39. 分类完成进度

分类菜单可显示：

```text
Impact
12 / 20 Reviewed

Projectile
8 / 15 Reviewed

Beam
6 / 6 Reviewed
```

便于发现：

```text
哪些资产尚未核验
```

---

# 40. 分类菜单状态标记

建议：

```text
● Production Ready

● Needs Review

● Failed
```

无需过度设计颜色逻辑。

核心是状态清晰。

---

# 41. VFX菜单排序

默认：

```text
Category
↓
Element
↓
DisplayName
```

支持：

```text
Name

Priority

Element

ReviewStatus
```

排序。

---

# 42. 收藏功能

增加：

```text
Favorite
```

用于：

```text
重点比较
Hero候选
需要复查
```

属于开发辅助功能，不进入 Runtime。

---

# 43. Test Character

建议测试关卡提供一个通用：

```text
SK_VFX_TestCharacter
```

至少有：

```text
root
pelvis
spine
head
hand_l
hand_r
foot_l
foot_r
weapon
```

标准 Socket。

用于测试：

```text
Cast
Status
Weapon
Trail
Character
```

---

# 44. Trail专用测试动画

至少准备：

```text
Swing Horizontal

Swing Vertical

Spin

Dash

Jump

Land
```

供：

```text
Trail
Character Motion
Weapon VFX
```

测试。

---

# 45. Projectile测试

Projectile Preview：

```text
Origin
        ↓
    20m距离
        ↓
Target Dummy
```

测试：

```text
Projectile主体

Trail

Hit

Projectile速度

远距离轮廓
```

---

# 46. Beam测试

Beam Preview：

```text
Source
────────────
Target
```

允许调整：

```text
Distance
```

例如：

```text
5m
10m
20m
30m
```

观察：

```text
Beam UV

Noise

宽度

稳定性
```

---

# 47. Area测试

提供标准地面网格：

```text
1m
2m
5m
10m
```

帮助判断：

```text
AOE Radius
```

视觉是否与 Gameplay 范围匹配。

---

# 48. Surface测试

建立：

```text
Stone

Metal

Wood

Ground

Wall
```

测试表面。

用于：

```text
Decal

Scorch

BulletHole

Crack

Frost

Residue
```

---

# 49. World测试

建立一个：

```text
Environment Zone
```

测试：

```text
Rain
Snow
Dust
Leaves
Fog
Embers
```

同时支持移动相机观察不同距离。

---

# 50. UI测试区

UI类 VFX 单独进入：

```text
UI Preview Mode
```

测试：

```text
Button

HUD

Damage

LowHealth

SkillReady

Reward

ScreenFX
```

不要在世界 SpawnPoint 中播放。

---

# 51. VR测试模式

增加：

```text
VR Preview Mode
```

自动使用：

```text
VRMobile Quality
```

重点检查：

```text
是否过亮

是否近距离遮挡

是否出现强烈全屏闪烁

Distortion是否安全

透明层成本

左右眼表现
```

---

# 52. 自动验证

Showcase 启动或刷新时自动检查 Catalog：

```text
Invalid Tag

Duplicate Tag

Missing Niagara

Missing Effect Type

Invalid Category

Missing Preview Profile
```

Niagara Effect Type 本身支持 Validation Rules，因此通用 Niagara 内容规范也可以通过 Effect Type 验证层进一步检查。

---

# 53. VFX入库流程

最终正式流程：

```text
创建VFX
↓
注册Catalog
↓
创建Preview Profile
↓
自动进入Showcase菜单
↓
人工视觉核验
↓
Gameplay核验
↓
压力测试
↓
性能核验
↓
PASS
↓
Production Ready
```

任何正式资产原则上不得绕过 Showcase 验收。

---

# 54. Showcase菜单最终结构

推荐：

```text
VFX SHOWCASE
│
├─ All
│
├─ Cast
│   ├─ Fire
│   ├─ Ice
│   ├─ Lightning
│   └─ ...
│
├─ Projectile
│
├─ Trail
│
├─ Beam
│
├─ Impact
│
├─ Area
│
├─ Status
│
├─ Space
│
├─ Lifecycle
│
├─ Character
│
├─ World
│
├─ Surface
│
├─ Destruction
│
├─ Indicator
│
└─ UI
```

选择：

```text
Impact
```

显示：

```text
Fire Light Impact
Fire Heavy Impact

Ice Light Impact
Ice Heavy Impact

Lightning Light Impact
Lightning Heavy Impact
...
```

点击：

```text
Lightning Heavy Impact
```

执行：

```text
VFX.Impact.Lightning.Heavy
        ↓
VFX Manager
        ↓
Target Dummy
        ↓
NS_Lightning_HeavyImpact
```

完成真正的运行链验证。

---

# 55. AI执行提示词

你现在是一名 Unreal Engine 5.8 C++ 工程师、UMG UI工程师、Niagara技术美术和VFX QA工程师。

请在现有 ModularGameVFX 架构基础上创建独立：

```text
ModularGameVFXShowcase
```

测试与人工验收模块。

不要重新实现：

```text
VFX Catalog

VFX Manager
```

必须复用现有：

```text
GameplayTag
→
VFX Catalog
→
VFX Manager
→
Niagara
```

完整正式运行链。

## 创建测试关卡

创建或规划：

```text
L_VFX_Showcase
```

包含：

```text
VFX Origin

Target Dummy

Test Character

Ground Test Area

Projectile Test Lane

Beam Test Lane

Area Test Grid

Surface Test Wall

Environment Test Zone

Stress Test Zone
```

## 创建UI

创建：

```text
WBP_VFX_Showcase

WBP_VFX_CategoryMenu

WBP_VFX_EntryList

WBP_VFX_EntryItem

WBP_VFX_Inspector

WBP_VFX_ParameterPanel

WBP_VFX_PerformancePanel

WBP_VFX_ReviewPanel
```

## 分类菜单

分类数据不得硬编码在Widget事件逻辑中。

根据 Catalog Metadata 自动生成：

```text
All
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

选择分类以后：

自动查询 Catalog。

只显示对应：

```text
FVFXCatalogEntry
```

## 技能/VFX菜单

技能菜单必须自动显示当前分类下全部VFX。

每个条目至少显示：

```text
DisplayName

GameplayTag

Element

Priority

ReviewStatus
```

点击条目立即调用：

```text
VFX Manager
```

施放对应 VFX。

禁止：

```text
Widget
→
直接引用UNiagaraSystem
```

## 自动Preview

根据：

```text
PreviewMode
```

判断：

```text
AtLocation

Attached

Projectile

Beam

Area

Screen

Environment
```

并自动选择正确测试位置和方式。

## Showcase Profile

建立开发测试数据：

```text
UVFXShowcaseProfile
```

记录：

```text
VFXTag

PreviewMode

PreviewDuration

PreviewDistance

TargetType

DefaultCamera

LoopAllowed
```

不要把大量测试字段污染 Runtime Catalog。

## 控制功能

必须支持：

```text
Play

Replay

Stop

Stop Immediate

Loop

Auto Preview

Previous

Next
```

## 参数

支持：

```text
PrimaryColor

SecondaryColor

Intensity

Scale

LifetimeScale
```

并根据资源能力支持：

```text
Radius
Width
Length
```

## Quality

支持切换：

```text
Cinematic

High

Medium

Low

VRMobile
```

切换以后重新播放当前特效。

必须测试真正 Niagara Scalability。

不要使用假的视觉模拟。

## 场景环境

至少支持：

```text
Neutral

Bright

Dark

Complex
```

四种测试背景。

## 性能

显示能够可靠获得的：

```text
FPS

FrameTime

ManagedVFXCount

ActiveVFXCount

Quality
```

如果无法可靠获得某些 Niagara GPU/CPU数据：

不要伪造。

## Stress Test

支持：

```text
1
5
10
20
50
100
```

实例压力测试。

压力测试只使用：

```text
VFX Manager
```

创建实例。

## Review

人工核验支持：

```text
PASS

FAIL

Needs Optimization

Needs Visual Rework

Not Tested
```

分别评估：

```text
Visual

Gameplay

Performance
```

只有三项全部通过：

```text
Production Ready
```

## Fail原因

至少支持：

```text
视觉平淡

打击感不足

层次不足

颜色问题

过度Bloom

遮挡严重

生命周期问题

Overdraw

粒子过多

性能过高

VR问题

分类错误

Catalog错误
```

## 自动过滤

增加：

```text
Search

Category Filter

Element Filter

Review Status Filter
```

例如：

```text
Impact
+
Lightning
+
Failed
```

只显示：

```text
尚未通过的雷元素Impact
```

## Test流程

最终完整运行：

```text
启动L_VFX_Showcase
        ↓
加载VFX Catalog
        ↓
自动生成分类菜单
        ↓
用户点击Impact
        ↓
自动生成Impact技能列表
        ↓
用户点击Lightning Heavy Impact
        ↓
读取GameplayTag
        ↓
调用VFX Manager
        ↓
在Target Dummy播放
        ↓
用户切换High / Low / VRMobile
        ↓
人工观察
        ↓
执行Stress Test
        ↓
检查Performance
        ↓
PASS / FAIL
        ↓
保存Review结果
        ↓
Next
```

## 代码原则

禁止：

```text
测试系统复制VFX播放逻辑

测试系统直接Spawn Niagara

测试系统硬引用具体项目角色

测试系统依赖神兽联盟代码
```

必须保持：

```text
ModularGameVFXShowcase
        ↓
ModularGameVFX
```

跨项目可使用。

## 资产自动发现

新 VFX 只要：

```text
注册VFX Catalog
+
拥有有效Category
```

就应该自动出现在 Showcase 分类菜单。

禁止每增加一个VFX：

```text
手工增加一个UMG Button
```

## 编译与验证

如果当前环境能够操作UE5.8：

直接实施并实际编译。

如果不能生成二进制 `.uasset`：

不要伪造文件。

完成：

```text
C++框架
数据结构
Widget逻辑设计
蓝图节点逻辑
目录
命名
配置
```

并明确列出需要在 Unreal Editor 中人工创建的资产。

最终输出：

```text
1. 创建文件
2. 修改文件
3. Showcase目录
4. Test Map结构
5. UI结构
6. 分类菜单逻辑
7. 技能菜单逻辑
8. Preview逻辑
9. Performance测试
10. Review流程
11. Blueprint调用流程
12. 编译结果
13. 尚需人工完成内容
```

最终目标不是制作一个简单的 Niagara Gallery。

而是建立：

```text
VFX浏览器
+
技能施放测试器
+
参数调试器
+
质量等级对比器
+
压力测试器
+
人工验收系统
```

作为整个 ModularGameVFX 的统一质量验证平台。
