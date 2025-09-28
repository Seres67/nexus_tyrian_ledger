#ifndef NEXUS_TYRIAN_LEDGER_WIN32_HTTP_HPP
#define NEXUS_TYRIAN_LEDGER_WIN32_HTTP_HPP

#include <string>
#include <tuple>
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
    DWORD status_code = 0;
    std::string result;

    const HINTERNET internet = InternetOpen("Seres/GW2/TyrianLedger", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!internet)
        return {status_code, result};

    const HINTERNET connect =
        InternetConnect(internet, host.c_str(), use_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT,
                        nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!connect) {
        InternetCloseHandle(internet);
        return {status_code, result};
    }

    const char *accept_types[] = {"application/json", nullptr};
    DWORD flags = INTERNET_FLAG_RELOAD;
    if (use_https)
        flags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

    const HINTERNET request = HttpOpenRequest(connect, "GET", path.c_str(), nullptr, nullptr, accept_types, flags, 0);
    if (!request) {
        InternetCloseHandle(connect);
        InternetCloseHandle(internet);
        return {status_code, result};
    }

    if (HttpSendRequest(request, headers.empty() ? nullptr : headers.c_str(), -1L, nullptr, 0)) {
        DWORD length = sizeof(status_code);
        HttpQueryInfo(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status_code, &length, nullptr);

        std::vector<char> buffer(4096);
        DWORD bytes_read = 0;
        while (InternetReadFile(request, buffer.data(), static_cast<DWORD>(buffer.size()), &bytes_read) &&
               bytes_read > 0) {
            result.append(buffer.data(), bytes_read);
        }
    }

    InternetCloseHandle(request);
    InternetCloseHandle(connect);
    InternetCloseHandle(internet);
    return {status_code, result};
}

} // namespace win32_http

#endif // NEXUS_TYRIAN_LEDGER_WIN32_HTTP_HPP
