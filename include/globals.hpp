#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <map>
#include <nexus/Nexus.h>
#include <session.hpp>
#include <unordered_map>

// handle to self hmodule
extern HMODULE self_module;
// addon definition
extern AddonDefinition addon_def;
// addon api
extern AddonAPI *api;

extern char addon_name[];

extern HWND game_handle;

extern Session current_session;

typedef struct
{
    std::string name;
    int id;
    std::string icon;
    bool show;
} Currency;
extern std::map<int, Currency> currencies_list;
extern std::unordered_map<int, int> currencies_start;
extern std::unordered_map<int, int> currencies;

extern std::chrono::time_point<std::chrono::system_clock> last_session_check;


#endif // GLOBALS_HPP
