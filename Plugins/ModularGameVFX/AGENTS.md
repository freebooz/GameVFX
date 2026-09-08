# ModularGameVFX 工作入口

对本插件开展效果创建、批量生产或质量登记前，先阅读 [生产规范入口](Documentation/Production/README.md)，并按任务范围阅读其中的单个 VFX 与批量验收原文。遵循用户当前明确指令；规范示例不扩大授权范围。

- 按职责分类物理业务目录；元素作为 Metadata/Tag。先搜索完整 System 与 Core 复用，再创建缺失部分。
- 现有 Catalog、Manager、强类型枚举与序列化字段保持兼容。不能把原文中当前不存在的 HeavyImpact/Holy 示例当成已实现枚举；具体映射见入口。
- 使用 Production 下的需求和验收 JSON 模板记录来源与证据。没有实际测量或评审时使用 null/NotAssessed；编译成功不等于视觉或性能通过。
- ProductionReady 必须同时具备 Visual、Gameplay、Performance、Naming、Catalog、Dependency 六项真实通过证据。未 Profile 必须明确“尚待实机 Profile”。
- 12 元素全套、18 项矩阵以及示例 Pack 不构成资产创建任务；仅执行用户实际请求的范围。
- 本次迁移旧位置和兼容 redirector/旧 CoreRedirect 按用户要求清理，映射留在备份；未来 Deprecated 规范不要求保留此次旧路径，也不授权删除其他未迁移资产。
- 共享工作区内保留其他人的修改；迁移、构建和验收由任务指定责任方执行，不并发改写同一资产。
