# Native excavation behaviour — mandatory gameplay contract

This document records the user's explicit gameplay requirements. They override any implementation shortcut that directly edits world cells.

## Required behaviour

Smart Digging is an automation/area-selection layer around Pokopia's **native Rock Smash power**, not a faster or free block deletion tool.

For each eligible cell in the selected cuboid, Smart Digging must cause the game to perform the same native **Rock Smash** action that the player would perform manually, with the following consequences:

1. **Energy/hunger cost:** the native power's energy/hunger meter is charged exactly as in ordinary gameplay. No direct world edits, free breaks, meter bypass, or synthetic refund.
2. **Native eligibility:** a cell can only be affected if the currently unlocked excavation power can break that block under the current game progression. The mod must not bypass material restrictions, upgrade tiers, quest gates, or tool requirements.
3. **Progression scaling:** the mod must use the currently equipped/unlocked power level and its native capability. If later upgrades allow harder blocks, Smart Digging inherits that capability automatically; it must not maintain a separate hard-coded material whitelist that can become stale.
4. **Native drops:** every block's drops, quantities, random rolls, collection/overflow behaviour and any associated world/save/quest effects must be exactly those produced by the base game.
5. **Native timing and state:** execute on the game thread and respect the native action's animation, cooldown, energy checks, hit shape, and failure path. Do not enqueue a whole cuboid as instantaneous world writes.
6. **Safe cancellation:** stopping the operation must prevent further native actions. Already completed breaks remain exactly as they would in normal gameplay.

## Required architecture

- Treat the cuboid as a queue of candidate cells, not as permission to destroy them.
- For each candidate, ask the game's own native targeting/ability path to validate the target and invoke the same action entry path used by the player's excavation power.
- Let the game itself decide whether the current ability can break the target. A rejected/too-hard target must remain unchanged and must not produce drops.
- Advance only after the native action has completed and its result is observable. Respect the normal energy meter and action cooldown.
- Never call a generic world set-block function to emulate breaking. It risks bypassing energy, tool progression, drops, quest hooks and save bookkeeping.
- Preserve multiplayer authority rules; do not locally fake host-side changes.

## Reverse-engineering findings and cautions

Recompia v0.17.0's `runtime/games/pkp/input/pkp_aim.cpp` documents that actions 881–885 are command-less swing tools (Rock Smash, Cut and their upgrades). The game constructs their preview and live hit shapes in separate native paths; Recompia's camera-aim hooks alter the heading while leaving the game's shape, validity, breaking and placing logic intact.

The user has clarified that Rock Smash is exactly the power intended. Implementation must trace and reuse the native Rock Smash execution path, including its upgraded variants, rather than a generic block-set function. The exact entry point, state prerequisites, completion signal, energy debit and drop side effects still need to be established before invoking it from a third-party mod.

The game command target query at `main+0x2e19a0` is used by command-based abilities/tools. The generic world set-block routine noted at `main+0x137e20` must not be used as the excavation implementation.

## Acceptance tests before enabling excavation

- With a low-level power, a block above its normal strength limit remains intact.
- With a later upgrade, the same block becomes breakable if and only if the base game allows it.
- The energy/hunger meter decreases by the same amount as the equivalent manual action.
- Each block produces exactly the base game's drops and side effects.
- A no-energy condition behaves like the base game.
- Cancellation stops future cells without undoing completed native actions.
- Repeated operation cannot duplicate drops or skip native cooldowns.
- A saved game reload and multiplayer session preserve authoritative world state.

Until the native action entry point, action completion signal, energy cost, target eligibility and drop path are verified, destructive excavation remains disabled.
