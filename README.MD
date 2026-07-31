# RongVault（Android VeraCrypt 容器工具）

RongVault 是一款面向 Android 的离线加密容器工具，用于创建、打开和管理
VeraCrypt 兼容的**非系统文件容器**。它将加密卷作为 Android 文档提供者暴露，
因此可以在解锁后通过内置文件管理器或系统“文件”应用访问卷内文件，而无需将
整个容器解密到设备存储中。

> 本项目仍处于开发阶段。请始终保留容器文件和卷头的独立备份，不要把测试版
> 软件作为唯一的数据保管方式。忘记密码、PIM 或密钥文件后无法恢复数据。

## 主要用途

- 在手机或平板上离线访问 VeraCrypt 文件容器中的资料。
- 创建普通卷或隐藏卷，将敏感文件保存在加密容器中。
- 在本地存储、SD 卡或 USB 存储设备之间导入、导出加密卷内文件。
- 在不把容器内容作为普通明文目录长期落盘的前提下浏览和管理文件。

## 已实现的功能

- 通过 Android Storage Access Framework（SAF）选择并保存本地、SD 卡和 USB
  上的可随机访问文件容器。
- 创建、添加、解锁、锁定和从目录中移除容器；移除目录记录不会删除原容器文件。
- 支持普通卷和隐藏卷；写入外层卷时可启用隐藏卷保护。
- 支持密码、PIM 和密钥文件；可选使用 Android Keystore 与设备生物识别保护已保存的
  外层卷凭据。
- 支持 VeraCrypt 非系统容器的 AES、Serpent、Twofish 及部分级联算法和 KDF 组合；
  使用主卷头和备用卷头进行识别。
- FAT 和 exFAT 卷支持读写、创建、重命名、删除、复制、移动及导入/导出；NTFS 仅以
  只读方式打开。
- 通过 `DocumentsProvider` 向 Android 系统文件界面提供已解锁卷，并在卷解锁或长时间
  文件操作期间运行前台服务。
- 内置文件管理器提供目录浏览、搜索、排序、显示隐藏文件、新建、重命名、删除以及
  卷内复制/移动等操作。

## 平台与构建环境

| 项目 | 当前范围 |
| --- | --- |
| 操作系统 | Android 15（API 35）及更高版本 |
| CPU 架构 | `arm64-v8a`（64 位 ARM） |
| 容器来源 | 本地、SD 卡或 USB 存储中的可随机访问文件 |
| 支持的文件系统 | FAT / exFAT 读写；NTFS 只读 |
| 构建工具 | JDK 17、Android SDK Platform 37、NDK 28.2.13676358 |

配置好 `local.properties` 中的 Android SDK 路径后，可构建调试版：

```powershell
.\gradlew.bat :app:assembleLiteDebug
```

克隆仓库时请使用 `git clone --recurse-submodules`；若已克隆，请先运行
`git submodule update --init --recursive` 以获取 NTFS 只读支持所需的固定版本源码。

发布构建还需要在源代码之外提供全部 `EDS_RELEASE_*` 签名配置，以及 HTTPS 形式的
`RONGVAULT_SOURCE_URL`。项目不会使用仓库中的签名密钥生成发布包。

## 限制与不支持范围

- 仅面向 VeraCrypt 1.26.29 兼容的**非系统文件容器**；不支持系统加密、物理分区或
  整块磁盘。
- 不支持 TrueCrypt、LUKS、EncFS、PKCS#11、EMV，以及云盘、网络位置或其他不可随机
  访问的容器来源。
- NTFS 不可写入；请勿尝试将其作为可写卷使用。
- 部分密码算法、KDF、级联算法、隐藏卷场景、SAF 提供者和桌面 VeraCrypt 的组合仍在
  持续验证中。兼容性结论和测试范围见
  [`docs/COMPATIBILITY_MATRIX.md`](docs/COMPATIBILITY_MATRIX.md)。
- 这不是 VeraCrypt 官方客户端，也不隶属于或获得 VeraCrypt 认可；项目没有经过独立的
  密码学或安全审计。

## 与 EDS Lite 和 VeraCrypt 的关系

### EDS Lite

本项目以 **EDS Lite 2.0.0.237** 源码为迁移基线，保留其部分历史代码与工程经验作为
参考。新产品界面、AndroidX/Gradle 构建、SAF 存储边界、Kotlin 业务层和 `vc_core` 原生
加密卷实现均在这一迁移目标下逐步重建。

RongVault 不是 EDS Lite 的官方版本，也不等同于 EDS Lite 的全部功能。它刻意收窄范围，
专注于 Android 上的 VeraCrypt 非系统文件容器，而不提供 EDS Lite 中的其他容器格式或
旧式路径访问模式。

EDS Lite [上游 README](https://github.com/sovworks/edslite#license) 声明其采用
**GPL-2.0-or-later（GPLv2+）**，其 GPLv2 正文见
[上游 LICENSE](https://github.com/sovworks/edslite/blob/master/LICENSE)。对从 EDS Lite
导入或修改的文件，本项目保留原有的版权与许可证声明，并以 GPL-3.0-or-later 发布包含
这些代码的整体作品。若某个导入文件另有 GPLv2-only 或其他许可证声明，应以该文件的
声明为准；此类文件不得在未确认许可证兼容性前与 GPLv3-only 代码合并发布。

### VeraCrypt

RongVault 参考 VeraCrypt 的公开容器格式和兼容性行为，目标是与 VeraCrypt 1.26.29 的
非系统文件容器互操作。它是独立实现，不包含 VeraCrypt 桌面程序，也不是 VeraCrypt 的
官方移动端、附属项目或认证客户端。

## 内置文件管理器与 Material Files（质感文件）

内置文件管理器借鉴并集成了开源项目
[Material Files（质感文件）](https://github.com/zhanghai/MaterialFiles) 1.7.4 的代码、
文件操作流程和 Material Design 体验。该部分经过适配后仅服务于已解锁的加密卷：

- 通过 RongVault 的 `UnlockedDocumentsProvider` 访问卷内文件；
- 不启用 Material Files 的 Root/Shizuku、网络存储、FTP 服务、APK 安装器或其凭据存储；
- 文件浏览和操作仍受加密卷会话及前台操作保护约束，不使用主机明文临时路径。

Material Files 以 [GPL-3.0-or-later](https://github.com/zhanghai/MaterialFiles/blob/master/LICENSE)
发布。本项目保留导入文件的版权和许可证声明，并在源代码中保留其完整许可证文本与上游来源说明，详见
[`materialfiles/MATERIAL_FILES_UPSTREAM.md`](materialfiles/MATERIAL_FILES_UPSTREAM.md)。

GPL 是对复制、修改和发布作出的预先授权，因此在遵守其条款时，使用这些代码通常不需要
另行获得上游作者同意；但 GPL 不授予项目名称、商标或“官方认可”的使用权。

## 开源许可证

RongVault 采用 **GNU General Public License v3.0 或更高版本（GPL-3.0-or-later）**
发布。你可以在遵守 GPL 条款的前提下使用、复制、修改和再发布本项目；分发修改版本时，
也必须提供与所分发版本对应的完整相应源代码，并保留全部版权、许可证与无担保声明。

发布 APK、其他二进制文件或源代码副本时，分发方应同时提供或以 GPL 允许的方式提供完整
相应源代码，包括构建脚本、已修改的上游代码和构建该版本所需的第三方源码；不得施加与
GPL 冲突的额外限制。第三方组件可能具有各自的许可证，分发前应逐项复核其许可条件。

本项目 GPLv3 的完整条款见 [LICENSE](LICENSE)（同样提供于 [COPYING](COPYING)）。
第三方组件的版权和许可证以其各自文件中的声明为准；发布包含 EDS Lite 来源代码的版本时，
也应随源代码一并保留 EDS Lite 的 GPLv2 许可证文本。
