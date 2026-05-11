# TrafficMonitor DeepSeek 余额插件

在 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 任务栏中实时显示 DeepSeek API 账户余额。

![screenshot](https://img.shields.io/badge/platform-Windows-blue)
![license](https://img.shields.io/github/license/ShangKeQian/TrafficMonitor-DeepSeek-Balance)
![release](https://img.shields.io/github/v/release/ShangKeQian/TrafficMonitor-DeepSeek-Balance)

## 功能

- 任务栏实时显示 DeepSeek API 余额（CNY）
- 可选显示余额消耗（支持两种统计周期）
- 自定义刷新间隔（10~3600 秒）
- 左键单击立即刷新余额
- 支持深色/浅色模式
- Tooltip 显示详细信息（余额、充值余额、赠送余额、上次刷新时间）

## 截图

任务栏显示效果：
```
┌──────────┐
│ ¥888.88  │
│ (¥0.15)  │
└───────── ┘
```

## 安装

1. 从 [Releases](https://github.com/ShangKeQian/TrafficMonitor-DeepSeek-Balance/releases/latest) 下载 `DeepSeekPlugin.dll`
2. 将 DLL 放到 TrafficMonitor 安装目录下的 `plugins` 文件夹
3. 重启 TrafficMonitor
4. 右键菜单 → 常规设置 → 插件管理 → DeepSeek 余额 → 选项，设置 API Key

## 配置说明

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| API Key | 空 | DeepSeek API 密钥 |
| 刷新间隔 | 300 秒 | 余额自动刷新间隔 |
| 显示消耗 | 开启 | 是否在余额下方显示消耗 |
| 消耗统计周期 | 本次运行消耗 | `本次运行消耗`：从启动开始累计；`每次调用消耗`：仅显示最近一次 API 调用的消耗 |

## 余额消耗计算方式

- **本次运行消耗**：`启动时余额 - 当前余额`（余额增加时，如充值后，重新计算基准）
- **每次调用消耗**：`上次余额 - 当前余额`（仅在余额减少时显示）

## 获取 API Key

前往 [DeepSeek API Keys](https://platform.deepseek.com/api_keys) 创建。

## 构建

**环境要求：** Windows SDK + CMake 3.15+ + MSVC (Visual Studio 2022)

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

输出：`build/out/Release/DeepSeekPlugin.dll`

无任何第三方依赖，仅使用 Windows SDK 自带的 WinHTTP。

## 技术实现

- **语言**：C++17
- **HTTP**：WinHTTP（同步请求，10 秒超时）
- **JSON 解析**：手动提取（无第三方库）
- **配置持久化**：INI 文件（Windows API）
- **界面**：原生 Win32 控件 + GDI 自定义绘制

## 许可

[MIT](LICENSE)
