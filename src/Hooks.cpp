#include "Hooks.h"

#include "HookUtil.hpp"
#include "PCH.h"
#include "SSCState.h"

namespace Hooks {
    namespace {
        struct PlayerNotifyAnimationGraphHook {
            using Fn = bool (*)(RE::IAnimationGraphManagerHolder*, const RE::BSFixedString&);
            static inline Fn _orig{nullptr};

            static bool thunk(RE::IAnimationGraphManagerHolder* a_this, const RE::BSFixedString& a_eventName) {
                if (a_this == static_cast<RE::IAnimationGraphManagerHolder*>(RE::PlayerCharacter::GetSingleton())) {
                    const char* s = a_eventName.c_str();
                    const std::string_view tag = s ? s : "";
                    if (tag != "" && tag != "Collision_RecoilStop" && tag != "moveStop" && tag != "moveStart" &&
                        tag != "TDM_Turn_180" && tag != "turnStop")
                        SSC_DEBUG_LOG("[GraphNotify] event={}", tag);
                    if (tag == "SneakStart" || tag == "SneakStop") {
                        if (!SSC::g_sneakOwnedByMod.load(std::memory_order_relaxed)) {
                            SSC_DEBUG_LOG("[GraphNotify] Not my mod sneak");
                            return false;
                        }
                    }
                    if (tag == "SprintStart" || tag == "SprintStop") {
                        if (!SSC::g_sprintOwnedByMod.load(std::memory_order_relaxed)) {
                            SSC_DEBUG_LOG("[GraphNotify] Not my mod sprint");
                            return false;
                        }
                    }
                }

                return _orig ? _orig(a_this, a_eventName) : false;
            }

            static void Install() {
                REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_PlayerCharacter[3]};
                const std::uintptr_t orig = vtbl.write_vfunc(1, thunk);
                _orig = reinterpret_cast<Fn>(orig);
            }
        };
    }

    void Install_Hooks() {
        PlayerNotifyAnimationGraphHook::Install();
    }
}