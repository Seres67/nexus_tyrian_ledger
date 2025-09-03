#include <filesystem>
#include <fstream>
#include <globals.hpp>
#include <nexus/Nexus.h>
#include <nlohmann/json.hpp>
#include <settings.hpp>

using json = nlohmann::json;
namespace Settings
{
const char *SESSIONS_PATH = "SessionsPath";
const char *SAVE_SESSIONS = "SaveSessions";
const char *WINDOW_ALPHA = "WindowAlpha";
const char *API_KEY = "ApiKey";
const char *DISPLAY_HELP = "DisplayHelp";
const char *LOCK_WINDOW = "LockWindow";

json json_settings;
std::mutex mutex;
std::filesystem::path settings_path;

std::filesystem::path sessions_path;
bool save_sessions = false;
float window_alpha = 1.f;
std::string api_key;
bool display_help = true;
bool lock_window = false;

void load(const std::filesystem::path &path)
{
    json_settings = json::object();
    if (!std::filesystem::exists(path)) {
        return;
    }

    {
        std::lock_guard lock(mutex);
        try {
            if (std::ifstream file(path); file.is_open()) {
                json_settings = json::parse(file);
                file.close();
            }
        } catch (json::parse_error &ex) {
            api->Log(ELogLevel_WARNING, addon_name, "settings.json could not be parsed.");
            api->Log(ELogLevel_WARNING, addon_name, ex.what());
        }
    }
    if (!json_settings[SESSIONS_PATH].is_null()) {
        json_settings[SESSIONS_PATH].get_to(sessions_path);
    }
    if (!json_settings[SAVE_SESSIONS].is_null()) {
        json_settings[SAVE_SESSIONS].get_to(save_sessions);
    }
    if (!json_settings[WINDOW_ALPHA].is_null()) {
        json_settings[WINDOW_ALPHA].get_to(window_alpha);
    }
    if (!json_settings[API_KEY].is_null()) {
        json_settings[API_KEY].get_to(api_key);
    }
    if (!json_settings[DISPLAY_HELP].is_null()) {
        json_settings[DISPLAY_HELP].get_to(display_help);
    }
    if (!json_settings[LOCK_WINDOW].is_null()) {
        json_settings[LOCK_WINDOW].get_to(lock_window);
    }
    api->Log(ELogLevel_INFO, addon_name, "settings loaded!");
}

void save(const std::filesystem::path &path)
{
    if (json_settings.is_null()) {
        api->Log(ELogLevel_WARNING, addon_name, "settings.json is null, cannot save.");
        return;
    }
    if (!std::filesystem::exists(path.parent_path())) {
        std::filesystem::create_directories(path.parent_path());
    }
    {
        std::lock_guard lock(mutex);
        if (std::ofstream file(path); file.is_open()) {
            file << json_settings.dump(1, '\t') << std::endl;
            file.close();
        }
        api->Log(ELogLevel_INFO, addon_name, "settings saved!");
    }
}
} // namespace Settings
