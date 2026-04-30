#pragma once
#include <atomic>

namespace SSC
{
    inline std::atomic_bool g_sneakOwnedByMod{false};
    inline std::atomic_bool g_sprintOwnedByMod{false};
    inline std::atomic_bool g_suppressSneakInput{false};

    inline void BeginOwnedSneak()
    {
        g_sneakOwnedByMod.store(true, std::memory_order_relaxed);
    }

    inline void BeginOwnedSprint(){
        g_sprintOwnedByMod.store(true, std::memory_order_relaxed);
        g_suppressSneakInput.store(true, std::memory_order_relaxed);
    }

    inline void EndOwnedSneakNow()
    {
        g_sneakOwnedByMod.store(false, std::memory_order_relaxed);
    }

    inline void EndOwnedSprintNow(){
        spdlog::info("[SSC] Ending owned sprint");
        g_sprintOwnedByMod.store(false, std::memory_order_relaxed);
        g_suppressSneakInput.store(false, std::memory_order_relaxed);
    }
}