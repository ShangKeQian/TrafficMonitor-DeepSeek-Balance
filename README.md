# TrafficMonitor DeepSeek 余额插件

在 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 任务栏中显示 DeepSeek API 账户余额。

## 功能

- 显示 DeepSeek API 余额（CNY）
- 显示余额消耗（可选）
- 可配置的刷新间隔
- 单击余额立即刷新

## 安装

1. 从 [Releases](../../releases) 下载 `DeepSeekPlugin.dll`
2. 放到 TrafficMonitor 安装目录下的 `plugins` 文件夹
3. 重启 TrafficMonitor
4. 右键菜单 → 其他功能 → 插件管理 → DeepSeek 余额 → 选项，设置 API Key

## 获取 API Key

前往 [DeepSeek API Keys](https://platform.deepseek.com/api_keys) 创建。

## 构建

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

输出：`build/out/Release/DeepSeekPlugin.dll`

无外部依赖，仅需 Windows SDK + CMake + MSVC。

## 许可

MIT
