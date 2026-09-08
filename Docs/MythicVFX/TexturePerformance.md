# 纹理性能与移动端兼容规范

## 通用约束

- AI 原始图只保存在 `SourceArt`；导入前生成 2 的幂、8-bit 运行源，禁止直接将任意尺寸原稿作为运行时纹理。
- 灰度 Mask、Noise、Flow、Distortion 数据纹理关闭 sRGB，并移除无用 Alpha。
- 单通道数据使用 `TC_Grayscale`，统一归入 `TEXTUREGROUP_Effects`。
- 非平铺 Sprite/Glint/Streak/Mask 使用 Clamp；仅通过四边像素验证的 Noise/Distortion 使用 Wrap。
- 每张纹理必须回读 Resolution、Compression、sRGB、Alpha、TextureGroup、AddressX/Y 与 MaxTextureSize。

## 尺寸预算

| 质量档 | Hero 主形状 | 微粒/噪声 | 说明 |
| --- | ---: | ---: | --- |
| Cinematic | 1024 | 1024 | 仅高端平台或离线展示 |
| High | 1024 | 512 | 高端移动设备需按显存实测 |
| Medium | 512 | 256 | 移动端默认目标 |
| Low | 256 | 128 | 保留主形状与 Gameplay Readability |

## CH01 Magic Spark 实际设置

- 10 张运行源全部为 8-bit 单通道、无 Alpha。
- 9 张为 1024×1024；Streak 为 1024×512。
- Unreal 回读统一为 `sRGB=False`、`TC_Grayscale`、`TEXTUREGROUP_Effects`、`CompressionNoAlpha=True`、`MaxTextureSize=1024`。
- Noise Soft、Noise Perlin、Distortion 的左右/上下边界 MAE 均为 0，可安全 Wrap。
- 主材质目标为一个纹理采样；不使用 Refraction、SceneDepth、PixelDepthOffset、动态分支或每粒子 Light Renderer。
- 移动端通过 Device Profile 下调 LOD 上限；Niagara 通过 SpawnRate、Burst 和次级层开关分级，核心 Spark 轮廓始终保留。
