#ifndef NEXUS_TYRIAN_LEDGER_WIN32_HTTP_HPP
#define NEXUS_TYRIAN_LEDGER_WIN32_HTTP_HPP

#include <string>
#include <vector>
#include <windows.h>
#include <wininet.h>

namespace win32_http
{
/**
 * @param host host without protocol
 * @param path path to request
 * @param headers example: "User-Agent: MyCustomAgent/1.0\r\nAuthorization: Bearer TOKEN123\r\n"
 * @param use_https
 */
inline std::tuple<DWORD, std::string> get(const std::string &host, const std::string &path, const std::string &headers,
                                          const bool use_https = true)
{
    DWORD statusCode = 0;
    std::string result;

    const HINTERNET hInternet = InternetOpen("Seres/GW2/TyrianLedger", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet)
        return {statusCode, result};

    const HINTERNET hConnect =
        InternetConnect(hInternet, host.c_str(), use_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT,
                        nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return {statusCode, result};
    }

    const char *acceptTypes[] = {"application/json", nullptr};
    DWORD flags = INTERNET_FLAG_RELOAD;
    if (use_https)
        flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

    const HINTERNET hRequest = HttpOpenRequest(hConnect, "GET", path.c_str(), nullptr, nullptr, acceptTypes, flags, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return {statusCode, result};
    }

    if (HttpSendRequest(hRequest, headers.empty() ? nullptr : headers.c_str(), -1L, nullptr, 0)) {
        DWORD length = sizeof(statusCode);
        HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, nullptr);

        std::vector<char> buffer(4096);
        DWORD bytesRead = 0;
        while (InternetReadFile(hRequest, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead) &&
               bytesRead > 0) {
            result.append(buffer.data(), bytesRead);
        }
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return {statusCode, result};
}

} // namespace win32_http

#endif // NEXUS_TYRIAN_LEDGER_WIN32_HTTP_HPP
