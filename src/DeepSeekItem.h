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
