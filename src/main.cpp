#include "cpr/api.h"
#include "globals.hpp"
#include "session.hpp"

#include <fstream>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <nexus/Nexus.h>
#include <settings.hpp>
#include <textures.hpp>
#include <windows.h>

void addon_load(AddonAPI *api_p);
void addon_unload();
void addon_render();
void addon_options();

BOOL APIENTRY dll_main(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        self_module = hModule;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }
    return TRUE;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef()
{
    addon_def.Signature = -919784258;
    addon_def.APIVersion = NEXUS_API_VERSION;
    addon_def.Name = addon_name;
    addon_def.Version.Major = 0;
    addon_def.Version.Minor = 1;
    addon_def.Version.Build = 0;
    addon_def.Version.Revision = 1;
    addon_def.Author = "Seres67";
    addon_def.Description = "A Nexus addon to track your current game session.";
    addon_def.Load = addon_load;
    addon_def.Unload = addon_unload;
    addon_def.Flags = EAddonFlags_None;
    addon_def.Provider = EUpdateProvider_GitHub;
    addon_def.UpdateLink = "https://github.com/Seres67/nexus_tyrian_ledger";

    return &addon_def;
}

void addon_load(AddonAPI *api_p)
{
    api = api_p;

    ImGui::SetCurrentContext(static_cast<ImGuiContext *>(api->ImguiContext));
    ImGui::SetAllocatorFunctions(static_cast<void *(*)(size_t, void *)>(api->ImguiMalloc),
                                 static_cast<void (*)(void *, void *)>(api->ImguiFree)); // on imgui 1.80+
    api->Renderer.Register(ERenderType_Render, addon_render);
    api->Renderer.Register(ERenderType_OptionsRender, addon_options);

    Settings::settings_path = api->Paths.GetAddonDirectory("tyrian_ledger\\settings.json");
    Settings::sessions_path = api->Paths.GetAddonDirectory("tyrian_ledger\\sessions");
    if (std::filesystem::exists(Settings::settings_path)) {
        Settings::load(Settings::settings_path);
    } else {
        Settings::json_settings[Settings::SESSIONS_PATH] = Settings::sessions_path;
        Settings::save(Settings::settings_path);
        std::filesystem::create_directories(Settings::sessions_path);
    }
    load_textures();
    load_start_session();
    current_session.start_time = std::chrono::system_clock::now();
    api->Log(ELogLevel_INFO, addon_name, "addon loaded!");
}

void addon_unload()
{
    api->Log(ELogLevel_INFO, addon_name, "unloading addon...");
    save_session_sync();
    /*api->QuickAccess.Remove("QA_TYRIAN_LEDGER");*/
    api->Renderer.Deregister(addon_render);
    api->Renderer.Deregister(addon_options);
    api->Log(ELogLevel_INFO, addon_name, "addon unloaded!");
    api = nullptr;
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
        auto next = last_session_check + std::chrono::minutes(5);
        auto now = std::chrono::system_clock::now();
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(next - now);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(next - now - minutes);
        ImGui::Text("Next update in: %d:%lld", minutes.count(), seconds.count());
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
    if (ImGui::Checkbox("Save sessions to CSV! Experimental!##TyrianLedgerSaveSessions", &Settings::save_sessions)) {
        Settings::json_settings[Settings::SAVE_SESSIONS] = Settings::save_sessions;
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
    if (ImGui::InputText("API Key##TyrianLedgerAPIKey", &Settings::api_key, ImGuiInputTextFlags_Password)) {
        Settings::json_settings[Settings::API_KEY] = Settings::api_key;
        Settings::save(Settings::settings_path);
    }
    std::vector<Currency> sorted_currencies;
    for (const auto &val : currencies_list | std::views::values) {
        sorted_currencies.emplace_back(val);
    }
    std::ranges::sort(sorted_currencies,                                                     // NOLINT
                      [](const Currency &a, const Currency &b) { return a.name < b.name; }); // NOLINT
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
