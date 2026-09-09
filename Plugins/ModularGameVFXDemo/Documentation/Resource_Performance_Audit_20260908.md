# 运行卡顿与资源释放审查（2026-09-08）

## 已确认的问题与修复

1. `AVFXShowcaseController::Tick` 原来每帧遍历全局 Niagara 组件，包括编辑器其他 World 的对象，再过滤当前 World。界面实际每 0.25 秒才刷新统计。现将组件计数和完成实例清理降为每 0.25 秒一次；逐帧动画与帧时间采样保持原行为。这减少统计开销，但不是 GPU 性能改善的测量结论。
2. 预览结束/管理器停止后，Showcase 仍保留定位组件、句柄与运动记录，直到重播或退出。新增句柄到定位组件的关联，管理器句柄失效后释放定位组件并清理运动记录。柔和停止保留关联直到 Niagara 真正完成，避免提前切断尾迹。生成失败也清理定位组件。
3. 该残留有自动化复现：管理器停止后预期 0 个定位组件，修复前实际为 1；修复后通过。它属于生命周期遗漏，但重播会清空，不能据此断言它是无限增长或严重卡顿的唯一原因。

## 已验证的释放路径

- 寒冰/火球/奥术投射物具有生命上限或命中销毁路径，附属灯光与组件随 Actor 销毁。
- 花瓣与治疗效果、火焰区域有 Actor 生命期限；施法中断会停止蓄力与地面预警。
- VFX 管理器响应 `OnSystemFinished`，移除实例及索引；退出时移除 ticker、取消异步加载、释放预加载引用。
- 导出检查的 5 个 Niagara 资产，其 29 个发射器均为 Self/Once 生命周期。系统层 Infinite 不能单独判为泄漏：UE 也会在发射器全部完成时结束系统。

## 真实 PIE 连续播放证据

首轮测试：5 轮 × 50 个实例，共 250 次；使用真实渲染的 PIE，未强制 GC。每轮先确认每个受测资产确有 10 个活动实例，再等待到 10 秒检查结束状态。

| 项目 | 结果 |
| --- | --- |
| 每轮等待后的受测活动实例 | 全部为 0 |
| Actor / 点光源 | 始终 36 / 5 |
| Niagara 池容量 | 首轮到 50，后续复用 50，未逐轮增加 |
| 私有内存 | 初始 4752 MB；预热后约 4825–4828 MB；末轮 4789 MB |

50 个非活动池组件是可复用缓存，不能把“对象仍存在”视为“仍在模拟”。本结果覆盖受测资产与约一分钟的连续播放，不能排除其他效果、长时间运行或 GPU 瓶颈。

补充战斗测试已通过：实际施放全部 8 种技能，重复 3 轮共 24 次，逐轮等待 10 秒后 Actor 恢复为 36、点光源恢复为 5、活动 Niagara 恢复为 0。并验证 A/D 不转向横移、无伤害时怪物不主动攻击、受击立即进入反击状态。`runtime_soak.json` 的 `ok=true`、`combat_spell_casts=24`、`ad_strafe=true`、`retaliation_only=true` 为本次结果。

证据：`Saved/ResourceAudit/niagara_soak_verified.json`、`Saved/ResourceAudit/fixed_tests/index.json`。后续包含战斗施法与输入回归的结果写入 `Saved/ResourceAudit/runtime_soak.json`；只有 `ok=true` 才代表整套完成。

## 仍需区分的其他卡顿来源

- 原运行日志明确记录累计 100 次 PSO 创建卡顿，且报告 0 个预缓存。这支持存在首次使用着色管线的卡顿，不能解释所有持续掉帧。
- 原预览曾以 3840×2037 渲染。尚未取得固定场景的 CPU/GPU 分项耗时，不能直接把低 FPS 归因于特效内存。
- 本机多个大型应用并行运行，审查时内存提交一度接近上限，UBA 因内存压力反复终止编译任务。已改用单并发构建，没有关闭其他项目或修改系统内存设置。

## 构建边界

资源修复完整项目构建成功，Showcase 7 项自动化测试通过。随后共享引擎的 NetCore 被判定需要重链接，但 DLL 被另一项目占用；未关闭该项目。最终变更采用限定项目模块的构建验证，日志为 `Saved/BuildAndRun/final_modules_build.log`。这不等于共享引擎的全部待构建内容已完成。

同时按之前要求更新了 A/D 横移、受击后反击和 F1 线框快捷键冲突。UMG 字号按视口 DPI 补偿为与战斗 Canvas 相同的 9 像素基准（按用户最后指定的 9 号字更新）；修改不保存或覆盖用户关卡。

界面复测完成播放 → 10 实例压力 → 停止返回战斗，停止后的受管实例、活动实例和控制器句柄均为 0；179 个文字控件的字号审计没有不一致。截图在 `Saved/ShowcaseSetup/Showcase_CombatHUDPlay.png` 等文件。字号更新中发现并修正了 UE 5.8 `SetWidgetStyle` 将临时参数地址传给 Slate 的问题，改用控件自身持有的样式存储；随后界面完整流程通过。验证脚本先结束 PIE 再退出编辑器，最终退出码 0，见 `Saved/BuildAndRun/final_combat_smoke.log`。

最终代码的 Showcase 7 项自动化测试再次全部通过，见 `Saved/ResourceAudit/final_tests/index.json`。引擎实际加载的输入配置确认 F1 的 `viewmode wireframe` 绑定已经移除，见 `Saved/ResourceAudit/input_config.json`。交互复测使用 1600×900 独立游戏窗口，启动记录在 `Saved/ResourceAudit/final_launch.json`。

用户随后将统一字号指定为 9 号（替代 10 号要求）。战斗 HUD 与 VFX 菜单均已更新；`font9_build.log` 模块编译成功，`font9_smoke.log` 完整界面流程退出码 0，179 个文字控件审计的 `expectedFontSize=9`、`fontMismatches=[]`。
