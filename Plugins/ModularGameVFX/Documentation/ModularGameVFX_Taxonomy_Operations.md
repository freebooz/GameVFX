# ModularGameVFX 分类与使用规范

唯一业务内容根为 `/ModularGameVFX/ModularGameVFXLibrary`。共用资源进入 `00_Core`，完整效果按主要功能进入 `01_Cast` 至 `15_UI`。没有效果的分类保留为空，不能用未完成资源填充数量。

每个正式 Catalog 条目使用强类型 Category、Element、Form、Context、Lifecycle 和 Priority。Priority 在代码中的序列化名称仍为 `PerformanceClass`，编辑器显示名为 Priority。业务标签采用 `VFX.<Category>.<Element或SubCategory>.<Name>`。项目角色专属标签只在配置/数据中声明。

当前完整法术有 13 个独立 Niagara 组件。花瓣魔法的粉色和翡翠色共用一个系统；火球命中与火焰冲击共用爆燃组件。早期 23 个系统明确标记 Legacy，不能由目录存在推断为已完成或性能达标。

## Core 与命名

Core 的 17 个指定子目录为 Energy、Fire、Smoke、Sparks、Dust、Lightning、Glow、Shockwave、Debris、Noise、Ribbon、Meshes、Materials、MaterialFunctions、Textures、NiagaraModules、Curves；EffectTypes 与 Catalog 另存性能策略和索引数据。

使用 NS_、NE_、NM_、M_、MI_、MF_、T_、SM_、DA_、ET_、BP_ 类型前缀。既有 Niagara Dynamic Input 继续使用 DI_，它们不是 Niagara Module。已具备视觉职责的 Emitter 名称（例如 Charge_ColdCore）保持原意。不要通过文件资源管理器直接改名 .uasset。

## 性能配置

`ET_VFX_Critical`、`ET_VFX_Combat`、`ET_VFX_Cosmetic`、`ET_VFX_Ambient` 是正式库统一使用的四类性能资产，配置距离裁剪、同类及单系统实例数量、距离重要性排序和分质量规则。迁移脚本按 Priority 绑定，原 Effect Type 对应关系保存在迁移报告中；系统自身固定边界、Emitter 预算及覆盖设置保留。替换性能策略后的视觉表现和实际预算需要重新验收。

持续 Status 使用 ManualRelease，AutoDestroy=false，由 Manager 句柄 StopVFX / StopAllVFXForOwner 显式停止。高频 OneShot 使用 AutoRelease。已有演示模块的调用方式仍需单独核查；目录迁移通过不代表业务调用规范已经全部验收。

Showcase 五档质量索引为 0=VRMobile，1=Low，2=Medium，3=High，4=Cinematic；High 使用 UE 的 Epic 基础档值 3。VRMobile 是本库的预算档名称；实际 VR/移动项目仍需选择 Device Profile 并在目标设备验证。既有系统和 Emitter 的覆盖配置仍须逐项观察降级效果。模板预算不能被表述为已通过实机性能验收。

降级优先减少次级粒子、生成数量、烟雾、Ribbon 细节、扭曲、粒子灯、Mesh 粒子及残留时间，最后才减少核心轮廓、主冲击及范围提示。通用 Effect Type 不会自动实现每个发射器的视觉降级顺序。

## 跨项目使用

复制整个插件并启用 Niagara 后，默认软引用指向随库提供的 DA_VFXCatalog_Default；也可在项目设置 Plugins → Modular Game VFX 指定其他 Catalog，或在宿主 Config/DefaultGame.ini 配置：

```ini
[/Script/ModularGameVFX.ModularGameVFXSettings]
DefaultCatalog=/ModularGameVFX/ModularGameVFXLibrary/00_Core/Catalog/DA_VFXCatalog_Default.DA_VFXCatalog_Default
```

业务通过现有 UModularGameVFXBlueprintLibrary 调用、停止及设置参数。宿主的伤害、输入和角色行为属于演示模块。

## 旧路径清理与证据

用户最终要求移除旧路径：完成迁移和引用更新后删除旧资产重定向器及旧运行配置路径，不继续提供旧资产路径兼容。迁移对应表仅保留在备份记录中。

迁移证据位于宿主 Saved/TaxonomyMigration。真实迁移完成以 migration_complete.json 为证，最终资产与引用验证以 verification.json 为证；不得把未执行的目标路径写成已迁移路径。
