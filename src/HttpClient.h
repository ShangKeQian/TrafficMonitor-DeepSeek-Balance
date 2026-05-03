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
