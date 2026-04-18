#include "InputListener.h"

#include "PCH.h"
#include "SSCState.h"

extern int g_SprintActionID;
extern int g_SneakActionID;

namespace {
    void PerformSneakToggle() {
        auto* playerControls = RE::PlayerControls::GetSingleton();
        if (!playerControls) return;

        auto* sneakHandler = playerControls->sneakHandler;
        if (!sneakHandler) return;

        RE::ButtonEvent* fakeEvent = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kKeyboard, "Sneak", 0, 1.0f, 0.0f);
        if (!fakeEvent) return;

        auto* data = &playerControls->data;

        SSC::BeginOwnedSneak();
        if (sneakHandler->CanProcess(fakeEvent)) {
            sneakHandler->ProcessButton(fakeEvent, data);
        }
        SSC::EndOwnedSneakNow();
    }

    void StartSprint() {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        SSC::BeginOwnedSprint();
        player->NotifyAnimationGraph("SprintStart");
        SSC::EndOwnedSprintNow();
    }

    void StopSprint() {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        SSC::BeginOwnedSprint();
        player->NotifyAnimationGraph("SprintStop");
        SSC::EndOwnedSprintNow();
    }
}

RE::BSEventNotifyControl InputListener::InputListener::ProcessEvent(const SKSE::ModCallbackEvent* a_event,
                                                                    RE::BSTEventSource<SKSE::ModCallbackEvent>*) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    std::string_view eventName = a_event->eventName.c_str();
    auto inputID = static_cast<int>(a_event->numArg);
    if (eventName == "InputManager_ActionTriggered") {
        if (inputID == g_SneakActionID) {
            SSC_DEBUG_LOG("[SSC] Tap → Sneak");
            PerformSneakToggle();
        }
        if (inputID == g_SprintActionID) {
            SSC_DEBUG_LOG("[SSC] Hold → Sprint");
            StartSprint();
        }
    } else if (eventName == "InputManager_ActionReleased" && inputID == g_SprintActionID) {
        SSC_DEBUG_LOG("[SSC] Release → Stop Sprint");
        StopSprint();
    }

    return RE::BSEventNotifyControl::kContinue;
}