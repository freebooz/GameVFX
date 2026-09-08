# 演示关卡与操作配置

启用 MythicVFXDemo 后打开 `/MythicVFXDemo/Maps/演示关卡`，点击运行。地图自带演示 GameMode，不需要把它设成整个项目的默认 GameMode。

1–8 分别是寒冰箭、冰环、粉色花瓣、绿色治疗、火球、火焰冲击、烈焰风暴、奥术飞弹。鼠标点击选择怪物。右键拖动转向，左键拖动观察，双键前进，滚轮缩放。W 前进、S 退步、A/D 转向、Q/E 横移，右键+A/D 横移，Space 跳跃，Tab 切换目标，Esc 取消。

独立空工程中要复现本演示的鼠标比例，请将 `DemoConfiguration/DefaultInput.ini` 用作该空工程的输入配置。已有游戏请只合并所需设置：MouseX/MouseY/Mouse2D 的轴灵敏度为 1，关闭鼠标平滑和 FOV 缩放，使用 Enhanced Input 的 PlayerInput 和 InputComponent。不要覆盖已有游戏的按键配置。

演示插件使用以下可选 Game.ini 参数，未配置时使用相同的代码默认值：

```ini
[FrostMage.Mouse]
CameraYawMoveSpeed=180.0
CameraPitchMoveSpeed=90.0
MouseSensitivityMultiplier=1.0
RawDegreesPerCount=0.10
InvertMousePitch=False
SmartCameraFollow=True

[FrostMage.Visuals]
CharacterMoonFillIntensity=12000.0
```

鼠标默认 0.10 度/计数是本项目的校准值，可按鼠标 DPI 调整倍数。本项目没有验证魔兽世界闭源客户端的全部底层输入算法。

默认第三人称相机距离 690 cm。冰环瞬发并冻结怪物 5 秒。绿色治疗每帧跟随玩家脚下的地面中心。怪物会游荡、追击，并在受到攻击后反击。法术蓝图调用只产生视觉效果；完整战斗规则由演示 C++ 实现。

角色、骨骼和基础移动动画来自 Unreal Engine 模板。相关模板资源的使用遵循其随附许可。演示插件是可选依赖，纯特效项目不需要复制这些角色资源。
