# Mythic VFX 1.0.0 安装、升级与复用

MythicVFX 是只含资产的 Niagara 内容插件。ModularGameVFXDemo 是可选的演示插件，提供角色、施法动画、怪物 AI、操作和 `L_ModularGameVFX_Demo`。仅使用特效时只安装 MythicVFX。

## 目录与依赖

```text
Plugins/
  MythicVFX/
    MythicVFX.uplugin
    Content/Effects/
      Frost/{Niagara,Materials,Textures,Meshes}/
      Fire/{Niagara,Materials,Textures,Meshes}/
      Arcane/{Niagara,Materials,Textures,Meshes}/
      Nature/{Niagara,Materials,Textures,Meshes}/
    Content/{Niagara,Materials,Textures,Meshes,Blueprints}/  既有通用库
    SourceArt/                 可编辑纹理与网格源文件，不参与 Cook
    Documentation/            目录表、版本说明、路径迁移表
  MythicVFXDemo/
    MythicVFXDemo.uplugin
    Content/Maps/L_ModularGameVFX_Demo.umap
    Content/{Characters,Animations,Environment,Blueprints,Cinematics}/
    Source/MythicVFXDemo/      可独立编译的演示运行时模块
    Tests/                    编辑器自动验证脚本
```

Niagara 中 Systems 存最终系统、Modules 存自定义模块、Scalability 存预算策略。Materials 中 Parents 存父材质、Instances 存材质实例。原通用库路径保持兼容。

依赖方向：宿主 → 演示插件 → 资源插件 → 引擎 Niagara。资源插件不引用宿主或演示角色。治疗、伤害、追踪弹道和 5 秒冻结属于演示玩法，不是单独生成 Niagara 就会自动执行的逻辑。

## 安装

1. 关闭目标工程编辑器。把 `MythicVFX` 整个文件夹复制到目标工程的 `Plugins` 目录。也可以使用发布包中的安装工具，默认仅安装资源插件。
2. 打开工程并启用 Mythic VFX 和 Niagara。在内容浏览器开启“显示插件内容”。资产挂载点始终是 `/MythicVFX/`。
3. 演示时另安装并启用 ModularGameVFXDemo。打开 `/ModularGameVFXDemo/Maps/L_ModularGameVFX_Demo`。地图设置自带演示 GameMode。
4. 演示 DLL 对应本机 UE 5.8 源码构建。其他引擎构建需从附带源码重新编译演示插件。纯内容插件不需要项目 C++。本次未验证旧版引擎和移动端。

## 复用

在蓝图中使用 Spawn System at Location 播放命中、冰环或地面效果。使用 Spawn System Attached 播放手部蓄力。弹体系统沿本地 +X 前进，挂在项目自己的弹体 Actor 上，由宿主驱动位置并执行命中判定。使用组件池 Auto Release；持续组件在结束时 Deactivate，避免反复创建临时 Actor。

花瓣系统使用 `User.GlowColor`、`User.PetalColor`、`User.CoreColor`、`User.Intensity`、`User.SpellDuration`。先关闭自动激活，设置参数，再 Activate。粉色花瓣和绿色治疗共用同一个系统。需要跟随玩家时，每帧将组件放在角色脚下的地面高度并保持世界朝上；演示插件已提供实现。

奥术弹体命中时设置 `User.HeadAlpha=0`，再 Deactivate，保留约 0.5 秒让尾迹退场。此参数只控制弹头显示，伤害与弹道由宿主负责。

火焰使用 `T_Fire_FlameFlipbook_4x4`、`M_Fire_FlameFlipbook_SubUV` 和 4×4 SubUV 混帧。纹理原始分辨率 1254×1254，导入拉伸为 2048×2048。不要改变 SubImageSize 而不同时更新序列帧布局。

## 升级与迁移

- 保持插件目录名 MythicVFX 和挂载点不变。跨工程复用优先复制整个插件，而不是把内容迁入 `/Game`。
- 版本号使用主版本.次版本.修订号。VFX ID 发布后不复用；路径变更必须发布 `AssetPathRedirects.csv` 与迁移说明。
- 升级前关闭编辑器，备份旧插件与工程配置，再完整替换插件文件夹。不要只覆盖部分 uasset。发布安装工具会核对文件校验值并保存旧插件备份。
- 项目定制材质实例和特效变体放到宿主自己的目录中。直接改插件内资产会与后续升级冲突。
- 从本次旧主工程升级时，源码里的旧 `/Game/FrostMage`、`/Game/FireMage`、`/Game/ArcaneMage`、`/Game/PetalBloom` 路径按迁移表替换。字符串路径无法依靠内容浏览器自动修复。
- 演示原生类已转移到 `/Script/MythicVFXDemo`。附带 CoreRedirects 处理原项目旧类名；新工程直接使用新类。

## 打包

插件文件夹是安装单元，版本 ZIP 是分发单元，Cook 产物是对应平台运行数据，三者用途不同。项目打包时由引用关系收集资源；动态字符串加载的额外 VFX 应加入宿主 Asset Manager 或明确的 Cook 目录。不要为只用一个法术而把全部通用素材库强制加入 Cook。

演示地图的原生角色持有本轮法术硬引用，演示 Cook 会收集当前法术及角色依赖。SourceArt、Documentation、Tests 不放在运行时 Content 下。发布包的 Validation 报告记录本次实际验证范围，性能数据只针对记录的硬件和画面。
