# RongVault（Android VeraCrypt 容器工具）

[English](README.MD)

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
- 支持 VeraCrypt 1.26.29 的 15 种非系统 XTS 算法进行打开和自动识别；普通卷和隐藏卷
  创建页面开放 Windows 版的 9 种创建算法。1.2.0 新增内容仍处于待编译、待验证状态。
- FAT 和 exFAT 卷支持读写、创建、重命名、删除、复制、移动及导入/导出；NTFS 仅以
  只读方式打开。
- 通过 `DocumentsProvider` 向 Android 系统文件界面提供已解锁卷，并在卷解锁或长时间
  文件操作期间运行前台服务。
- 内置文件管理器恢复 Root 策略（`NEVER`、`AUTOMATIC`、`ALWAYS`），在授予 Root 后可
  浏览和编辑已挂载的设备路径。只读挂载、AVB 和内核策略仍然有效，不提供块设备写入或
  自动重新挂载。
- 提供本机 Apache FTP 服务器，可共享普通存储、Root 路径或当前已解锁卷中的一个目录。
  服务监听所有网络接口，默认端口 2121、账号登录且只读；匿名登录和写入必须由用户主动
  开启。FTP 内容和凭据不会加密。
- 内置文件管理器提供目录浏览、搜索、排序、显示隐藏文件、新建、重命名、删除以及
  卷内复制/移动等操作。
- 容匣 1.3.0 将 ZipXtract 原生整合到同一文件管理器：可浏览和提取 ZIP/JAR、7z、
  RAR/RAR5、TAR 及压缩流，创建 ZIP/7z/TAR，并仅在安全可写位置更新未加密 7z。
  提取采用流式处理，拒绝目录穿越和归档链接，归档文件系统仍保持只读。上游快照固定
  为 ZipXtract v7.1.1；编译、夹具和设备兼容性验证仍待进行。
- 支持 `.md`、`.markdown`、`.mkd` 和标准 Markdown MIME 类型文件的格式化预览；该功能
  默认开启，可在设置中关闭，仍可切换回源文件编辑。

## 加密算法与 VeraCrypt 兼容范围

RongVault 采用 VeraCrypt 非系统容器的基本设计：数据以 XTS 模式加密，卷头由密码、PIM 和
密钥文件保护，并使用选定的密码派生函数（KDF）。RongVault 1.2.0 增加完整的读取算法
注册表、9 种 Windows 创建算法、5 种 PBKDF2 和 Argon2id；这些改动已完成源代码，但仍
保持“待编译/待验证”状态：

现有数据的 `CipherHint` 名称与数值、native request v2 帧格式以及已保存凭据字段顺序均
保持不变；本轮只扩展界面显示名称和可用能力列表。

| 项目 | RongVault 1.2.0 代码范围（待编译/待验证） | 与桌面 VeraCrypt 的差距 |
| --- | --- | --- |
| 数据加密算法 | 15 种 VeraCrypt 非系统 XTS 算法均注册到打开/自动识别路径。普通卷和隐藏卷创建页面开放 AES、Serpent、Twofish、Camellia、AES-Twofish、AES-Twofish-Serpent、Serpent-AES、Serpent-Twofish-AES、Twofish-Serpent 共 9 种算法。 | VeraCrypt 还支持系统加密及平台特有布局；RongVault 的每种算法在矩阵更新前均未验证。 |
| KDF 选项 | 打开、创建、隐藏卷保护和修改凭据代码路径均携带 6 种 KDF：PBKDF2-HMAC-SHA-512（默认）、SHA-256、BLAKE2s-256、Whirlpool、Streebog、Argon2id。隐藏卷保护为保持 native v2 布局使用 KDF 自动识别；选择 Argon2id 时显示由 PIM 推导的内存量和迭代次数。 | VeraCrypt 的桌面端配置覆盖和测试更完整。RongVault 的每种 KDF/算法组合在兼容性矩阵标为已验证前，均应视为依赖具体兼容性测试。 |
| 卷类型 | 非系统文件容器、普通卷和隐藏卷。 | VeraCrypt 还支持系统加密，并可使用分区和整块磁盘。 |
| 文件系统 | FAT、exFAT 可读写；NTFS 只读。 | VeraCrypt 通过桌面操作系统驱动挂载卷，与主机文件系统的集成模式不同。 |
| 平台 | Android 15+、`arm64-v8a`，通过 SAF 和 `DocumentsProvider` 工作。 | VeraCrypt 提供 Windows、macOS、Linux 的官方桌面发行版。 |

可选择算法和 KDF 并不等于所有组合都已完成互操作验证，尤其是 Argon2id 和少见组合。
1.2.0 源代码改动均明确标为**待编译/待验证**。请查阅
[`docs/COMPATIBILITY_MATRIX.md`](docs/COMPATIBILITY_MATRIX.md) 了解已记录的测试状态。若需要
系统加密、分区支持或桌面级验证，应使用桌面 VeraCrypt。

## Root 与 FTP 范围

Root 策略可在内置文件管理器的设置中选择。默认的 `AUTOMATIC` 只在 Android 拒绝普通
访问时分发到 libsu 5.2.2 Root 服务，`ALWAYS` 会为每次挂载文件系统操作请求 Root。Root
被拒绝、超时或撤销时会明确返回错误，不会静默降级；Shizuku/Sui 仍保持禁用。授予权限
后，`/` 根目录入口可浏览 `/data`、`/system`、`/vendor`、`/product` 等已挂载路径，但
不提供块设备写入或自动重新挂载。

FTP 页面在启动时捕获不可变的共享根目录快照。普通和 Root 路径沿用现有 home-directory
设置持久化；已解锁卷中的目录仅在进程内有效，卷锁定或进程重建后自动失效。服务会在关闭
卷前停止并断开 FTP 客户端，传输期间触碰卷会话以刷新自动锁计时，并拒绝归档、FTP/SFTP/
SMB/WebDAV 远程路径、目录越界和符号链接逃逸。Root、FTP、已解锁卷共享和匿名写入均为
**待编译/待验证**的 1.2.0 功能。

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

发布构建还需要在源代码之外提供全部 `EDS_RELEASE_*` 签名配置，以及 HTTPS 形式的
`RONGVAULT_SOURCE_URL`。项目不会使用仓库中的签名密钥生成发布包。

## 限制与不支持范围

- 仅面向 VeraCrypt 1.26.29 兼容的**非系统文件容器**；不支持系统加密、物理分区或
  整块磁盘。
- Root 仅用于浏览和管理已挂载文件系统路径；只读挂载、AVB、内核限制和被拒绝的 Root
  请求仍然有效，应用不会写入块设备或重新挂载分区。
- FTP 是用户主动开启的明文本机服务，默认账号登录、2121 端口、只读并监听所有接口；
  匿名登录和匿名写入必须主动开启，内容和凭据均不加密。
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

内置文件管理器借鉴、内置并适配了开源项目
[Material Files（质感文件）](https://github.com/zhanghai/MaterialFiles) 1.7.4 的代码、
文件操作流程和 Material Design 体验。RongVault 对该部分作出了以下适配：

- 文件访问改由 RongVault 的 `UnlockedDocumentsProvider` 提供，使文件管理器只看到当前的
  加密卷会话，而非主机存储路径；
- 新建、复制、移动、删除和重命名均受卷会话及前台操作保护约束，常规卷内操作不使用主机
  明文临时目录；
- Root 通过 libsu 和上游 RootablePath 分发恢复，Shizuku/Sui 仍保持禁用。FTP 页面与服务
  使用受边界保护的 provider 文件系统、不可变共享根快照和 specialUse 前台服务；网络存储、
  APK 安装器和 Material Files 凭据存储仍不启用；
- 集成并适配了 Material Files 的 Markdown 查看器：识别 `.md`、`.markdown`、`.mkd` 文件名
  和 Markdown MIME 类型，使用 Markwon 渲染格式化预览（包括表格、删除线、任务列表和
  本地相对图片），解析卷内相对链接，并可返回源文件编辑。Markdown 渲染默认开启，用户可
  在设置中关闭。

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

本项目 GPLv3 的完整条款见 [LICENSE](LICENSE)。
第三方组件的版权和许可证以其各自文件中的声明为准；发布包含 EDS Lite 来源代码的版本时，
也应随源代码一并保留 EDS Lite 的 GPLv2 许可证文本。
