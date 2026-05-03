#include "Config.h"
#include <Windows.h>

namespace {
    constexpr DWORD kMaxValueLen = 2048;
}

static std::wstring ReadString(LPCWSTR path, LPCWSTR key, LPCWSTR def)
{
    wchar_t buf[kMaxValueLen];
    DWORD len = GetPrivateProfileStringW(L"DeepSeek", key, def, buf, kMaxValueLen, path);
    // 缓冲区不足时返回 buf_size-1，超出部分被静默截断
    if (len >= kMaxValueLen - 1)
        return def;
    return buf;
}

static bool WriteString(LPCWSTR path, LPCWSTR key, LPCWSTR val)
{
    return WritePrivateProfileStringW(L"DeepSeek", key, val, path) != 0;
}

void LoadConfig(DeepSeekConfig& cfg, const wchar_t* ini_path)
{
    cfg.api_key = ReadString(ini_path, L"api_key", L"");
    cfg.refresh_interval = GetPrivateProfileIntW(L"DeepSeek", L"refresh_interval", 300, ini_path);
    cfg.show_consumption = GetPrivateProfileIntW(L"DeepSeek", L"show_consumption", 1, ini_path) != 0;
    cfg.consumption_period = GetPrivateProfileIntW(L"DeepSeek", L"consumption_period", 0, ini_path);
}

void SaveConfig(const DeepSeekConfig& cfg, const wchar_t* ini_path)
{
    WriteString(ini_path, L"api_key", cfg.api_key.c_str());

    wchar_t buf[32];
    _itow_s(cfg.refresh_interval, buf, 32, 10);
    WriteString(ini_path, L"refresh_interval", buf);

    WriteString(ini_path, L"show_consumption", cfg.show_consumption ? L"1" : L"0");

    _itow_s(cfg.consumption_period, buf, 32, 10);
    WriteString(ini_path, L"consumption_period", buf);
}
