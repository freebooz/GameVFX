# Mythic VFX

`MythicVFXLab` 是研发、预览和验证工程；`Plugins/MythicVFX` 是唯一允许迁移到目标游戏的正式视觉包。

## 依赖边界

- `/Game/VFXLab/` 可以依赖 `/MythicVFX/`。
- `/MythicVFX/` 禁止依赖 `/Game/VFXLab/` 或任何具体游戏内容。
- Unreal 资产只能通过运行中的 Monolith MCP 创建、修改、编译和验证，不直接生成或修改 `.uasset`。

## 当前里程碑

先完成 CH01 Magic Spark 的 Texture → Material → Material Instance → Emitter → Niagara System → Lab Preview → Screenshot → Validation 垂直切片。未通过 CH01 验收前不创建 CH02～CH24 正式资产。
