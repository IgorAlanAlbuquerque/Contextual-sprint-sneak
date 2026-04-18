#include "Hooks.h"

#include "HookUtil.hpp"
#include "InputListener.h"
#include "PCH.h"
#include "SSCState.h"

namespace Hooks {
    namespace {
        struct PollInputDevicesHook {
            using Fn = void(RE::BSTEventSource<RE::InputEvent*>*, RE::InputEvent* const*);
            static inline std::uintptr_t func{0};

            static void thunk(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent* const* a_events) {
                InputListener::UpdateSSC();
                if (func == 0) return;
                reinterpret_cast<Fn*>(func)(a_dispatcher, a_events);
            }

            static void Install() {
                Hook::stl::write_call<PollInputDevicesHook>(REL::RelocationID(67315, 68617, 0xC519E0),
                                                            REL::VariantOffset(0x7B, 0x7B, 0x81));
            }
        };
        struct SneakHandlerCanProcessHook {
            using Fn = bool (*)(RE::SneakHandler*, RE::InputEvent*);
            static inline Fn _orig{nullptr};

            static bool thunk(RE::SneakHandler* a_this, RE::InputEvent* a_event) {
                if (std::string_view{a_event->QUserEvent().c_str()} == "Sneak") {
                    if (!SSC::g_sneakOwnedByMod.load(std::memory_order_relaxed)) {
                        SSC_DEBUG_LOG("[SneakHandler] Not my mod sneak");
                        return false;
                    } else {
                        SSC_DEBUG_LOG("[SneakHandler] Sneak owned");
                        SSC::EndOwnedSneakNow();
                    }
                }

                return _orig(a_this, a_event);
            }

            static void Install() {
                REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_SneakHandler[0]};
                const std::uintptr_t orig = vtbl.write_vfunc(1, thunk);
                _orig = reinterpret_cast<Fn>(orig);
            }
        };

        struct SprintHandlerCanProcessHook {
            using Fn = bool (*)(RE::SprintHandler*, RE::InputEvent*);
            static inline Fn _orig{nullptr};

            static bool thunk(RE::SprintHandler* a_this, RE::InputEvent* a_event) {
                if (std::string_view{a_event->QUserEvent().c_str()} == "Sprint") {
                    if (!SSC::g_sprintOwnedByMod.load(std::memory_order_relaxed)) {
                        SSC_DEBUG_LOG("[SprintHandler] Not my mod sprint");
                        return false;
                    }
                }

                return _orig(a_this, a_event);
            }

            static void Install() {
                REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_SprintHandler[0]};
                const std::uintptr_t orig = vtbl.write_vfunc(1, thunk);
                _orig = reinterpret_cast<Fn>(orig);
            }
        };
    }

    void Install_Hooks() {
        SneakHandlerCanProcessHook::Install();
        SprintHandlerCanProcessHook::Install();
    }
}