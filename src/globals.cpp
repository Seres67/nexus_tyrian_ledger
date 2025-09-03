#include <globals.hpp>

HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *api = nullptr;
char addon_name[] = "Tyrian Ledger";
HWND game_handle = nullptr;
Session current_session{};
std::map<int, Currency> currencies_list{};
std::unordered_map<int, int> currencies_start{};
std::unordered_map<int, int> currencies{};
std::mutex session_mutex;
std::chrono::time_point<std::chrono::system_clock> last_session_check;
