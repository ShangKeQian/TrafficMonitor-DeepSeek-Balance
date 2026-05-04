#include "DeepSeekItem.h"
#include <Windows.h>
#include <cstdio>

CDeepSeekItem::CDeepSeekItem(CDeepSeekPlugin* owner)
    : m_owner(owner)
    , m_line1(L"等待首次刷新...")
{
}

void CDeepSeekItem::UpdateDisplayText(double balance, double consumption, bool show_consumption)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    wchar_t buf[128];

    // 第一行：余额
    swprintf_s(buf, L"¥%.2f", balance);
    m_line1 = buf;

    // 第二行：消耗
    if (show_consumption) {
        swprintf_s(buf, L"(¥%.2f)", consumption);
        m_line2 = buf;
    } else {
        m_line2 = L"";
    }
}

void CDeepSeekItem::SetStatusText(const wchar_t* text)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_line1 = text;
    m_line2 = L"";
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
    return L"";
}

const wchar_t* CDeepSeekItem::GetItemValueText() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_line1.c_str();
}

const wchar_t* CDeepSeekItem::GetItemValueSampleText() const
{
    return L"¥888.88";
}

bool CDeepSeekItem::IsCustomDraw() const
{
    return true;
}

int CDeepSeekItem::GetItemWidth() const
{
    return 56;
}

void CDeepSeekItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    HDC hdc = (HDC)hDC;
    RECT rc;

    // 加锁拷贝，释放锁后再绘制（避免死锁）
    std::wstring line1, line2;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        line1 = m_line1;
        line2 = m_line2;
    }

    COLORREF textColor = dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0);
    int oldBkMode = SetBkMode(hdc, TRANSPARENT);
    COLORREF oldTextColor = SetTextColor(hdc, textColor);

    // 第一行：余额
    rc = { x, y, x + w, y + h / 2 };
    DrawTextW(hdc, line1.c_str(), -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

    // 第二行：消耗
    if (!line2.empty()) {
        rc = { x, y + h / 2, x + w, y + h };
        DrawTextW(hdc, line2.c_str(), -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    SetTextColor(hdc, oldTextColor);
    SetBkMode(hdc, oldBkMode);
}

int CDeepSeekItem::OnMouseEvent(MouseEventType type, int /*x*/, int /*y*/, void* /*hWnd*/, int /*flag*/)
{
    if (type == MT_LCLICKED) {
        m_requestRefresh = true;
        return 1;
    }
    return 0;
}
