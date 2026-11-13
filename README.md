# blackscreen

一个极简的 Windows 黑屏工具：运行后**除任务栏外全部变黑**，窗口不置顶，双击即可最小化到任务栏。

单文件 C 源码，纯 Win32 API，零第三方依赖，产物只有一个约 130 KB 的 exe。

## 操作

| 操作 | 结果 |
| --- | --- |
| 双击 `blackscreen.exe` | 主显示器工作区铺满纯黑，任务栏保持可见 |
| 双击黑屏 | 最小化到任务栏（点任务栏按钮还原） |
| `Esc` / 鼠标右键 / `Alt+F4` | 退出 |
| 任务栏按钮右键 → 关闭 | 退出 |

- **不置顶**：普通窗口层级，其他窗口能盖住它，`Alt+Tab` 可自由切走。
- 任务栏始终可见，且本程序在任务栏有自己的按钮，所以最小化是真·最小化到任务栏。
- 适合演示息屏、遮挡屏幕、录屏/拍照时当纯黑背景。

## 下载

到 [Releases](https://github.com/gdtools/blackscreen/releases) 下载 `blackscreen.exe`（x64）。静态链接，只依赖 Windows 自带的 `KERNEL32.dll` / `USER32.dll` / `GDI32.dll` / `msvcrt.dll`，**无需 VC++ 运行库**。每个 Release 附带 `blackscreen.exe.sha256` 校验和。

## 本地编译

MinGW-w64（Git Bash / MSYS2）：

```bash
gcc -O2 -Wall -Wextra -mwindows -static blackscreen.c -o blackscreen.exe
```

MSVC（Developer Command Prompt）：

```bat
cl /O2 /W4 blackscreen.c /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib
```

Linux 交叉编译（与 CI 一致）：

```bash
sudo apt-get install -y mingw-w64
x86_64-w64-mingw32-gcc -O2 -Wall -Wextra -mwindows -static blackscreen.c -o blackscreen.exe
```

## 自动发布

由 [`.github/workflows/release.yml`](.github/workflows/release.yml) 提供，构建环境为 `ubuntu-latest` + MinGW-w64 交叉编译。

| 触发条件 | 动作 |
| --- | --- |
| `push` / `pull_request` 到 `main` | 编译校验，上传 `blackscreen-x64` 构建产物（exe + sha256） |
| `push` `v*` 标签 | 编译并**自动创建 Release**，附带 exe 与 sha256，自动生成 Release Notes |
| `workflow_dispatch` | 手动跑一次编译校验 |

发布新版本：

```bash
git tag v1.0.1
git push origin v1.0.1
```

推完标签约一分钟，[Releases](https://github.com/gdtools/blackscreen/releases) 里会自动出现 `v1.0.1`。仓库里**不存放 exe**，所有二进制都由 Actions 生成。

## 实现要点

| 需求 | 关键实现 |
| --- | --- |
| 除任务栏外全黑 | `SystemParametersInfoW(SPI_GETWORKAREA)` 取工作区，`WM_PAINT` 用 `BLACK_BRUSH` 填充客户区 |
| 不置顶 | `SetWindowPos(hwnd, HWND_NOTOPMOST, ...)`，不使用 `WS_EX_TOPMOST` |
| 最小化到任务栏 | `WS_EX_APPWINDOW`（popup 窗口默认不进任务栏，必须加） |
| 双击生效 | 窗口类注册 `CS_DBLCLKS`，处理 `WM_LBUTTONDBLCLK` → `ShowWindow(SW_MINIMIZE)` |
| 无边框黑幕 | `WS_POPUP \| WS_SYSMENU \| WS_VISIBLE`，`SYSMENU` 用于任务栏右键关闭 |
| 分辨率 / 任务栏变化 | `WM_DISPLAYCHANGE`、`WM_SETTINGCHANGE` 重新贴合工作区；已最小化时跳过，避免被弹回 |
| 不闪烁 | `WM_ERASEBKGND` 直接返回 1，绘制统一交给 `WM_PAINT` |

## 已知限制

- 只覆盖**主显示器**工作区。要多屏全黑，用 `EnumDisplayMonitors` + `GetMonitorInfo` 为每块屏各建一个窗口。
- 任务栏设为「自动隐藏」时工作区等于整屏，黑幕会铺满整屏（任务栏本来也不可见）。
- 只提供 x64 构建。
