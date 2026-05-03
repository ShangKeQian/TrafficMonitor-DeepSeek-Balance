#include "DeepSeekItem.h"
#include <cstdio>

CDeepSeekItem::CDeepSeekItem(CDeepSeekPlugin* owner)
    : m_owner(owner)
    , m_labelText(L"")
    , m_valueText(L"等待首次刷新...")
{
}

void CDeepSeekItem::UpdateDisplayText(const std::wstring& label, double balance, double consumption, bool show_consumption)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    wchar_t buf[128];

    if (label.empty()) {
        // 无标签：余额作为第一行，消耗作为第二行
        if (show_consumption && consumption > 0.01) {
            swprintf_s(buf, L"¥%.2f", balance);
            m_labelText = buf;
            swprintf_s(buf, L"(-¥%.2f)", consumption);
            m_valueText = buf;
        } else {
            swprintf_s(buf, L"¥%.2f", balance);
            m_labelText = buf;
            m_valueText = L"";
        }
    } else {
        // 有标签：标签第一行，余额和消耗作为后续行
        m_labelText = label;
        if (show_consumption && consumption > 0.01) {
            swprintf_s(buf, L"¥%.2f\n(-¥%.2f)", balance, consumption);
        } else {
            swprintf_s(buf, L"¥%.2f", balance);
        }
        m_valueText = buf;
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
