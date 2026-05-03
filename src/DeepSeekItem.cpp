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
