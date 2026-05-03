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
    int GetAPIVersion() const override;
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
