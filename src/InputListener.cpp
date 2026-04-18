#include "InputListener.h"

#include "PCH.h"
#include "SSCState.h"

extern int g_SprintActionID;
extern int g_SneakActionID;

namespace {
    float timeheld = 0.0f;
    bool g_sprintActive = false;

    float now() { return static_cast<float>(RE::GetDurationOfApplicationRunTime()) / 1000.0f; }

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
        auto* playerControls = RE::PlayerControls::GetSingleton();
        if (!playerControls) return;

        auto* sprintHandler = playerControls->sprintHandler;
        if (!sprintHandler) return;

        RE::ButtonEvent* fakeEvent = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kKeyboard, "Sprint", 0, 1.0f, 0.0f);
        if (!fakeEvent) return;

        auto* data = &playerControls->data;

        g_sprintActive = true;
        SSC::BeginOwnedSprint();
        if (sprintHandler->CanProcess(fakeEvent)) {
            sprintHandler->ProcessButton(fakeEvent, data);
        }
        timeheld = now();
        SSC_DEBUG_LOG("[SSC] Sprint started");
    }

    void StopSprint() {
        auto* playerControls = RE::PlayerControls::GetSingleton();
        if (!playerControls) return;

        auto* sprintHandler = playerControls->sprintHandler;
        if (!sprintHandler) return;

        float held = now() - timeheld;
        RE::ButtonEvent* fakeEvent = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kKeyboard, "Sprint", 0, 0.0f, held);
        if (!fakeEvent) return;

        auto* data = &playerControls->data;

        SSC::BeginOwnedSprint();
        if (sprintHandler->CanProcess(fakeEvent)) {
            sprintHandler->ProcessButton(fakeEvent, data);
        }
        SSC::EndOwnedSprintNow();
        g_sprintActive = false;
        timeheld = 0.0f;
        SSC_DEBUG_LOG("[SSC] Sprint stopped");
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

void InputListener::UpdateSSC() {
    if (!g_sprintActive) return;

    auto* playerControls = RE::PlayerControls::GetSingleton();
    if (!playerControls) return;

    auto* sprintHandler = playerControls->sprintHandler;
    if (!sprintHandler) return;

    float held = now() - timeheld;
    auto* fakeEvent = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kKeyboard, "Sprint", 0, 0.5f, held);
    if (!fakeEvent) return;

    if (sprintHandler->CanProcess(fakeEvent)) {
        sprintHandler->ProcessButton(fakeEvent, &playerControls->data);
    }
    SSC_DEBUG_LOG("[SSC] Sprint updated, held for {:.2f} seconds", held);
}