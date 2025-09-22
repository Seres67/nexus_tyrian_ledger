
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib/httplib.h>

#include <filesystem>
#include <fstream>
#include <globals.hpp>
#include <session.hpp>
#include <settings.hpp>

void load_start_session()
{
    std::thread(
        []()
        {
            httplib::Headers headers = {{"Authorization", std::format("Bearer {}", Settings::api_key)}};
            httplib::Client cli("https://api.guildwars2.com");
            const auto last_modified_check = cli.Get("/v2/account?v=latest", headers);
            if (last_modified_check->status == 200) {
                const auto last_modified = last_modified_check->get_header_value("Last-Modified");
                if (!api)
                    return;
                std::istringstream in{last_modified};
                std::chrono::sys_seconds tp;
                std::chrono::from_stream(in, "%a, %d %b %Y %H:%M:%S GMT", tp);
                const std::string ts = std::format("{:%Y-%m-%d %H:%M:%S}", tp);
                if (!api)
                    return;
                if (!in) {
                    if (!api)
                        return;
                    api->Log(ELogLevel_DEBUG, addon_name, "could not parse last_modified");
                }
                {
                    std::lock_guard<std::mutex> lock(session_mutex);
                    last_session_check = tp;
                }
            }
            const auto response2 = cli.Get("/v2/account/wallet", headers);
            if (response2->status == 200) {
                for (auto wallet = nlohmann::json::parse(response2->body); const auto &curr : wallet) {
                    currencies_start[curr["id"]] = curr["value"];
                }
                current_session.start_time = std::chrono::system_clock::now();

            } else {
                if (!api)
                    return;
                api->Log(ELogLevel_WARNING, addon_name, response2->body.c_str());
            }
        })
        .detach();
}

void save_session()
{
    if (!Settings::save_sessions)
        return;
    std::thread(
        []()
        {
            auto current_time = std::chrono::system_clock::now();
            current_session.end_time = current_time;
            const std::chrono::time_point now =
                std::chrono::zoned_time{std::chrono::current_zone(), current_time}.get_local_time();
            auto dp = std::chrono::floor<std::chrono::days>(now);
            const std::chrono::year_month_day ymd{dp};
            const std::chrono::hh_mm_ss hms{std::chrono::floor<std::chrono::milliseconds>(now - dp)};
            std::string date_end = std::to_string(static_cast<int>(ymd.year())) + "-" +
                                   std::to_string(static_cast<unsigned int>(ymd.month())) + "-" +
                                   std::to_string(static_cast<unsigned int>(ymd.day())) + "_" +
                                   std::to_string(hms.hours().count()) + "-" + std::to_string(hms.minutes().count()) +
                                   "-" + std::to_string(hms.seconds().count()) + "-" +
                                   std::to_string(hms.subseconds().count());
            const std::chrono::time_point now_start =
                std::chrono::zoned_time{std::chrono::current_zone(), current_session.start_time}.get_local_time();
            auto dp_start = std::chrono::floor<std::chrono::days>(now_start);
            const std::chrono::year_month_day ymd_start{dp_start};
            const std::chrono::hh_mm_ss hms_start{std::chrono::floor<std::chrono::milliseconds>(now_start - dp_start)};
            std::string date_start =
                std::to_string(static_cast<int>(ymd_start.year())) + "-" +
                std::to_string(static_cast<unsigned int>(ymd_start.month())) + "-" +
                std::to_string(static_cast<unsigned int>(ymd_start.day())) + "_" +
                std::to_string(hms_start.hours().count()) + "-" + std::to_string(hms_start.minutes().count()) + "-" +
                std::to_string(hms_start.seconds().count()) + "-" + std::to_string(hms_start.subseconds().count());
            std::ofstream output_file(Settings::sessions_path.string() + "\\" + date_start + "__" + date_end +
                                      ".session");
            api->Log(ELogLevel_INFO, addon_name, "Opening file...");
            output_file << current_session.start_time.time_since_epoch().count() << '\n';
            output_file << current_session.end_time.time_since_epoch().count() << '\n';
            output_file << "Name,Value\n";
            api->Log(ELogLevel_INFO, addon_name, "Saving currrency data...");
            for (const auto &[id, value] : currencies) {
                output_file << currencies_list[id].name << "," << value << '\n';
            }
            api->Log(ELogLevel_INFO, addon_name, "Closing file...");
            output_file.close();
            api->Log(ELogLevel_INFO, addon_name, "Session saved!");
        })
        .detach();
}

void save_session_sync()
{
    if (!Settings::save_sessions)
        return;

    auto current_time = std::chrono::system_clock::now();
    current_session.end_time = current_time;
    const std::chrono::time_point now =
        std::chrono::zoned_time{std::chrono::current_zone(), current_time}.get_local_time();
    auto dp = std::chrono::floor<std::chrono::days>(now);
    const std::chrono::year_month_day ymd{dp};
    const std::chrono::hh_mm_ss hms{std::chrono::floor<std::chrono::milliseconds>(now - dp)};
    std::string date_end = std::to_string(static_cast<int>(ymd.year())) + "-" +
                           std::to_string(static_cast<unsigned int>(ymd.month())) + "-" +
                           std::to_string(static_cast<unsigned int>(ymd.day())) + "_" +
                           std::to_string(hms.hours().count()) + "-" + std::to_string(hms.minutes().count()) + "-" +
                           std::to_string(hms.seconds().count()) + "-" + std::to_string(hms.subseconds().count());
    const std::chrono::time_point now_start =
        std::chrono::zoned_time{std::chrono::current_zone(), current_session.start_time}.get_local_time();
    auto dp_start = std::chrono::floor<std::chrono::days>(now_start);
    const std::chrono::year_month_day ymd_start{dp_start};
    const std::chrono::hh_mm_ss hms_start{std::chrono::floor<std::chrono::milliseconds>(now_start - dp_start)};
    std::string date_start =
        std::to_string(static_cast<int>(ymd_start.year())) + "-" +
        std::to_string(static_cast<unsigned int>(ymd_start.month())) + "-" +
        std::to_string(static_cast<unsigned int>(ymd_start.day())) + "_" + std::to_string(hms_start.hours().count()) +
        "-" + std::to_string(hms_start.minutes().count()) + "-" + std::to_string(hms_start.seconds().count()) + "-" +
        std::to_string(hms_start.subseconds().count());
    std::ofstream output_file(Settings::sessions_path.string() + "\\" + date_start + "__" + date_end + ".session");
    api->Log(ELogLevel_INFO, addon_name, "Opening file...");
    output_file << current_session.start_time.time_since_epoch().count() << '\n';
    output_file << current_session.end_time.time_since_epoch().count() << '\n';
    output_file << "Name,Value,Difference\n";
    api->Log(ELogLevel_INFO, addon_name, "Saving currrency data...");
    for (const auto &[id, value] : currencies) {
        output_file << currencies_list[id].name << "," << value << "," << currencies_start[id] << '\n';
    }
    api->Log(ELogLevel_INFO, addon_name, "Closing file...");
    output_file.close();
    api->Log(ELogLevel_INFO, addon_name, "Session saved!");
}

void pull_session()
{
    std::thread(
        []()
        {
            httplib::Client cli("https://api.guildwars2.com");
            httplib::Headers headers = {{"Authorization", std::format("Bearer {}", Settings::api_key)}};
            const auto response = cli.Get("/v2/account/wallet", headers);
            if (response->status == 200) {
                for (auto wallet = nlohmann::json::parse(response->body); const auto &curr : wallet) {
                    currencies[curr["id"]] = curr["value"];
                }
                if (!api)
                    return;
                api->Log(ELogLevel_INFO, addon_name, "Session pulled!");
            } else {
                if (!api)
                    return;
                api->Log(ELogLevel_WARNING, addon_name, "Failed to pull session");
                if (!api)
                    return;
                api->Log(ELogLevel_WARNING, addon_name, response->body.c_str());
            }
        })
        .detach();
}

void check_session()
{
    const auto current_time = std::chrono::system_clock::now();
    if (current_time - last_session_check > std::chrono::minutes(5) + std::chrono::seconds(1)) {
        api->Log(ELogLevel_INFO, addon_name, "Checking session");
        {
            std::lock_guard<std::mutex> lock(session_mutex);
            last_session_check = current_time;
        }
        pull_session();
    }
}
