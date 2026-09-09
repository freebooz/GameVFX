# Mythic VFX 1.0 模块交付实施计划

> **For agentic workers:** Use inline execution for this authorized migration. Preserve existing assets and do not dispatch agents.

**Goal:** 把已完成特效从主工程迁入独立内容插件，演示玩法另装可选插件，交付可安装、升级、迁移、复用及打包的版本。

**Architecture:** 沿用 MythicVFX 内容插件，新增 Effects/Frost、Fire、Arcane、Nature 目录。MythicVFXDemo 插件拥有演示 C++、角色依赖、动画、布景和名为 `L_ModularGameVFX_Demo` 的地图，只依赖 MythicVFX。主工程成为宿主，不再拥有任何演示类或法术资产。已存在的通用资源库保留稳定路径和单独清单。

**Tech Stack:** Unreal Engine 5.8 source build, Niagara, Unreal Python through Monolith MCP, C++, PowerShell/Python release tooling.

**Spec:** 用户本任务消息：独立安装、升级、迁移、复用、打包；规范目录；地图使用英文名 `L_ModularGameVFX_Demo`；目录表含 VFX Catalog、VFX ID、效果描述；文件名精准。

## Global Constraints

- 运行时内容不得依赖 /Game 或 /Script/MythicVFXLab。
- 现有旧插件路径保留兼容；本次迁移路径记录到 AssetPathRedirects.csv。
- 新名字说明类别、用途和阶段，不把迭代号写入正式资产名。
- 所有 UAsset 移动通过 Unreal AssetTools，禁止直接文件搬移破坏引用。
- 原工程最新资产和源码备份在 Saved/ModuleRelease/BackupBeforeExtraction。
- 不修改迁移备份 MythicVFXLab_MigrationBackup_20260907。

## Task 1: 清点与备份

- [x] 用 AssetRegistry 导出全量依赖 inventory_before.json。
- [ ] 生成无重名路径映射；备份源码、配置、涉及迁移的资产。
- [ ] 输出本次 13 个 Niagara 系统与既有通用库的独立目录。

## Task 2: 资产模块与演示模块

- [ ] 新建 Plugins/MythicVFXDemo/MythicVFXDemo.uplugin 和 Source/MythicVFXDemo。
- [ ] 迁移 Gameplay 文件并修改 API 宏及资产路径；用 CoreRedirects 兼容旧类包。
- [ ] 核心特效移动到 /MythicVFX/Effects；角色和布景移动到 /MythicVFXDemo。
- [ ] 地图移动并重命名为 `/ModularGameVFXDemo/Maps/L_ModularGameVFX_Demo`。
- [ ] 主模块简化为宿主入口；默认地图更新。
- [ ] 编译插件和宿主，重新打开地图并保存引用。

## Task 3: 独立性与交付

- [ ] 对发布内容遍历硬软依赖，断言无 /Game、/Script/MythicVFXLab。
- [ ] 生成稳定 VFX ID、目录 CSV/Markdown/JSON、安装升级迁移文档、文件 SHA256 清单。
- [ ] 在新的验证宿主安装插件，运行演示；执行 Cook/Package 验证并保存日志。
- [ ] 回归当前法术、冰冻时间、Niagara 预算与生命周期，区分旧验证与迁移后验证。
- [ ] 生成版本 ZIP 和安装/升级工具；还原主工程 `L_ModularGameVFX_Demo` 供人工查看。
