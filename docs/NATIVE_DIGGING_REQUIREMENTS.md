# Native excavation behaviour — mandatory gameplay contract

This document records the user's explicit gameplay requirements. They override any implementation shortcut that directly edits world cells.

## Required behaviour

Smart Digging is an automation/area-selection layer around Pokopia's **native Rock Smash** power, not a faster or free block deletion tool.

### Selection and activation

1. Toggle Smart Digging with **F6**.
2. While the game's native pointer/mouse mode is active, right-click the block under the pointer to set point A, then right-click the second block to set point B.
3. The selected region is an axis-aligned cuboid. Point A defines its top Y plane; configurable depth extends downward from it.
4. The preview states are: **red** while defining the two points, **green** once the complete region is valid and waiting, and **blue** while excavation is active.
5. Once the region is complete, the player may stop aiming at the selected blocks. The preview remains independent of the current aim.
6. Rock Smash is the **activation gesture**, not a requirement to target a block. Pressing Rock Smash while the player's position is **inside the selected volume** starts excavation, even if the player aims into empty air.
7. The player must be inside the volume. Being adjacent to it, standing on its boundary from an outside cell, or aiming at it from outside must not activate excavation. Outside the volume, Rock Smash retains its ordinary game behaviour.
8. F6 off clears the selection; re-enabling starts a fresh selection. After a completed selection, the next right-click starts a new A/B selection.
9. The mod does not enable native pointer/mouse mode; the player must activate that mode in the game.

### Native block behaviour

For each eligible cell in the selected cuboid, Smart Digging must cause the game to perform the same native Rock Smash interaction that the player would perform manually, with the following consequences:

1. **Energy/hunger cost:** the native power's energy/hunger meter is charged exactly as in ordinary gameplay. No direct world edits, free breaks, meter bypass, or synthetic refund.
2. **Native eligibility and durability:** the game decides whether the currently unlocked power can affect each block. Preserve each block's native hardness, damage thresholds, animation duration, instant-break behaviour for fragile blocks, and the failure/bounce animation for blocks that are too hard.
3. **Progression scaling:** the mod must use the currently equipped/unlocked power level and its native capability. Later upgrades inherit their native capability automatically; no separate hard-coded material whitelist.
4. **Native drops:** every block's drops, quantities, random rolls, collection/overflow behaviour and associated world/save/quest effects must be exactly those produced by the base game.
5. **Complete native animation lifecycle:** each target must run through the original Rock Smash action state machine, not merely call a damage function or wait a guessed duration. Preserve the character's original wind-up, swing, impact, repeated-hit, block-break and recovery animations; native per-block timing; instant-break behaviour where the game uses it; and the original too-hard punch/bounce failure animation. The next target may begin only when the native action reports completion or reaches a verified terminal state.
6. **Native audiovisual and gameplay effects:** preserve the original impact/break particles, sound, visual effects, drops, inventory/overflow handling, quests and save effects. No synthetic replacement effects and no direct block deletion.
7. **Native timing and state:** execute on the game thread and respect native cooldown, energy checks, hit shape, durability and failure path. Do not enqueue a whole cuboid as instantaneous world writes.
6. **Four-block batch ceiling:** process batches of at most four blocks per native Rock Smash cycle. A final partial batch may contain one to three blocks. If native game logic handles fewer targets in a context, obey that lower limit.
7. **Safe cancellation:** stopping the operation must prevent further native actions. Already completed breaks remain exactly as they would in normal gameplay.

## Required architecture

- Treat the cuboid as a queue of candidate cells, not permission to destroy them.
- Gate activation using the character's actual in-world cell position, strictly inside the selected cuboid, not by distance or aim target.
- Detect Rock Smash activation independently of the currently aimed block; aiming into empty air is valid once the player is inside the selected volume.
- Partition candidate cells into deterministic batches of at most four. For each batch, ask the game's own native targeting/ability path to validate targets and invoke the same action entry path used by Rock Smash.
- Let the game itself decide whether the current ability can break a target. Too-hard blocks must remain unchanged and use the native bounce/failure behaviour.
- Advance only after the current native action has completed and its results are observable. Respect energy and cooldown.
- Never call a generic world set-block function to emulate breaking. Preserve multiplayer authority.

## Reverse-engineering findings and cautions

Recompia v0.17.0's `runtime/games/pkp/input/pkp_aim.cpp` documents that actions 881–885 are command-less swing tools (Rock Smash, Cut and their upgrades). The game constructs their preview and live hit shapes in separate native paths. The ordinary highlighted target is not a suitable activation dependency for Smart Digging: the user explicitly permits starting the operation while aiming into empty air.

The exact Rock Smash activation hook, player world-position layout, authoritative block-break call, action completion signal, energy debit and drop side effects still need to be established before invoking the native path.

The command target query at `main+0x2e19a0` is used by command-based abilities/tools. The generic world set-block routine noted at `main+0x137e20` must not be used as excavation.

## Acceptance tests before enabling excavation

- Activation succeeds while the player is strictly inside the volume, including when aiming into empty air.
- Activation fails when the player is adjacent to or outside the volume; normal Rock Smash remains unchanged outside.
- A low-level power cannot break a block above its normal strength limit; a later upgrade can only do so if the base game permits it.
- Fragile blocks, durable blocks and too-hard blocks retain their exact native timing, animations and outcomes.
- Energy/hunger decreases exactly as for equivalent native actions.
- Each block produces exactly the base game's drops and side effects.
- No-energy, cancellation, save reload and multiplayer behaviour match the base game.

Until the native activation hook, player-position layout, block-break ABI, completion signal, energy cost, target eligibility and drop path are verified, destructive excavation remains disabled.
