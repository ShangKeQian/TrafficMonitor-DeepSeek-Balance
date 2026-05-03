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

int CDeepSeekPlugin::GetAPIVersion() const
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

        if (m_sessionStartBalance < 0)
            m_sessionStartBalance = currentBalance;

        double consumption = 0.0;
        if (m_config.consumption_period == 0) {
            if (m_sessionStartBalance > currentBalance)
                consumption = m_sessionStartBalance - currentBalance;
        } else {
            if (m_lastBalance > 0 && currentBalance < m_lastBalance)
                consumption = m_lastBalance - currentBalance;
        }

        m_item.UpdateDisplayText(currentBalance, consumption, m_config.show_consumption);
        m_lastBalance = currentBalance;
        m_lastFetchSystemTime = std::chrono::system_clock::now();

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
        {
            std::lock_guard<std::mutex> lock(m_tooltipMutex);
            m_tooltipCache = tipBuf;
        }
    } else {
        if (m_lastBalance < 0) {
            m_item.SetStatusText(FetchResultToString(result));
        } else {
            wchar_t buf[128];
            swprintf_s(buf, L"¥%.2f %s", m_lastBalance, FetchResultToString(result));
            m_item.SetStatusText(buf);
        }
        {
            std::lock_guard<std::mutex> lock(m_tooltipMutex);
            m_tooltipCache = L"DeepSeek API 余额\n错误: ";
            m_tooltipCache += FetchResultToString(result);
        }
    }

    m_item.SetTooltipText(m_tooltipCache);
}

ITMPlugin::OptionReturn CDeepSeekPlugin::ShowOptionsDialog(void* hParent)
{
    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_UPDOWN_CLASS };
    InitCommonControlsEx(&icex);

    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = DefWindowProcW;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = L"DeepSeekConfigDlg";
        if (RegisterClassExW(&wc))
            classRegistered = true;
    }

    HWND hDlg = CreateWindowExW(0, L"DeepSeekConfigDlg", L"DeepSeek 插件设置",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 340, 230,
        (HWND)hParent, nullptr, GetModuleHandle(nullptr), nullptr);

    if (!hDlg) return OR_OPTION_NOT_PROVIDED;

    CreateConfigControls(hDlg);

    SetDlgItemTextW(hDlg, 101, m_config.api_key.c_str());
    wchar_t buf[32];
    swprintf_s(buf, L"%d", m_config.refresh_interval);
    SetDlgItemTextW(hDlg, 102, buf);
    SendDlgItemMessage(hDlg, 103, UDM_SETRANGE, 0, MAKELPARAM(3600, 10));
    SendDlgItemMessage(hDlg, 104, BM_SETCHECK,
        m_config.show_consumption ? BST_CHECKED : BST_UNCHECKED, 0);

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

    auto applyConfig = [&]() {
        wchar_t keyBuf[512], intervalBuf[32];
        GetDlgItemTextW(hDlg, 101, keyBuf, 512);
        GetDlgItemTextW(hDlg, 102, intervalBuf, 32);
        m_config.api_key = keyBuf;
        m_config.refresh_interval = _wtoi(intervalBuf);
        if (m_config.refresh_interval < 10) m_config.refresh_interval = 10;
        m_config.show_consumption =
            (SendDlgItemMessage(hDlg, 104, BM_GETCHECK, 0, 0) == BST_CHECKED);
    };

    bool changed = false;
    MSG msg;
    BOOL ret;
    while ((ret = GetMessage(&msg, hDlg, 0, 0)) > 0) {
        if (IsDialogMessage(hDlg, &msg))
            continue;

        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
            applyConfig();
            changed = true;
            break;
        }
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            break;
        }
        if (msg.message == WM_CLOSE) {
            break;
        }
        if (msg.message == WM_COMMAND && LOWORD(msg.wParam) == IDOK) {
            applyConfig();
            changed = true;
            break;
        }
        if (msg.message == WM_COMMAND && LOWORD(msg.wParam) == IDCANCEL) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(hDlg);

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
    std::lock_guard<std::mutex> lock(m_tooltipMutex);
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
    m_lastFetchTime = std::chrono::steady_clock::time_point();
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
