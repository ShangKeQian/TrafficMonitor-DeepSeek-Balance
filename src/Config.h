#pragma once
#include <string>

struct DeepSeekConfig {
    std::wstring api_key;
    std::wstring label_text;      // 标签文本，空字符串则不显示标签
    int refresh_interval = 300;   // 秒
    bool show_consumption = true;
    int consumption_period = 0;   // 0=本次运行, 1=当日
};

// 从 INI 文件加载配置
void LoadConfig(DeepSeekConfig& cfg, const wchar_t* ini_path);

// 保存配置到 INI 文件
void SaveConfig(const DeepSeekConfig& cfg, const wchar_t* ini_path);
