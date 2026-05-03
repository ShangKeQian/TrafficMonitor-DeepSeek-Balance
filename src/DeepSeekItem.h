#pragma once
#include "PluginInterface.h"
#include <string>
#include <atomic>
#include <mutex>

class CDeepSeekPlugin;

class CDeepSeekItem : public IPluginItem
{
public:
    CDeepSeekItem(CDeepSeekPlugin* owner);

    void UpdateDisplayText(const std::wstring& label, double balance, double consumption, bool show_consumption);
    void SetStatusText(const wchar_t* text);
    void SetTooltipText(const std::wstring& text);
    bool IsRefreshRequested() const { return m_requestRefresh.load(std::memory_order_acquire); }
    void ClearRefreshRequest() { m_requestRefresh.store(false, std::memory_order_release); }

    // IPluginItem
    const wchar_t* GetItemName() const override;
    const wchar_t* GetItemId() const override;
    const wchar_t* GetItemLableText() const override;
    const wchar_t* GetItemValueText() const override;
    const wchar_t* GetItemValueSampleText() const override;
    int OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) override;

private:
    CDeepSeekPlugin* m_owner;
    mutable std::mutex m_mutex;
    std::wstring m_labelText;
    std::wstring m_valueText;
    std::wstring m_tooltipText;
    std::atomic<bool> m_requestRefresh{false};
};
