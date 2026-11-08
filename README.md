# ELAN Fingerprint Unlock for macOS

为 ASUS M7400QC 的 ELAN 04f3:0c6e 指纹模块提供实验性 macOS 锁屏解锁。

真实 USB 指纹采集 → 图像处理与局部特征匹配 → 匹配成功 → 读取本机钥匙串密码 → 确认 loginwindow 的空安全密码框 → 输入密码并提交。

采集通信依据 elanpress 协议及 elan-fp-fix 的状态查询方式；使用 filip-rs/libfprint 最新 elanpress 的 SIFT 风格局部特征/几何一致性匹配，不是 NBIS，也不是仅检测触碰。

## 实测结果

在实测设备（ASUS M7400QC，ELAN 04f3:0c6e，150 × 52）上验证通过：

- 录入：同一根食指录入 8 次完成建模
- 解锁：录入的食指可以稳定解锁
- 拒绝：其他手指无法解锁
- 锁屏：已完成真实锁屏解锁

实测准确性很高。尚未测定正式误识率，也没有活体防伪保证。

## 安装和使用

从 Releases 下载 DMG，拖到 Applications。启动后：

1. 移开手指，点击录入，按提示用同一根手指录入 8 次；轻触，不要按下电源键。
2. 在应用的安全文本框输入当前用户的 macOS 登录密码并保存。应用用 OpenDirectory 在本机验证后，存入当前用户钥匙串。
3. 在系统设置 → 隐私与安全性 → 辅助功能中授权此应用，再点击启用。钥匙串提示应由用户自行确认。
4. 按 Control + Command + Q 锁屏，点一下密码框，松开后轻触录入的手指。

关闭窗口后保留菜单栏指纹图标，无 Dock 图标。菜单可以停止监听或退出。没有自动安装启动项。

## 限制与数据处理

- 仅验证 ELAN USB 04f3:0c6e、150 × 52；其他型号拒绝启动。
- 密码与模板保存在本机钥匙串，发布包不含任何模板或密码；没有联网代码或遥测。
- 匹配成功后才读取密码；重复检查锁屏状态、loginwindow 与空安全密码框。每次锁屏最多 4 次不匹配，最多尝试一次密码提交。
- macOS Secure Input、权限、输入焦点或固件差异都可能阻止自动输入。可以随时使用系统密码登录。
- 不是原生 Touch ID，不支持 Apple Touch ID API、Apple Pay、FileVault 预启动或冷启动首次登录，也不处理其他应用的密码弹窗。
- 登录钥匙串若锁定，自动读取可能失败。更新临时签名后可能需要移除旧辅助功能条目，再授权新版本。
- 本地钥匙串凭据和普通进程中的匹配无法提供 Secure Enclave 级别的隔离。同一用户下的恶意软件可能破坏此方案；它是便捷解锁实验。
- 本项目使用通用 Bundle ID/钥匙串服务名，不导入任何开发机器上的旧数据。

## 构建与检查

需要 Intel macOS 和 Xcode Command Line Tools。无需 Homebrew，无需管理员权限。运行 ./build.sh，产物位于 dist/。可运行 ./build/ELANFingerprint.app/Contents/MacOS/ElanUnlock --self-test 检查合成图匹配。编译器和 SDK 环境差异可能影响可复现性。合成图自检仅验证算法基本路径；真实设备验证需用户配合按压。

## 实测电脑

| 项目 | 规格 |
| --- | --- |
| 笔记本 | ASUS Vivobook Pro 14X OLED M7400QC |
| CPU | AMD Ryzen 7 5800H，8 核 16 线程 |
| 核显 | AMD Radeon Vega 8 / Renoir，系统报告 512 MB |
| 独显 | NVIDIA GeForce RTX 3050 Laptop GPU，4 GB GDDR6 |
| 内存 | 16 GB DDR4-3200 |
| 内屏 | 14 英寸 OLED，2880 × 1800，90 Hz |
| 外接屏 | Lenovo Y27q-30，2560 × 1440，144 Hz，当前由 AMD 输出 |
| 系统 | macOS Sequoia 15.8.1，x86_64，OpenCore |
| 指纹模块 | ELAN USB 04f3:0c6e，采集尺寸 150 × 52 |

硬件型号相同不保证固件和驱动行为一致。

## 上游和许可证

- 设计参考：[hackintosh-user/hack-touchid](https://github.com/hackintosh-user/hack-touchid)。本项目没有复制其认证守护进程；菜单、钥匙串和锁屏保护逻辑为独立实现。
- [filip-rs/libfprint](https://github.com/filip-rs/libfprint/tree/elanpress)：图像处理和特征匹配，© Filip Spanne，LGPL-2.1-or-later。
- [casimir-engineering/elan-fp-fix](https://github.com/casimir-engineering/elan-fp-fix)：传感器状态读取与图像轮询的参考。
- [libusb/libusb](https://github.com/libusb/libusb)：用户态 USB，LGPL-2.1-or-later。

本项目以 LGPL-2.1-or-later 发布。完整 vendored libusb 源码、所用 matcher 源码、构建脚本与上游提交标识随仓库提供；保留原作者版权与许可证。可修改、替换库并重新链接构建。
