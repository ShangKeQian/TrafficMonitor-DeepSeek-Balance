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
