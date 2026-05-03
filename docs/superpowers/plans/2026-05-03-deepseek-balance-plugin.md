# DeepSeek API 余额显示插件 — 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 构建 TrafficMonitor 插件 DLL，在任务栏显示 DeepSeek API 账户余额和消耗。

**Architecture:** 单 DLL 项目，CDeepSeekPlugin (ITMPlugin 单例) → CDeepSeekItem (IPluginItem) 提供显示文本，WinHTTP 同步请求 balance API，Win32 原生对话框配置，INI 文件持久化。无第三方依赖。

**Tech Stack:** C++17, Windows SDK, WinHTTP, CMake

**项目结构:**
```
DeepSeekPlugin/
├── CMakeLists.txt
├── include/
│   └── PluginInterface.h      # 从 TrafficMonitor 仓库下载
├── src/
│   ├── Config.h / Config.cpp  # INI 配置读写
│   ├── HttpClient.h / .cpp    # WinHTTP GET + JSON 解析
│   ├── DeepSeekItem.h / .cpp  # IPluginItem 实现
│   ├── DeepSeekPlugin.h / .cpp # ITMPlugin 实现 + 配置对话框
│   └── dllmain.cpp            # TMPluginGetInstance 导出
├── docs/
│   └── superpowers/
│       ├── specs/
│       └── plans/
└── README.md
```

---

### Task 1: 项目脚手架

**Files:**
- Create: `include/PluginInterface.h`
- Create: `CMakeLists.txt`
- Create: `src/dllmain.cpp`

- [ ] **Step 1: 下载 PluginInterface.h**

```bash
curl -o "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin/include/PluginInterface.h" \
  "https://raw.githubusercontent.com/zhongyang219/TrafficMonitor/master/include/PluginInterface.h"
```

- [ ] **Step 2: 验证下载成功**

```bash
head -5 "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin/include/PluginInterface.h"
```

Expected: 看到 `#pragma once` 和 `#define PLUGIN_INTERFACE_H`

- [ ] **Step 3: 创建 CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.15)
project(DeepSeekPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(DeepSeekPlugin SHARED
    src/dllmain.cpp
    src/Config.cpp
    src/HttpClient.cpp
    src/DeepSeekItem.cpp
    src/DeepSeekPlugin.cpp
)

target_include_directories(DeepSeekPlugin PRIVATE include src)

target_link_libraries(DeepSeekPlugin PRIVATE
    winhttp
    comctl32
)

set_target_properties(DeepSeekPlugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out"
)

# Disable CRT warnings about unsafe functions
target_compile_definitions(DeepSeekPlugin PRIVATE _CRT_SECURE_NO_WARNINGS)
```

- [ ] **Step 4: 创建 dllmain.cpp**

```cpp
#include "DeepSeekPlugin.h"

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}

extern "C" __declspec(dllexport) ITMPlugin* TMPluginGetInstance()
{
    return &CDeepSeekPlugin::Instance();
}
```

- [ ] **Step 5: 编译空项目验证脚手架正确**

```bash
cd "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin" && mkdir -p build && cd build && cmake .. && cmake --build . --config Release
```

Expected: 编译失败（缺少 DeepSeekPlugin.h），但 CMake 配置成功。

- [ ] **Step 6: Commit**

```bash
git add -A && git commit -m "feat: add project scaffold with CMake and PluginInterface.h"
```

---

### Task 2: Config 模块 — INI 配置读写

**Files:**
- Create: `src/Config.h`
- Create: `src/Config.cpp`

- [ ] **Step 1: 创建 Config.h**

```cpp
#pragma once
#include <string>

struct DeepSeekConfig {
    std::wstring api_key;
    int refresh_interval = 300;   // 秒
    bool show_consumption = true;
    int consumption_period = 0;   // 0=本次运行, 1=当日
};

// 从 INI 文件加载配置
void LoadConfig(DeepSeekConfig& cfg, const wchar_t* ini_path);

// 保存配置到 INI 文件
void SaveConfig(const DeepSeekConfig& cfg, const wchar_t* ini_path);
```

- [ ] **Step 2: 创建 Config.cpp**

```cpp
#include "Config.h"
#include <Windows.h>
#include <string>

static std::wstring ReadString(LPCWSTR path, LPCWSTR key, LPCWSTR def)
{
    wchar_t buf[512];
    GetPrivateProfileStringW(L"DeepSeek", key, def, buf, 512, path);
    return buf;
}

static void WriteString(LPCWSTR path, LPCWSTR key, LPCWSTR val)
{
    WritePrivateProfileStringW(L"DeepSeek", key, val, path);
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
```

- [ ] **Step 3: Commit**

```bash
git add src/Config.h src/Config.cpp && git commit -m "feat: add Config module for INI read/write"
```

---

### Task 3: HttpClient 模块 — WinHTTP 请求 + JSON 解析

**Files:**
- Create: `src/HttpClient.h`
- Create: `src/HttpClient.cpp`

- [ ] **Step 1: 创建 HttpClient.h**

```cpp
#pragma once
#include <string>

enum class FetchResult {
    Ok,
    NoApiKey,
    NetworkError,
    HttpError,
    ParseError
};

struct BalanceInfo {
    double total_balance = 0.0;
    double topped_up_balance = 0.0;
    double granted_balance = 0.0;
};

FetchResult FetchBalance(const std::wstring& api_key, BalanceInfo& out);
const wchar_t* FetchResultToString(FetchResult r);
```

- [ ] **Step 2: 创建 HttpClient.cpp**

```cpp
#include "HttpClient.h"
#include <Windows.h>
#include <winhttp.h>
#include <string>

#pragma comment(lib, "winhttp.lib")

const wchar_t* FetchResultToString(FetchResult r)
{
    switch (r) {
        case FetchResult::Ok:          return L"";
        case FetchResult::NoApiKey:    return L"未配置";
        case FetchResult::NetworkError:return L"[!]";
        case FetchResult::HttpError:   return L"Key 无效";
        case FetchResult::ParseError:  return L"[格式错误]";
    }
    return L"";
}

// 极简 JSON 浮点数提取：在 json 中查找 "key":" 或 "key": 之后的值
static double ExtractFloat(const std::string& json, const char* key)
{
    size_t pos = json.find(key);
    if (pos == std::string::npos) return 0.0;

    pos = json.find(':', pos);
    if (pos == std::string::npos) return 0.0;
    pos++;

    // 跳过空白和引号
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '"' || json[pos] == '\t'))
        pos++;

    char buf[64] = {};
    int i = 0;
    while (pos < json.size() && i < 63 && (isdigit((unsigned char)json[pos]) || json[pos] == '.' || json[pos] == '-')) {
        buf[i++] = json[pos++];
    }
    return atof(buf);
}

FetchResult FetchBalance(const std::wstring& api_key, BalanceInfo& out)
{
    if (api_key.empty())
        return FetchResult::NoApiKey;

    // 解析 host
    std::wstring host = L"api.deepseek.com";
    std::wstring path = L"/user/balance";

    HINTERNET hSession = WinHttpOpen(L"DeepSeekPlugin/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return FetchResult::NetworkError;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return FetchResult::NetworkError;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
        nullptr, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return FetchResult::NetworkError;
    }

    // 设置 Authorization header
    std::wstring auth = L"Bearer " + api_key;
    WinHttpAddRequestHeaders(hRequest, auth.c_str(), (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

    // 发送
    BOOL sent = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!sent) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return FetchResult::NetworkError;
    }

    // 接收响应
    BOOL received = WinHttpReceiveResponse(hRequest, nullptr);
    if (!received) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return FetchResult::NetworkError;
    }

    // 检查 HTTP 状态码
    DWORD statusCode = 0;
    DWORD size = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);

    if (statusCode != 200) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return (statusCode == 401 || statusCode == 403) ? FetchResult::HttpError : FetchResult::NetworkError;
    }

    // 读取响应体
    std::string body;
    DWORD bytesRead = 0;
    char buffer[4096];
    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        body.append(buffer, bytesRead);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (body.empty())
        return FetchResult::ParseError;

    // 解析 JSON
    out.total_balance = ExtractFloat(body, "\"total_balance\"");
    out.topped_up_balance = ExtractFloat(body, "\"topped_up_balance\"");
    out.granted_balance = ExtractFloat(body, "\"granted_balance\"");

    // 检查 is_available
    if (body.find("\"is_available\"") != std::string::npos &&
        body.find("\"is_available\": true") == std::string::npos &&
        body.find("\"is_available\":true") == std::string::npos) {
        out.total_balance = 0.0;
    }

    return FetchResult::Ok;
}
```

- [ ] **Step 3: Commit**

```bash
git add src/HttpClient.h src/HttpClient.cpp && git commit -m "feat: add WinHTTP client for DeepSeek balance API"
```

---

### Task 4: DeepSeekItem — IPluginItem 实现

**Files:**
- Create: `src/DeepSeekItem.h`
- Create: `src/DeepSeekItem.cpp`

- [ ] **Step 1: 创建 DeepSeekItem.h**

```cpp
#pragma once
#include "PluginInterface.h"
#include <string>
#include <chrono>

class CDeepSeekPlugin; // 前向声明

class CDeepSeekItem : public IPluginItem
{
public:
    CDeepSeekItem(CDeepSeekPlugin* owner);

    // 供所属插件调用
    void UpdateDisplayText(double balance, double consumption, bool show_consumption);
    void SetStatusText(const wchar_t* text);
    void SetTooltipText(const std::wstring& text);
    bool IsRefreshRequested() const { return m_requestRefresh; }
    void ClearRefreshRequest() { m_requestRefresh = false; }

    // IPluginItem
    const wchar_t* GetItemName() const override;
    const wchar_t* GetItemId() const override;
    const wchar_t* GetItemLableText() const override;
    const wchar_t* GetItemValueText() const override;
    const wchar_t* GetItemValueSampleText() const override;
    int OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) override;

private:
    CDeepSeekPlugin* m_owner;
    std::wstring m_labelText;
    std::wstring m_valueText;
    std::wstring m_tooltipText;
    bool m_requestRefresh = false;
};
```

- [ ] **Step 2: 创建 DeepSeekItem.cpp**

```cpp
#include "DeepSeekItem.h"
#include <cstdio>

CDeepSeekItem::CDeepSeekItem(CDeepSeekPlugin* owner)
    : m_owner(owner)
    , m_labelText(L"DeepSeek")
    , m_valueText(L"等待首次刷新...")
{
}

void CDeepSeekItem::UpdateDisplayText(double balance, double consumption, bool show_consumption)
{
    wchar_t buf[128];
    if (show_consumption && consumption > 0.01) {
        swprintf_s(buf, L"¥%.2f (-¥%.2f)", balance, consumption);
    } else {
        swprintf_s(buf, L"¥%.2f", balance);
    }
    m_valueText = buf;
}

void CDeepSeekItem::SetStatusText(const wchar_t* text)
{
    m_valueText = text;
}

void CDeepSeekItem::SetTooltipText(const std::wstring& text)
{
    m_tooltipText = text;
}

const wchar_t* CDeepSeekItem::GetItemName() const
{
    return L"DeepSeek 余额";
}

const wchar_t* CDeepSeekItem::GetItemId() const
{
    return L"DeepSeekBalance";
}

const wchar_t* CDeepSeekItem::GetItemLableText() const
{
    return m_labelText.c_str();
}

const wchar_t* CDeepSeekItem::GetItemValueText() const
{
    return m_valueText.c_str();
}

const wchar_t* CDeepSeekItem::GetItemValueSampleText() const
{
    return L"¥888.88 (-¥888.88)";
}

int CDeepSeekItem::OnMouseEvent(MouseEventType type, int /*x*/, int /*y*/, void* /*hWnd*/, int /*flag*/)
{
    if (type == MT_LCLICKED) {
        m_requestRefresh = true;
        return 1;
    }
    return 0;
}
```

- [ ] **Step 3: Commit**

```bash
git add src/DeepSeekItem.h src/DeepSeekItem.cpp && git commit -m "feat: add DeepSeekItem display item implementation"
```

---

### Task 5: DeepSeekPlugin — ITMPlugin 主类 + 配置对话框

**Files:**
- Create: `src/DeepSeekPlugin.h`
- Create: `src/DeepSeekPlugin.cpp`

- [ ] **Step 1: 创建 DeepSeekPlugin.h**

```cpp
#pragma once
#include "PluginInterface.h"
#include "Config.h"
#include "HttpClient.h"
#include "DeepSeekItem.h"
#include <string>
#include <chrono>

class CDeepSeekPlugin : public ITMPlugin
{
public:
    static CDeepSeekPlugin& Instance();

    // 供 DeepSeekItem 使用
    void RequestImmediateRefresh();

    // ITMPlugin
    int GetAPIVersion() override;
    IPluginItem* GetItem(int index) override;
    void DataRequired() override;
    OptionReturn ShowOptionsDialog(void* hParent) override;
    const wchar_t* GetInfo(PluginInfoIndex index) override;
    const wchar_t* GetTooltipInfo() override;
    void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    void OnInitialize(ITrafficMonitor* pApp) override;

private:
    CDeepSeekPlugin();
    ~CDeepSeekPlugin() = default;
    CDeepSeekPlugin(const CDeepSeekPlugin&) = delete;
    CDeepSeekPlugin& operator=(const CDeepSeekPlugin&) = delete;

    void LoadSettings();
    void SaveSettings();
    void DoFetch();

    ITrafficMonitor* m_pApp = nullptr;
    DeepSeekConfig m_config;
    CDeepSeekItem m_item;
    BalanceInfo m_balance;
    double m_lastBalance = -1.0;
    double m_sessionStartBalance = -1.0;
    double m_todayConsumption = 0.0;
    std::chrono::steady_clock::time_point m_lastFetchTime;
    std::chrono::system_clock::time_point m_lastFetchSystemTime;
    bool m_hasError = false;
    FetchResult m_lastResult = FetchResult::Ok;
    std::wstring m_tooltipCache;
    std::wstring m_iniPath;
};
```

- [ ] **Step 2: 创建配置对话框头文件和辅助函数**

在 `DeepSeekPlugin.cpp` 开头写入必要的 include 和对话框控件创建函数：

```cpp
#include "DeepSeekPlugin.h"
#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <cstdio>
#include <ctime>

#pragma comment(lib, "comctl32.lib")

// 在窗口上创建所有配置控件（不使用 .rc 资源文件）
static void CreateConfigControls(HWND hDlg)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // API Key 标签
    CreateWindowW(L"STATIC", L"DeepSeek API Key:",
        WS_CHILD | WS_VISIBLE, 14, 14, 300, 16, hDlg, nullptr, hInst, nullptr);

    // API Key 输入框
    CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        14, 34, 300, 22, hDlg, (HMENU)101, hInst, nullptr);

    // 刷新间隔标签
    CreateWindowW(L"STATIC", L"刷新间隔（秒）:",
        WS_CHILD | WS_VISIBLE, 14, 68, 200, 16, hDlg, nullptr, hInst, nullptr);

    // 刷新间隔输入框
    CreateWindowW(L"EDIT", L"300",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        14, 88, 100, 22, hDlg, (HMENU)102, hInst, nullptr);

    // 刷新间隔微调控件
    CreateWindowW(UPDOWN_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
        0, 0, 0, 0, hDlg, (HMENU)103, hInst, nullptr);
    SendDlgItemMessage(hDlg, 103, UDM_SETBUDDY, (WPARAM)GetDlgItem(hDlg, 102), 0);

    // 显示消耗复选框
    CreateWindowW(L"BUTTON", L"显示最近一次 API 调用消耗",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        14, 124, 300, 20, hDlg, (HMENU)104, hInst, nullptr);

    // 确定 / 取消按钮
    CreateWindowW(L"BUTTON", L"确定",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        148, 160, 80, 26, hDlg, (HMENU)IDOK, hInst, nullptr);
    CreateWindowW(L"BUTTON", L"取消",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        236, 160, 80, 26, hDlg, (HMENU)IDCANCEL, hInst, nullptr);
}
```

- [ ] **Step 3: 实现 CDeepSeekPlugin 核心方法**

```cpp
// ---- CDeepSeekPlugin 单例 ----

CDeepSeekPlugin& CDeepSeekPlugin::Instance()
{
    static CDeepSeekPlugin inst;
    return inst;
}

CDeepSeekPlugin::CDeepSeekPlugin()
    : m_item(this)
{
}

// ---- ITMPlugin 接口 ----

int CDeepSeekPlugin::GetAPIVersion()
{
    return 7;
}

IPluginItem* CDeepSeekPlugin::GetItem(int index)
{
    if (index == 0) return &m_item;
    return nullptr;
}

void CDeepSeekPlugin::DataRequired()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastFetchTime).count();

    // 检查是否达到刷新间隔，或鼠标点击触发了立即刷新
    bool timeToRefresh = (elapsed >= m_config.refresh_interval);
    bool immediateRefresh = m_item.IsRefreshRequested();

    if (timeToRefresh || immediateRefresh) {
        DoFetch();
        m_lastFetchTime = now;
        m_item.ClearRefreshRequest();
    }
}

void CDeepSeekPlugin::DoFetch()
{
    if (m_config.api_key.empty()) {
        m_item.SetStatusText(L"未配置");
        m_item.SetTooltipText(L"DeepSeek API 余额\n请右键 → 插件管理 → 选项设置 API Key");
        return;
    }

    BalanceInfo info;
    FetchResult result = FetchBalance(m_config.api_key, info);

    if (result == FetchResult::Ok) {
        double currentBalance = info.total_balance;

        // 初始化 session 基准余额
        if (m_sessionStartBalance < 0)
            m_sessionStartBalance = currentBalance;

        // 计算消耗
        double consumption = 0.0;
        if (m_lastBalance > 0 && currentBalance < m_lastBalance) {
            consumption = m_lastBalance - currentBalance;
        }

        if (m_config.consumption_period == 0) {
            // 本次运行消耗
            if (m_sessionStartBalance > currentBalance)
                consumption = m_sessionStartBalance - currentBalance;
            else
                consumption = 0.0;
        }
        // consumption_period == 1 当日消耗（用 m_lastBalance 差值近似）

        m_item.UpdateDisplayText(currentBalance, consumption, m_config.show_consumption);
        m_lastBalance = currentBalance;
        m_hasError = false;
        m_lastFetchSystemTime = std::chrono::system_clock::now();

        // 构建 tooltip
        auto tt = std::chrono::system_clock::to_time_t(m_lastFetchSystemTime);
        tm localTime;
        localtime_s(&localTime, &tt);
        wchar_t timeBuf[64];
        wcsftime(timeBuf, 64, L"%Y-%m-%d %H:%M:%S", &localTime);

        wchar_t tipBuf[512];
        swprintf_s(tipBuf,
            L"DeepSeek API 余额\n"
            L"余额: ¥%.2f\n"
            L"充值余额: ¥%.2f\n"
            L"赠送余额: ¥%.2f\n"
            L"刷新间隔: %d秒\n"
            L"上次刷新: %s",
            info.total_balance,
            info.topped_up_balance,
            info.granted_balance,
            m_config.refresh_interval,
            timeBuf);
        m_tooltipCache = tipBuf;
    } else {
        m_hasError = true;
        m_lastResult = result;
        if (m_lastBalance < 0) {
            m_item.SetStatusText(FetchResultToString(result));
        } else {
            // 保留上次缓存值 + 错误标记
            wchar_t buf[128];
            swprintf_s(buf, L"¥%.2f %s", m_lastBalance, FetchResultToString(result));
            m_item.SetStatusText(buf);
        }
        m_tooltipCache = L"DeepSeek API 余额\n错误: ";
        m_tooltipCache += FetchResultToString(result);
    }

    m_item.SetTooltipText(m_tooltipCache);
}

OptionReturn CDeepSeekPlugin::ShowOptionsDialog(void* hParent)
{
    // 加载对话框用的 CommCtrl
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_UPDOWN_CLASS };
    InitCommonControlsEx(&icex);

    // 注册对话框类
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefDlgProcW;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"DeepSeekConfigDlg";
    RegisterClassExW(&wc);

    // 创建对话框窗口（手动布局，不使用 .rc）
    HWND hDlg = CreateWindowExW(0, L"DeepSeekConfigDlg", L"DeepSeek 插件设置",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 340, 230,
        (HWND)hParent, nullptr, GetModuleHandle(nullptr), nullptr);

    if (!hDlg) return OR_OPTION_NOT_PROVIDED;

    CreateConfigControls(hDlg);

    // 用临时数据初始化
    DialogData data;
    data.cfg = &m_config;
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)&data);

    // 初始化控件值
    SetDlgItemTextW(hDlg, 101, m_config.api_key.c_str());
    wchar_t buf[32];
    swprintf_s(buf, L"%d", m_config.refresh_interval);
    SetDlgItemTextW(hDlg, 102, buf);
    SendDlgItemMessage(hDlg, 103, UDM_SETRANGE, 0, MAKELPARAM(3600, 10));
    SendDlgItemMessage(hDlg, 104, BM_SETCHECK,
        m_config.show_consumption ? BST_CHECKED : BST_UNCHECKED, 0);

    // 居中于父窗口
    if (hParent) {
        RECT rcParent, rcDlg;
        GetWindowRect((HWND)hParent, &rcParent);
        GetWindowRect(hDlg, &rcDlg);
        int dlgW = rcDlg.right - rcDlg.left;
        int dlgH = rcDlg.bottom - rcDlg.top;
        int x = rcParent.left + ((rcParent.right - rcParent.left) - dlgW) / 2;
        int y = rcParent.top + ((rcParent.bottom - rcParent.top) - dlgH) / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    ShowWindow(hDlg, SW_SHOW);

    // 模态消息循环
    bool changed = false;
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
            // Enter = OK
            wchar_t keyBuf[512], intervalBuf[32];
            GetDlgItemTextW(hDlg, 101, keyBuf, 512);
            GetDlgItemTextW(hDlg, 102, intervalBuf, 32);
            m_config.api_key = keyBuf;
            m_config.refresh_interval = _wtoi(intervalBuf);
            if (m_config.refresh_interval < 10) m_config.refresh_interval = 10;
            m_config.show_consumption =
                (SendDlgItemMessage(hDlg, 104, BM_GETCHECK, 0, 0) == BST_CHECKED);
            changed = true;
            DestroyWindow(hDlg);
            continue;
        }
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            DestroyWindow(hDlg);
            continue;
        }
        if (msg.message == WM_CLOSE || msg.message == WM_DESTROY) {
            DestroyWindow(hDlg);
            continue;
        }
        if (msg.message == WM_COMMAND && LOWORD(msg.wParam) == IDOK) {
            wchar_t keyBuf[512], intervalBuf[32];
            GetDlgItemTextW(hDlg, 101, keyBuf, 512);
            GetDlgItemTextW(hDlg, 102, intervalBuf, 32);
            m_config.api_key = keyBuf;
            m_config.refresh_interval = _wtoi(intervalBuf);
            if (m_config.refresh_interval < 10) m_config.refresh_interval = 10;
            m_config.show_consumption =
                (SendDlgItemMessage(hDlg, 104, BM_GETCHECK, 0, 0) == BST_CHECKED);
            changed = true;
            DestroyWindow(hDlg);
            continue;
        }
        if (msg.message == WM_COMMAND && LOWORD(msg.wParam) == IDCANCEL) {
            DestroyWindow(hDlg);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (changed) SaveSettings();
    return changed ? OR_OPTION_CHANGED : OR_OPTION_UNCHANGED;
}

const wchar_t* CDeepSeekPlugin::GetInfo(PluginInfoIndex index)
{
    switch (index) {
        case TMI_NAME:        return L"DeepSeek 余额";
        case TMI_DESCRIPTION: return L"显示 DeepSeek API 账户余额和消耗信息";
        case TMI_AUTHOR:      return L"ShangKeQian";
        case TMI_COPYRIGHT:   return L"MIT";
        case TMI_VERSION:     return L"1.0.0";
        case TMI_URL:         return L"https://github.com/ShangKeQian/TrafficMonitor-DeepSeekPlugin";
        default:              return L"";
    }
}

const wchar_t* CDeepSeekPlugin::GetTooltipInfo()
{
    return m_tooltipCache.empty() ? L"DeepSeek 余额" : m_tooltipCache.c_str();
}

void CDeepSeekPlugin::OnExtenedInfo(ExtendedInfoIndex, const wchar_t*) {}

void CDeepSeekPlugin::OnInitialize(ITrafficMonitor* pApp)
{
    m_pApp = pApp;
    LoadSettings();
}

void CDeepSeekPlugin::RequestImmediateRefresh()
{
    // DataRequired 中通过 m_item.IsRefreshRequested() 检查
}

void CDeepSeekPlugin::LoadSettings()
{
    if (m_pApp) {
        m_iniPath = m_pApp->GetPluginConfigDir();
        m_iniPath += L"\\DeepSeekPlugin.ini";
    } else {
        m_iniPath = L"DeepSeekPlugin.ini";
    }
    LoadConfig(m_config, m_iniPath.c_str());
}

void CDeepSeekPlugin::SaveSettings()
{
    SaveConfig(m_config, m_iniPath.c_str());
}
```

- [ ] **Step 4: Commit**

```bash
git add src/DeepSeekPlugin.h src/DeepSeekPlugin.cpp && git commit -m "feat: add DeepSeekPlugin main class with config dialog and fetch logic"
```

---

### Task 6: 构建验证

- [ ] **Step 1: CMake 生成并编译**

```bash
cd "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin" && rm -rf build && mkdir build && cd build && cmake .. && cmake --build . --config Release
```

Expected: 编译成功，`build/out/DeepSeekPlugin.dll` 生成。

- [ ] **Step 2: 检查 DLL 导出符号**

```bash
dumpbin /EXPORTS "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin/build/out/DeepSeekPlugin.dll"
```

Expected: 看到 `TMPluginGetInstance` 在导出表中。

- [ ] **Step 3: 验证 DLL 依赖**

```bash
dumpbin /DEPENDENTS "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin/build/out/DeepSeekPlugin.dll"
```

Expected: 仅有 Windows 系统 DLL + winhttp.dll，无其他第三方依赖。

- [ ] **Step 4: Commit**

```bash
git add -A && git commit -m "chore: verify build output and DLL exports"
```

---

### Task 7: README 文档

**Files:**
- Create: `README.md`

- [ ] **Step 1: 创建 README.md**

```markdown
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

输出：`build/out/DeepSeekPlugin.dll`

无外部依赖，仅需 Windows SDK + CMake + MSVC。

## 许可

MIT
```

- [ ] **Step 2: Commit**

```bash
git add README.md && git commit -m "docs: add README"
```

---

### Task 8: 最终验证与推送

- [ ] **Step 1: 清理构建，重新完整编译**

```bash
cd "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin" && rm -rf build && mkdir build && cd build && cmake .. && cmake --build . --config Release 2>&1
```

Expected: 零错误零警告编译通过。

- [ ] **Step 2: 推送所有提交到 GitHub**

```bash
cd "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin" && git push origin main
```

Expected: 推送成功。

- [ ] **Step 3: 最终检查 git log**

```bash
cd "C:/Users/商克谦/OneDrive/Desktop/DeepSeekPlugin" && git log --oneline
```

Expected: 看到 8 个清晰分步的 commit。
