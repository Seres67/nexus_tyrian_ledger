#include <globals.hpp>
#include <mutex>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <textures.hpp>
#include <thread>
#include <win32-http.hpp>

void load_textures()
{
    std::thread(
        []()
        {
            auto [status, body] = win32_http::get("api.guildwars2.com", "/v2/currencies?ids=all", "");
            if (status == 200) {
                for (auto currencies_json = nlohmann::json::parse(body); const auto &currency : currencies_json) {
                    if (currency["name"].get<std::string>().empty())
                        continue;
                    if (!Settings::json_settings.contains(std::string("TYRIAN_LEDGER_").append(currency["name"])))
                        Settings::json_settings[std::string("TYRIAN_LEDGER_").append(currency["name"])] = false;
                    currencies_list[currency["id"]] = {
                        currency["name"], currency["id"], currency["icon"],
                        Settings::json_settings[std::string("TYRIAN_LEDGER_").append(currency["name"])]};
                    auto url = currency["icon"].get<std::string>();
                    const auto pos = url.find('/', url.find("//") + 2);
                    std::string remote = url.substr(0, pos);
                    std::string endpoint = url.substr(pos);
                    std::string identifier =
                        std::string("TYRIAN_LEDGER_ICON_").append(currency["name"].get<std::string>());
                    {
                        std::lock_guard l(api_mutex);
                        if (api->Textures.Get(identifier.c_str()) == nullptr)
                            api->Textures.LoadFromURL(identifier.c_str(), remote.c_str(), endpoint.c_str(), nullptr);
                    }
                }
            } else {
                {
                    std::lock_guard l(api_mutex);
                    api->Log(ELogLevel_WARNING, addon_name, body.c_str());
                }
            }
        })
        .detach();
}
