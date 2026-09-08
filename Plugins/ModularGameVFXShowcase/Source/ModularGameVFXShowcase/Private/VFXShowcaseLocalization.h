#pragma once
#include "CoreMinimal.h"

/** Presentation-only vocabulary. Gameplay tags, action payloads and stored enum keys stay unchanged. */
namespace VFXShowcaseUI
{
    constexpr int32 FontSize = 12;

    inline FString Chinese(const FString& Key)
    {
        static const TMap<FString, FString> Labels = {
            {TEXT("All"), TEXT("全部")}, {TEXT("Favorites"), TEXT("收藏")}, {TEXT("Recent"), TEXT("最近浏览")},
            {TEXT("Failed"), TEXT("未通过")}, {TEXT("Unreviewed"), TEXT("未验收")}, {TEXT("Unspecified"), TEXT("未分类")},
            {TEXT("Cast"), TEXT("施法")}, {TEXT("Projectile"), TEXT("投射物")}, {TEXT("Trail"), TEXT("拖尾")},
            {TEXT("Beam"), TEXT("光束")}, {TEXT("Impact"), TEXT("命中")}, {TEXT("Area"), TEXT("范围")},
            {TEXT("Status"), TEXT("状态")}, {TEXT("Space"), TEXT("空间")}, {TEXT("Lifecycle"), TEXT("生命周期")},
            {TEXT("Character"), TEXT("角色")}, {TEXT("World"), TEXT("世界")}, {TEXT("Surface"), TEXT("表面")},
            {TEXT("Destruction"), TEXT("破坏")}, {TEXT("Indicator"), TEXT("指示器")}, {TEXT("UI"), TEXT("界面")},
            {TEXT("None"), TEXT("无")}, {TEXT("Energy"), TEXT("能量")}, {TEXT("Fire"), TEXT("火焰")},
            {TEXT("Water"), TEXT("水流")}, {TEXT("Ice"), TEXT("寒冰")}, {TEXT("Lightning"), TEXT("雷电")},
            {TEXT("Wind"), TEXT("风")}, {TEXT("Earth"), TEXT("大地")}, {TEXT("Nature"), TEXT("自然")},
            {TEXT("Poison"), TEXT("毒素")}, {TEXT("Light"), TEXT("光明")}, {TEXT("Dark"), TEXT("暗色")},
            {TEXT("Arcane"), TEXT("奥术")}, {TEXT("Tech"), TEXT("科技")}, {TEXT("Physical"), TEXT("物理")},
            {TEXT("Flash"), TEXT("闪光")}, {TEXT("Explosion"), TEXT("爆炸")}, {TEXT("Shockwave"), TEXT("冲击波")},
            {TEXT("Ring"), TEXT("圆环")}, {TEXT("Orb"), TEXT("球体")}, {TEXT("Slash"), TEXT("斩击")},
            {TEXT("Column"), TEXT("柱体")}, {TEXT("Cloud"), TEXT("云团")}, {TEXT("Burst"), TEXT("迸发")},
            {TEXT("Wave"), TEXT("波浪")}, {TEXT("Field"), TEXT("力场")}, {TEXT("Circle"), TEXT("圆形")},
            {TEXT("Portal"), TEXT("传送门")}, {TEXT("Spark"), TEXT("火花")}, {TEXT("Smoke"), TEXT("烟雾")},
            {TEXT("Debris"), TEXT("碎片")}, {TEXT("Combat"), TEXT("战斗")}, {TEXT("Weapon"), TEXT("武器")},
            {TEXT("Cinematic"), TEXT("影视级")}, {TEXT("Interaction"), TEXT("交互")}, {TEXT("OneShot"), TEXT("单次")},
            {TEXT("Loop"), TEXT("循环")}, {TEXT("Persistent"), TEXT("持续")}, {TEXT("Attached"), TEXT("附着")},
            {TEXT("AtLocation"), TEXT("指定位置")}, {TEXT("Screen"), TEXT("屏幕")}, {TEXT("Environment"), TEXT("环境")},
            {TEXT("Critical"), TEXT("关键")}, {TEXT("Cosmetic"), TEXT("装饰")}, {TEXT("Ambient"), TEXT("氛围")},
            {TEXT("AutoRelease"), TEXT("自动回收")}, {TEXT("ManualRelease"), TEXT("手动回收")},
            {TEXT("ManualRelease_OnComplete"), TEXT("结束后手动回收")}, {TEXT("FreeInPool"), TEXT("对象池空闲")},
            {TEXT("CPU"), TEXT("处理器")}, {TEXT("GPU"), TEXT("图形处理器")}, {TEXT("CPU + GPU"), TEXT("处理器与图形处理器")},
            {TEXT("Unavailable"), TEXT("无法获取")}, {TEXT("Missing"), TEXT("缺失")},
            {TEXT("Neutral"), TEXT("中性灰")}, {TEXT("Bright"), TEXT("明亮")}, {TEXT("Complex"), TEXT("复杂场景")},
            {TEXT("High"), TEXT("高")}, {TEXT("Medium"), TEXT("中")}, {TEXT("Low"), TEXT("低")}, {TEXT("VRMobile"), TEXT("虚拟现实与移动端")},
            {TEXT("1.0x"), TEXT("正常速度")}, {TEXT("0.5x"), TEXT("半速")}, {TEXT("0.25x"), TEXT("四分之一速度")}, {TEXT("0.1x"), TEXT("十分之一速度")},
            {TEXT("Horizontal"), TEXT("横挥")}, {TEXT("Vertical"), TEXT("竖劈")}, {TEXT("SwingHorizontal"), TEXT("横挥")},
            {TEXT("SwingVertical"), TEXT("竖劈")}, {TEXT("Spin"), TEXT("旋转")}, {TEXT("Dash"), TEXT("冲刺")}, {TEXT("Jump"), TEXT("跳跃")}, {TEXT("Land"), TEXT("落地")},
            {TEXT("Stone"), TEXT("石材")}, {TEXT("Metal"), TEXT("金属")}, {TEXT("Wood"), TEXT("木材")}, {TEXT("Ground"), TEXT("地面")}, {TEXT("Wall"), TEXT("墙面")},
            {TEXT("NotTested"), TEXT("未测试")}, {TEXT("Pass"), TEXT("通过")}, {TEXT("Fail"), TEXT("不通过")},
            {TEXT("NeedsOptimization"), TEXT("需要优化")}, {TEXT("NeedsVisualRework"), TEXT("需要视觉重做")}, {TEXT("ProductionReady"), TEXT("可用于生产")},
            {TEXT("NotAssessed"), TEXT("未评估")}, {TEXT("Approved"), TEXT("已通过")}, {TEXT("KeepCurrent"), TEXT("保持当前结论")}, {TEXT("ResetAllToNotTested"), TEXT("全部重置为未测试")},
            {TEXT("Visual"), TEXT("视觉")}, {TEXT("Gameplay"), TEXT("玩法可读性")}, {TEXT("Performance"), TEXT("性能")},
            {TEXT("Naming"), TEXT("命名")}, {TEXT("Catalog"), TEXT("资源目录")}, {TEXT("Dependency"), TEXT("依赖关系")},
            {TEXT("WeakVisual"), TEXT("视觉平淡")}, {TEXT("WeakImpact"), TEXT("打击感不足")}, {TEXT("PoorLayering"), TEXT("层次不足")},
            {TEXT("ColorIssue"), TEXT("颜色问题")}, {TEXT("ExcessiveBloom"), TEXT("泛光过强")}, {TEXT("Occlusion"), TEXT("遮挡严重")},
            {TEXT("LifetimeIssue"), TEXT("生命周期问题")}, {TEXT("Overdraw"), TEXT("过度绘制")}, {TEXT("TooManyParticles"), TEXT("粒子过多")},
            {TEXT("HighCost"), TEXT("性能开销过高")}, {TEXT("VRIssue"), TEXT("虚拟现实表现问题")}, {TEXT("WrongCategory"), TEXT("分类错误")},
            {TEXT("CatalogIssue"), TEXT("资源目录错误")}, {TEXT("Other"), TEXT("其他")},
            {TEXT("Category / Element / Name"), TEXT("分类／元素／名称")}, {TEXT("Name"), TEXT("名称")},
            {TEXT("Priority"), TEXT("优先级")}, {TEXT("Element"), TEXT("元素")}, {TEXT("Category"), TEXT("分类")}, {TEXT("ReviewStatus"), TEXT("验收状态")},
            {TEXT("Intensity"), TEXT("强度")}, {TEXT("Scale"), TEXT("缩放")}, {TEXT("LifetimeScale"), TEXT("持续时间倍率")},
            {TEXT("Radius"), TEXT("半径")}, {TEXT("Width"), TEXT("宽度")}, {TEXT("Length"), TEXT("长度")}, {TEXT("Progress"), TEXT("进度")},
            {TEXT("PrimaryColor"), TEXT("主颜色")}, {TEXT("SecondaryColor"), TEXT("副颜色")},
            {TEXT("R"), TEXT("红色通道")}, {TEXT("G"), TEXT("绿色通道")}, {TEXT("B"), TEXT("蓝色通道")}, {TEXT("A"), TEXT("透明度")},
            {TEXT("Previous"), TEXT("上一个")}, {TEXT("Play"), TEXT("播放")}, {TEXT("Replay"), TEXT("重播")},
            {TEXT("Stop"), TEXT("停止")}, {TEXT("Stop Immediate"), TEXT("立即停止")}, {TEXT("Next"), TEXT("下一个")},
            {TEXT("Loop: OFF"), TEXT("循环：关闭")}, {TEXT("Loop: ON"), TEXT("循环：开启")},
            {TEXT("Auto Preview: OFF"), TEXT("自动预览：关闭")}, {TEXT("Auto Preview: ON"), TEXT("自动预览：开启")},
            {TEXT("Set A"), TEXT("设为对比甲")}, {TEXT("Set B"), TEXT("设为对比乙")}, {TEXT("Compare A / B"), TEXT("并排对比")},
            {TEXT("Combat simulation"), TEXT("战斗模拟")}, {TEXT("Refresh / Validate"), TEXT("刷新与校验")},
            {TEXT("Save review"), TEXT("保存验收")}, {TEXT("Save + Next"), TEXT("保存并看下一个")},
            {TEXT("Final PASS"), TEXT("最终通过")}, {TEXT("FAIL"), TEXT("不通过")}, {TEXT("Toggle Favorite"), TEXT("收藏／取消收藏")},
            {TEXT("VFX SHOWCASE  /  Catalog & acceptance workbench"), TEXT("特效展示与验收工作台")},
            {TEXT("Choose a catalog entry to play through VFX Manager."), TEXT("选择目录中的特效，通过特效管理器播放。")},
            {TEXT("Review status"), TEXT("验收状态")}, {TEXT("Sort"), TEXT("排序方式")}, {TEXT("0 entries"), TEXT("共零项特效")},
            {TEXT("LIVE PERFORMANCE"), TEXT("实时性能")}, {TEXT("Waiting for controller"), TEXT("正在等待测试控制器")},
            {TEXT("No VFX selected"), TEXT("尚未选择特效")}, {TEXT("Compare A: —  |  B: —"), TEXT("对比甲：未选择　对比乙：未选择")},
            {TEXT("Select an entry for Single mode. Stress uses its own test zone.\nUse the Camera distance controls to inspect near and far readability."), TEXT("选择特效进入单项预览，压力测试使用独立区域。\n使用相机距离控件观察近、中、远距离可读性。")},
            {TEXT("ENVIRONMENT"), TEXT("测试环境")}, {TEXT("Background"), TEXT("场景背景")}, {TEXT("Quality"), TEXT("质量等级")},
            {TEXT("Playback speed"), TEXT("播放速度")}, {TEXT("Preview / beam distance"), TEXT("预览与光束距离")}, {TEXT("Camera distance"), TEXT("相机距离")},
            {TEXT("Near 3m"), TEXT("近处：三米")}, {TEXT("Medium 10m"), TEXT("中距：十米")}, {TEXT("Far 40m"), TEXT("远处：四十米")},
            {TEXT("VR preview / VRMobile quality"), TEXT("虚拟现实预览／移动端质量")},
            {TEXT("MANUAL REVIEW"), TEXT("人工验收")}, {TEXT("No review selected"), TEXT("尚未选择验收对象")},
            {TEXT("Check: focus, impact, force, speed, depth, color, silhouette, timing, readability, end feedback and performance."), TEXT("检查视觉中心、冲击力、力量感、速度、层次、颜色、轮廓、节奏、可读性、结束反馈与性能。")},
            {TEXT("Final verdict (KeepCurrent preserves recorded gates)"), TEXT("最终结论（保持当前结论可保留各项结果）")}, {TEXT("Failure reason"), TEXT("未通过原因")},
            {TEXT("Production Ready requires all six gates approved with reviewer + evidence for this asset version. Saves review data only."), TEXT("当前资源版本的六项验收全部通过，并具备评审人和证据后，方可用于生产。此处仅保存验收记录。")},
            {TEXT("INSPECTOR"), TEXT("资源详情")}, {TEXT("Select an entry from the Catalog."), TEXT("请从资源目录选择一项特效。")},
            {TEXT("PARAMETERS"), TEXT("参数调试")}, {TEXT("This resource exposes no supported user parameters."), TEXT("此资源没有可调整的用户参数。")},
            {TEXT("Reset to Catalog defaults"), TEXT("恢复资源目录默认参数")}
        };
        if (const FString* Result = Labels.Find(Key)) return *Result;
        return Key;
    }
}
