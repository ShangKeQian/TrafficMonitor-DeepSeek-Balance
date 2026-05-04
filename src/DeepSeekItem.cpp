#include "DeepSeekItem.h"
#include <cstdio>

CDeepSeekItem::CDeepSeekItem(CDeepSeekPlugin* owner)
    : m_owner(owner)
    , m_labelText(L"等待首次刷新...")
    , m_valueText(L"")
{
}

void CDeepSeekItem::UpdateDisplayText(double balance, double consumption, bool show_consumption)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    wchar_t buf[128];

    // 第一行：余额 → Label；第二行：消耗 → Value
    swprintf_s(buf, L"¥%.2f", balance);
    m_labelText = buf;

    if (show_consumption && consumption > 0.01) {
        swprintf_s(buf, L"(-¥%.2f)", consumption);
        m_valueText = buf;
    } else {
        m_valueText = L"";
    }
}

void CDeepSeekItem::SetStatusText(const wchar_t* text)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_valueText = text;
}

void CDeepSeekItem::SetTooltipText(const std::wstring& text)
{
    std::lock_guard<std::mutex> lock(m_mutex);
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
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_labelText.c_str();
}

const wchar_t* CDeepSeekItem::GetItemValueText() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_valueText.c_str();
}

const wchar_t* CDeepSeekItem::GetItemValueSampleText() const
{
    return L"¥888.88";
}

int CDeepSeekItem::OnMouseEvent(MouseEventType type, int /*x*/, int /*y*/, void* /*hWnd*/, int /*flag*/)
{
    if (type == MT_LCLICKED) {
        m_requestRefresh = true;
        return 1;
    }
    return 0;
}
