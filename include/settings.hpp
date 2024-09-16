#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <mutex>
#include <nlohmann/json.hpp>

namespace Settings
{

void load(const std::filesystem::path &path);
void save(const std::filesystem::path &path);

extern nlohmann::json json_settings;
extern std::filesystem::path settings_path;
extern std::mutex mutex;

extern std::filesystem::path sessions_path;
extern bool save_sessions;
extern float window_alpha;
extern std::string api_key;
extern bool display_help;
extern bool lock_window;

extern const char *SESSIONS_PATH;
extern const char *SAVE_SESSIONS;
extern const char *WINDOW_ALPHA;
extern const char *API_KEY;
extern const char *DISPLAY_HELP;
extern const char *LOCK_WINDOW;
} // namespace Settings

#endif // SETTINGS_HPP
