# ModularGameVFX 统一重命名设计

日期：2026-09-08
状态：待用户书面审核

## 目标

将当前可迁移的特效插件及其可选演示模块统一命名为 `ModularGameVFX`，消除 `MythicVFX`、`MythicVFXDemo` 和 `MVFX-*` 等旧品牌标识，同时通过虚幻重定向和编号映射保护现有关卡、资产与外部使用方。

宿主研发工程继续使用 `MythicVFXLab`。它只承担编辑、验证和发布工作，不属于可安装的特效插件。

## 标准名称

| 对象 | 新名称 | 旧名称 |
|---|---|---|
| 核心内容插件 | `ModularGameVFX` | `MythicVFX` |
| 核心资源挂载根目录 | `/ModularGameVFX` | `/MythicVFX` |
| 可选演示插件 | `ModularGameVFXDemo` | `MythicVFXDemo` |
| 演示资源挂载根目录 | `/ModularGameVFXDemo` | `/MythicVFXDemo` |
| 演示运行时模块 | `ModularGameVFXDemo` | `MythicVFXDemo` |
| C++ 导出宏 | `MODULARGAMEVFXDEMO_API` | `MYTHICVFXDEMO_API` |
| VFX 编号前缀 | `MGVFX-*` | `MVFX-*` |
| 正式发布根目录 | `ModularGameVFX_Releases` | `MythicVFX_Releases` |

插件描述、创建者显示名称、安装器名称、清单名称、文档标题、目录表文件名和发布包文件名均使用完整字符串 `ModularGameVFX`，不再使用带空格的 `Mythic VFX` 品牌名。

## 目录与模块结构

正式结构如下：

```text
Plugins/
  ModularGameVFX/
    ModularGameVFX.uplugin
    Content/
    Config/
    Documentation/
    SourceArt/
  ModularGameVFXDemo/
    ModularGameVFXDemo.uplugin
    Content/
    Config/
    Documentation/
    Source/
      ModularGameVFXDemo/
```

核心插件继续只包含可迁移的 Niagara、纹理、材质、网格体、可复用模块、性能配置和源文件。演示插件继续包含角色、动画、敌人 AI、第三人称输入、施法逻辑和 `演示关卡`，并单向依赖核心插件。

核心插件不得依赖 `/Game`、`/ModularGameVFXDemo` 或宿主工程脚本模块。演示插件可以依赖核心插件和所需的引擎插件。

## 资产迁移

所有核心资产从 `/MythicVFX/...` 迁移到 `/ModularGameVFX/...`；所有演示资产从 `/MythicVFXDemo/...` 迁移到 `/ModularGameVFXDemo/...`。迁移必须通过虚幻资产工具完成，以便更新硬引用、软引用、材质引用、Niagara 引用、动画预览网格和地图引用。

迁移后修复并保存所有重定向器。代码、配置、测试和工具中的硬编码资产路径同步更新为新路径。导入元数据继续指向插件内部 `SourceArt`，不得重新指向宿主工程目录。

## 兼容性

在宿主配置和演示插件配置中添加以下兼容关系：

- `/MythicVFX/...` → `/ModularGameVFX/...`
- `/MythicVFXDemo/...` → `/ModularGameVFXDemo/...`
- `/Script/MythicVFXDemo` → `/Script/ModularGameVFXDemo`
- `/Script/MythicVFXLab` → `/Script/ModularGameVFXDemo`

已有的 10 个演示类继续使用原类名，例如 `FMFrostMage` 和 `FMFrostMonster`，只迁移脚本包名。这样可以缩小蓝图和地图的兼容风险。

旧 VFX ID 保留在 `Legacy VFX ID` 列中，例如 `MVFX-FROST-001` 映射到 `MGVFX-FROST-001`。新发布包、文档和后续版本只使用 `MGVFX-*` 作为主编号。

## 文档与交付物

目录表主列保持 `VFX Catalog`、`VFX ID`、效果名称、效果描述、演示按键、Niagara 资产路径、效果阶段、复用方式和版本，并新增 `Legacy VFX ID`。Excel、CSV、JSON 和 Markdown 四种格式使用一致数据源。

安装器统一命名为 `Install-ModularGameVFX.ps1`，支持仅安装核心插件或同时安装演示插件。安装清单、哈希校验、升级保护、修改文件检测和回滚行为保持不变，但所有目标目录和插件条目使用新名称。

正式发布路径为：

```text
F:/game/ModularGameVFX_Releases/1.0.0/
```

旧发布候选目录只作为历史构建保留，不作为正式交付路径。

## 实施顺序

1. 停止 PIE，保存工程并正常关闭虚幻编辑器。
2. 生成迁移前备份与资产路径清单。
3. 重命名插件目录、描述文件、模块目录、构建文件、配置和代码标识。
4. 编译新模块，使虚幻能够加载新脚本包。
5. 启动虚幻，通过资产工具迁移两套插件内容并修复重定向器。
6. 更新宿主工程、文档、测试、安装工具和发布脚本。
7. 重建模块，执行依赖审计、当前 13 个法术系统编译检查和演示关卡运行检查。
8. 重新生成目录表、安装包和发布清单，并在一个空白宿主工程验证安装。

## 验证标准

完成重命名必须满足：

- 虚幻编辑器能够加载 `演示关卡`，玩家、怪物、输入和 1–8 号法术仍可运行。
- 当前制作的 13 个 Niagara 法术组件编译时没有错误。
- 核心插件及演示插件不存在对 `/Game`、`/Script/MythicVFXLab`、`/Script/MythicVFXDemo`、`/MythicVFX` 或 `/MythicVFXDemo` 的未允许依赖。
- 新插件目录中不存在用于发布的旧品牌文件名。
- 旧资源路径与旧脚本包引用可通过重定向加载。
- 核心插件可以独立安装到空白内容工程；启用演示插件后可以打开 `演示关卡`。
- 发布清单中的所有文件哈希与实际文件一致。

旧通用资源库中已经存在的 Niagara 编译问题不归因于本次重命名。目录表必须如实标注其验证状态；这些问题不得被写成已通过。本次迁移不能增加新的编译错误。

## 回退策略

迁移开始前完整复制两个插件目录、宿主配置、工程描述文件和源码到带时间戳的备份目录。若新模块无法编译或资产迁移中断，从该备份恢复整个插件集合，不混合恢复单个 `.uasset` 文件。正式发布目录只在全部验证通过后生成。
