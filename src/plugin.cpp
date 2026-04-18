#include "Hooks.h"
#include "InputListener.h"
#include "InputManagerAPI.h"
#include "PCH.h"

#ifndef DLLEXPORT
    #include "REL/Relocation.h"
#endif
#ifndef DLLEXPORT
    #define DLLEXPORT __declspec(dllexport)
#endif

int g_SprintActionID = -1;
int g_SneakActionID = -1;

namespace {
    void InitializeLogger() {
        if (auto path = SKSE::log::log_directory()) {
            *path /= "SprintSneakContextual.log";
            auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
            auto logger = std::make_shared<spdlog::logger>("global", sink);
            spdlog::set_default_logger(logger);
            spdlog::set_level(spdlog::level::info);
            spdlog::flush_on(spdlog::level::info);
            spdlog::info("[CSS] Logger inicializado.");
        }
    }

    int FindOrCreateAction(InputManagerAPI::IInputManager* api, const char* name) {
        const size_t count = api->GetInputCount(0);

        for (size_t i = 0; i < count; ++i) {
            const char* existing = api->GetInputName(0, static_cast<int>(i));
            if (existing && std::strcmp(existing, name) == 0) return static_cast<int>(i);
        }

        return api->CreateInput(0, name);
    }

    void SetupInputManagerAPI() {
        using namespace InputManagerAPI;
        auto* api = RequestAPIDirect();
        if (!api) {
            spdlog::error("[API] Failed to obtain InputManager API!");
            return;
        }
        spdlog::info("[API] InputManager API obtained successfully!");

        g_SneakActionID = FindOrCreateAction(api, "Sneak");
        api->UpdateListener(0, g_SneakActionID, "SprintSneakContextual", "Tap to Sneak", true);

        if (auto tapInfo = api->GetActionInfo(g_SneakActionID); !tapInfo.isValid) {
            tapInfo.id = g_SneakActionID;
            tapInfo.name = "Sneak";
            tapInfo.pcMainKey = 56;
            tapInfo.pcMainAction = 1;
            tapInfo.gamepadMainKey = 330;
            tapInfo.gamepadMainAction = 4;
            tapInfo.isValid = true;
            api->UpdateActionMapping(g_SneakActionID, tapInfo);
        }

        g_SprintActionID = FindOrCreateAction(api, "Sprint");
        api->UpdateListener(0, g_SprintActionID, "SprintSneakContextual", "Hold to Sprint", true);

        if (auto tapInfo = api->GetActionInfo(g_SprintActionID); !tapInfo.isValid) {
            tapInfo.id = g_SprintActionID;
            tapInfo.name = "Sprint";
            tapInfo.pcMainKey = 56;
            tapInfo.pcMainAction = 2;
            tapInfo.gamepadMainKey = 330;
            tapInfo.gamepadMainAction = 4;
            tapInfo.isValid = true;
            api->UpdateActionMapping(g_SprintActionID, tapInfo);
        }
        spdlog::info("[API] Actions configured.");
    }

    void GlobalMessageHandler(SKSE::MessagingInterface::Message* msg) {
        if (!msg) return;

        switch (msg->type) {
            case SKSE::MessagingInterface::kInputLoaded: {
                spdlog::info("[MSG] kInputLoaded");
                SetupInputManagerAPI();
                break;
            }
            case SKSE::MessagingInterface::kDataLoaded: {
                spdlog::info("[MSG] kDataLoaded");
                Hooks::Install_Hooks();
                spdlog::info("[MSG] Hooks installed.");
                InputListener::InputListener::Register();
                spdlog::info("[MSG] Listener registered.");
                break;
            }
            default:
                break;
        }
    }

}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
    InitializeLogger();
    if (const auto mi = SKSE::GetMessagingInterface()) {
        mi->RegisterListener(GlobalMessageHandler);
    }

    return true;
}