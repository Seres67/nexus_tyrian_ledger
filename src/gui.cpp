#include "../include/gui.hpp"
#include "../include/globals.hpp"

#include "../include/settings.hpp"
#include "../modules/imgui/imgui.h"
#include <algorithm>
#include <ranges>

void render_tracker()
{
    if (Settings::display_help && ImGui::CollapsingHeader("How it works##SessionTrackerHelp")) {
        ImGui::TextWrapped("White number is the amount you had at the start of the session.");
        ImGui::TextWrapped(
            "The number next to it is the amount you have gained. Green is gained, red is lost, yellow is the same.");
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
        std::string identifier = std::string("SESSION_ICON_").append(name);
        auto texture = api->Textures.Get(identifier.c_str());
        if (texture != nullptr) {
            ImGui::Image(texture->Resource, ImVec2(16, 16));
            ImGui::SameLine();
            ImGui::Text(": %s", name.c_str());
        } else
            ImGui::Text("%s: ", name.c_str());
        ImGui::SameLine();
        ImGui::Text("%d", currencies_start[id]);
        ImGui::SameLine();
        if (currencies[id] > currencies_start[id]) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "+%d", currencies[id] - currencies_start[id]);
        } else if (currencies[id] < currencies_start[id]) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "-%d", currencies_start[id] - currencies[id]);
        } else {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "0");
        }
    }
}
