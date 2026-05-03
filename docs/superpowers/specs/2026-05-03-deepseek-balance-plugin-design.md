# DeepSeek API 余额显示插件 — 设计文档

## 概述

为 TrafficMonitor 开发一个插件，在任务栏/主窗口中实时显示 DeepSeek API 账户余额和消耗信息。

## 整体架构

```
┌──────────────────────────────────────┐
│         TrafficMonitor 主程序         │
│  ┌────────────────────────────────┐  │
│  │  定时调用 DataRequired()        │  │
│  │  调用 GetItemLableText()       │  │
│  │  调用 GetItemValueText()       │  │
│  └───────────┬────────────────────┘  │
└──────────────┼───────────────────────┘
               │
┌──────────────▼───────────────────────┐
│     DeepSeekPlugin.dll               │
│  ┌────────────────────────────────┐  │
│  │  CDeepSeekPlugin : ITMPlugin   │  │
│  │  - 单例模式                     │  │
│  │  - 成员: CDeepSeekItem          │  │
│  └───────────┬────────────────────┘  │
│  ┌───────────▼────────────────────┐  │
│  │  CDeepSeekItem : IPluginItem   │  │
│  │  - 缓存余额和消耗字符串          │  │
│  └────────────────────────────────┘  │
│  ┌────────────────────────────────┐  │
│  │  Config (API Key, 刷新间隔等)   │  │
│  └────────────────────────────────┘  │
│  ┌────────────────────────────────┐  │
│  │  HTTP Client (WinHTTP)         │  │
│  └────────────────────────────────┘  │
└──────────────────────────────────────┘
               │
               ▼
   https://api.deepseek.com/user/balance
```

## 组件设计

### 1. CDeepSeekPlugin — 插件主类

实现 `ITMPlugin` 接口，单例模式。

- `GetItem(int index)`: 仅有一个显示项，index=0 返回 `CDeepSeekItem` 指针
- `DataRequired()`: 检查刷新间隔，达到阈值则发起 HTTP 请求
- `GetInfo()`: 返回插件名称、版本、作者等信息
- `ShowOptionsDialog()`: 弹出配置对话框（API Key、刷新间隔、是否显示消耗）
- `GetTooltipInfo()`: 返回详细的余额和消耗信息
- `OnInitialize()`: 获取 `ITrafficMonitor*` 指针，加载配置文件
- `OnMouseEvent()`: 委托给 `CDeepSeekItem`
- `OnExtenedInfo()`: 接收主题色等扩展信息

### 2. CDeepSeekItem — 显示项

实现 `IPluginItem` 接口，`IsCustomDraw()` 返回 `false`，依赖主程序绘制。

- 缓存格式化后的余额和消耗字符串
- `GetItemLableText`: 返回 `"DeepSeek"`
- `GetItemValueText`: 返回格式化余额（和可选的消耗）
- `GetItemValueSampleText`: 返回 `"¥888.88 (-¥888.88)"` 用于宽度计算
- `OnMouseEvent`: 左键单击触发立即刷新

### 3. HTTP 请求（WinHTTP）

```
GET https://api.deepseek.com/user/balance
Authorization: Bearer {api_key}
```

- 同步请求，超时 10 秒
- 不引入第三方 JSON 库，手动解析响应字段
- 余额消耗 = 上次余额 - 当前余额（仅在余额减少且上次 > 0 时计算）

### 4. 配置管理

通过 `ShowOptionsDialog` 弹窗设置，存储到 `{plugin_config_dir}/DeepSeekPlugin.ini`。

| 配置项 | 类型 | 默认值 | 说明 |
|--------|------|--------|------|
| api_key | string | 空 | DeepSeek API 密钥 |
| refresh_interval | int | 300 | 刷新间隔（秒） |
| show_consumption | bool | true | 是否显示最近消耗 |
| consumption_period | int | 0 | 消耗统计周期（0=本次运行, 1=当日） |

### 5. 显示文本格式

**标签**: `DeepSeek`

**数值**（显示消耗）: `¥12.50 (-¥0.03)`

**数值**（不显示消耗）: `¥12.50`

**示例文本**: `¥888.88 (-¥888.88)`

**鼠标提示**:
```
DeepSeek API 余额
余额: ¥12.50
今日消耗: ¥0.15
刷新间隔: 300秒
上次刷新: 2026-05-03 21:30:15
```

## 错误处理

| 场景 | 用户可见行为 |
|------|-------------|
| API Key 未配置 | 数值显示 "未配置" |
| 网络请求失败/超时 | 保留上次缓存值，追加 `[!]` |
| API 返回错误 (401/429等) | 数值显示 "Key 无效"/"请求频繁" |
| JSON 解析失败 | 保留上次缓存值，追加 `[格式错误]` |
| 首次加载无网络 | 数值显示 "等待首次刷新..." |

所有错误状态下标签始终显示 "DeepSeek"。

## 鼠标事件

- **左键单击**: 立即触发一次余额刷新
- **右键单击**: 委托主程序默认行为（弹出菜单）

## 测试策略

### 开发调试
- 使用 PluginTester 工具验证显示和刷新逻辑

### 集成测试
- DLL 部署到 TrafficMonitor 的 `plugins/` 目录
- 验证自动加载、插件管理列表、选项对话框功能

### 边界情况
- API Key 为空、余额为 0、余额为负数
- 刷新间隔设为极小值（1 秒）
- 深色/浅色模式切换

## 技术选型

- **语言**: C++ (C++17)
- **界面**: 无额外框架，依赖 TrafficMonitor 主程序绘制
- **HTTP**: WinHTTP（Windows 自带）
- **JSON**: 手动解析（字段简单，不引入第三方库）
- **持久化**: INI 文件（Windows API `GetPrivateProfileString`/`WritePrivateProfileString`）
- **依赖**: 仅 `PluginInterface.h` + Windows SDK
