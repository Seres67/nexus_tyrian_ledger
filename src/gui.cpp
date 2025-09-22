#include <algorithm>
#include <globals.hpp>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <ranges>
#include <settings.hpp>

inline std::string format_with_commas(int64_t value)
{
    std::string num = std::to_string(value);

    int insert_position = static_cast<int>(num.length()) - 3;
    while (insert_position > 0) {
        num.insert(insert_position, ",");
        insert_position -= 3;
    }
    return num;
}

inline std::string format_gold(int value)
{
    std::string num = std::to_string(value);

    int insert_position = num.length();
    num.insert(insert_position, "c");
    insert_position -= 2;
    num.insert(insert_position, "s");
    insert_position -= 2;
    num.insert(insert_position, "g");
    return num;
}

void render_tracker()
{
    if (Settings::display_help && ImGui::CollapsingHeader("How it works##TyrianLedgerHelp")) {
        ImGui::TextWrapped("White number is the amount you had at the start of the session.");
        ImGui::TextWrapped(
            "The number next to it is the amount you have gained. Green is gained, red is lost, yellow is the same.");
        ImGui::TextWrapped(
            "If numbers don't update after 5 minutes, then the API is having issues and fixing it is out of my reach.");
        ImGui::TextWrapped("You can disable this help in the addons' settings.");
    }
    if (Settings::api_key.empty()) {
        ImGui::TextWrapped("You need to set an API key in the settings and unload/load the addon to use it.");
        return;
    }
    std::vector<Currency> sorted_currencies;
    for (const auto &val : currencies_list | std::views::values) {
        sorted_currencies.emplace_back(val);
    }
    std::ranges::sort(sorted_currencies, [](const Currency &a, const Currency &b) { return a.name < b.name; });
    for (const auto &[name, id, icon, show] : sorted_currencies) {
        if (!currencies.contains(id) || !currencies_start.contains(id) || !show)
            continue;
        std::string identifier = std::string("TYRIAN_LEDGER_ICON_").append(name);
        const auto texture = api->Textures.Get(identifier.c_str());
        if (texture != nullptr) {
            ImGui::Image(texture->Resource, ImVec2(16, 16));
            ImGui::SameLine();
            ImGui::Text(": %s", name.c_str());
        } else
            ImGui::Text("%s: ", name.c_str());
        ImGui::SameLine();
        if (name == "Coin")
            ImGui::Text("%s", format_gold(currencies[id]).c_str());
        else
            ImGui::Text("%s", format_with_commas(currencies[id]).c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", format_with_commas(currencies_start[id]).c_str());
        ImGui::SameLine();
        if (currencies[id] > currencies_start[id]) {
            if (name == "Coin")
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "+%s",
                                   format_gold(currencies[id] - currencies_start[id]).c_str());
            else
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "+%s",
                                   format_with_commas(currencies[id] - currencies_start[id]).c_str());
        } else if (currencies[id] < currencies_start[id]) {
            if (name == "Coin")
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "-%s",
                                   format_gold(currencies_start[id] - currencies[id]).c_str());
            else
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "-%s",
                                   format_with_commas(currencies_start[id] - currencies[id]).c_str());

        } else {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "0");
        }
    }
}

bool window_open = false;
void addon_render()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (Settings::lock_window)
        flags |= ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground;
    ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(Settings::window_alpha);
    if (ImGui::Begin("Tyrian Ledger##Tyrian LedgerMainWindow", &window_open, flags)) {
        check_session();
        std::chrono::time_point<std::chrono::system_clock> next;
        {
            std::lock_guard lock(session_mutex);
            next = last_session_check + std::chrono::minutes(5) + std::chrono::seconds(1);
        }
        const auto now = std::chrono::system_clock::now();
        auto countdown = std::chrono::duration_cast<std::chrono::seconds>(next - now);
        if (countdown.count() < 0)
            countdown = std::chrono::seconds(0);
        ImGui::Text("Next update in: %lld:%02lld", countdown.count() / 60, countdown.count() % 60);
        render_tracker();
        ImGui::End();
    }
}

#include <imgui/misc/cpp/imgui_stdlib.h>
void addon_options()
{
    if (ImGui::Checkbox("Display help##TyrianLedgerDisplayHelp", &Settings::display_help)) {
        Settings::json_settings[Settings::DISPLAY_HELP] = Settings::display_help;
        Settings::save(Settings::settings_path);
    }
    if (ImGui::InputText("API Key##TyrianLedgerAPIKey", &Settings::api_key, ImGuiInputTextFlags_Password)) {
        Settings::json_settings[Settings::API_KEY] = Settings::api_key;
        Settings::save(Settings::settings_path);
    }

    if (ImGui::Checkbox("Lock Window##TyrianLedgerLockWindow", &Settings::lock_window)) {
        Settings::json_settings[Settings::LOCK_WINDOW] = Settings::lock_window;
        Settings::save(Settings::settings_path);
    }
    if (ImGui::SliderFloat("Window Opacity##TyrianLedgerOpacity", &Settings::window_alpha, 0.f, 1.f)) {
        Settings::json_settings[Settings::WINDOW_ALPHA] = Settings::window_alpha;
        Settings::save(Settings::settings_path);
    }
    ImGui::TextColored({1.0, 1.0, 0, .933}, "Experimental");
    if (ImGui::Checkbox("Save sessions to CSV##TyrianLedgerSaveSessions", &Settings::save_sessions)) {
        Settings::json_settings[Settings::SAVE_SESSIONS] = Settings::save_sessions;
        Settings::save(Settings::settings_path);
    }
    ImGui::NewLine();
    ImGui::Text("Currencies");
    std::vector<Currency> sorted_currencies;
    for (const auto &val : currencies_list | std::views::values) {
        sorted_currencies.emplace_back(val);
    }
    std::ranges::sort(sorted_currencies, [](const Currency &a, const Currency &b) { return a.name < b.name; });
    for (auto &[name, id, icon, _show] : sorted_currencies) {
        if (ImGui::Checkbox(name.c_str(), &currencies_list[id].show)) {
            Settings::json_settings[std::string("TYRIAN_LEDGER_").append(name)] = currencies_list[id].show;
            Settings::save(Settings::settings_path);
        }
        if (const auto texture = api->Textures.Get(std::string("TYRIAN_LEDGER_ICON_").append(name).c_str());
            texture != nullptr) {
            ImGui::SameLine();
            ImGui::Image(texture->Resource, ImVec2(16, 16));
        }
    }
}
