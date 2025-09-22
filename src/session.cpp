
#include <win32-http.hpp>

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
            auto [status, body] = win32_http::get("api.guildwars2.com", "/v2/account?v=latest",
                                                  std::format("Authorization: Bearer {}", Settings::api_key));
            if (status == 200) {
                auto account_json = nlohmann::json::parse(body);
                const std::string last_modified_str = account_json["last_modified"].get<std::string>();

                if (!api)
                    return;
                api->Log(ELogLevel_DEBUG, addon_name, last_modified_str.c_str());

                std::istringstream in(last_modified_str);
                std::chrono::sys_time<std::chrono::seconds> last_modified_tp;
                in >> std::chrono::parse("%FT%TZ", last_modified_tp);

                if (!in) {
                    if (!api)
                        return;
                    api->Log(ELogLevel_DEBUG, addon_name, "Failed to parse last_modified");
                } else {
                    std::lock_guard lock(session_mutex);
                    last_session_check = last_modified_tp;
                }
            }
            auto [status2, body2] = win32_http::get("api.guildwars2.com", "/v2/account/wallet?v=latest",
                                                    std::format("Authorization: Bearer {}", Settings::api_key));
            if (status2 == 200) {
                auto wallet_json = nlohmann::json::parse(body2);
                {
                    std::lock_guard lock(session_mutex);
                    currencies_start.clear();
                    for (const auto &curr : wallet_json) {
                        currencies_start[curr["id"]] = curr["value"];
                    }
                    current_session.start_time = std::chrono::system_clock::now();
                }
            } else {
                if (!api)
                    return;
                api->Log(ELogLevel_WARNING, addon_name, status2 ? body2.c_str() : "No response from wallet API");
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
            auto [status, body] = win32_http::get("api.guildwars2.com", "/v2/account/wallet",
                                                  std::format("Authorization: Bearer {}", Settings::api_key));

            if (status == 200) {
                for (auto wallet = nlohmann::json::parse(body); const auto &curr : wallet) {
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
                api->Log(ELogLevel_WARNING, addon_name, body.c_str());
            }
        })
        .detach();
}

void check_session()
{
    const auto current_time = std::chrono::system_clock::now();
    const auto next_update = last_session_check + std::chrono::minutes(5);
    if (current_time >= next_update) {
        api->Log(ELogLevel_INFO, addon_name, "Checking session");
        {
            std::lock_guard lock(session_mutex);
            last_session_check = next_update;
        }
        pull_session();
    }
}
