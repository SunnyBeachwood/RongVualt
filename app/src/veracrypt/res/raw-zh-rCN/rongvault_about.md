# 关于容匣

**版本 {{VERSION}}**

容匣是一个在 Android 上离线创建、打开和管理 VeraCrypt 兼容文件容器的
应用。容匣是独立项目，与 VeraCrypt 没有关联，也未获其认可。

## 项目 README

容匣支持普通卷和隐藏卷、卷类型自动识别、密码/PIM 解锁、打开全部 15 种
VeraCrypt 非系统 XTS 算法、9 种 Windows 创建算法和 6 种 KDF，以及为符合
条件的已保存凭据提供生物识别解锁。1.1.0 算法/KDF 扩展仍待编译、待互操作
验证。FAT 和 exFAT 支持读写，NTFS 仅支持只读。应用不会把整个容器解密到设备上的明文目录。

完整 README、兼容性说明、构建方法和安全限制请访问项目源码仓库：

[在 GitHub 阅读完整 README](https://github.com/SunnyBeachwood/RongVualt/blob/main/README.zh-CN.md)

[打开 RongVualt GitHub 项目](https://github.com/SunnyBeachwood/RongVualt)

## 许可证

容匣以 **GPL-3.0-or-later** 发布，发行包中包含完整 GPL 文本。

## 使用的开源组件

本发行版包含 EDS Lite 来源代码、Material Files 1.7.4、Botan、FatFs、
libyal NTFS 库、Markwon、AndroidX、Kotlin 及其运行时依赖。其版权和许可证
声明保留在源码发行包与应用内声明中。
