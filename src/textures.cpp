#include "httplib/httplib.h"
#include <globals.hpp>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <textures.hpp>

void load_textures()
{
    std::thread(
        []()
        {
            httplib::Client cli("https://api.guildwars2.com");
            const auto response = cli.Get("/v2/currencies?ids=all");
            if (response->status == 200) {
                for (auto currencies_json = nlohmann::json::parse(response->body);
                     const auto &currency : currencies_json) {
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
                    if (!api)
                        return;
                    if (api->Textures.Get(identifier.c_str()) == nullptr)
                        api->Textures.LoadFromURL(identifier.c_str(), remote.c_str(), endpoint.c_str(), nullptr);
                }
            } else {
                if (!api)
                    return;
                api->Log(ELogLevel_WARNING, addon_name, response->body.c_str());
            }
        })
        .detach();
}
