# Native prototype status

This is the first native Modtopia integration milestone, pinned to Recompia v0.17.0 and Pokopia 2.0.1 build ID `76a489f3bd3642fa5e18bd83d32c98ba`.

## What this prototype does

- Loads as a native Modtopia code mod and subscribes to the game-thread `FrameStart` event.
- Checks the exact game version and build ID before accepting startup.
- Uses configurable F6/F7/F8/F9/F10 key settings.
- Reads the local player object through the Recompia 0.17.0 player-manager layout.
- Reads the game's own mouse-mode hit cell (signed x/y/z) only while the game's pointer mode reports a valid hit.
- Calls the mapped game's block-ID getter read-only to reject empty cells.
- Uses the existing tested two-point cuboid geometry and configurable depth.
- Limits a proposed region to 4096 cells and reports its dimensions/count.

## Important limitations

- The current target-cell source is the game's Joy-Con 2 mouse-mode hit cell. The player must have that native pointer mode active. This prototype does not yet provide a camera-ray crosshair fallback.
- F10 is deliberately validation-only. It does **not** modify the world.
- A safe arbitrary block-break function has not yet been verified. Recompia's `0x137e20` set-block routine is identified in comments, but its full ABI, side effects and safe caller context are not established. The story dig-spot function `0x980ca0` is not a general block-break API and is not used.
- No 3D cuboid preview is drawn yet. The inspected third-party Modtopia API has no general world-overlay drawing callback.
- The manifest is build-pinned. Other game builds are refused instead of risking invalid memory access.

## Controls

| Key | Action |
|---|---|
| F6 | Enable/disable |
| F7 | Capture point A |
| F8 | Capture point B |
| F9 | Reset selection |
| F10 | Validate selection (no digging) |

Change keys and depth in Modtopia's in-game mod settings.

The GitHub Actions workflow builds `bin/linux-x64/mod.so` and packages a `.pkmod` artifact against the SDK bundled in Recompia v0.17.0.
