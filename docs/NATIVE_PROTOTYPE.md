# Native prototype status

Pinned to Recompia v0.17.0 and Pokopia 2.0.1 build ID `76a489f3bd3642fa5e18bd83d32c98ba`.

## Current controls and selection flow

- **F6** toggles Smart Digging on/off.
- While enabled and while Pokopia's native pointer/mouse mode is active, **right mouse click** selects point A, then point B.
- The second click immediately validates the cuboid and reports its volume. There is no separate confirm or reset key.
- Turning Smart Digging off with F6 clears the selection. Turning it on again starts from an empty selection.
- After a completed selection, the next right click starts a fresh selection at point A.
- Depth remains configurable in the mod settings (default 5).
- The selection click is blocked from reaching the game's ordinary action input while Smart Digging is enabled in pointer mode, so selecting a corner does not also trigger the native tool action.

## Implementation details

- Loads as a native Modtopia code mod and subscribes to the game-thread `FrameStart` event.
- Checks the exact game version and build ID before accepting startup.
- Reads the local player's pointer-mode state and hit cell from the mapped game layout.
- Uses Modtopia's `pad_read` to detect the right mouse click mapped by Recompia's Joy-Con 2 mouse input path to `MT_PAD_R`; uses `pad_block` to keep the selection click from activating the game's tool.
- Calls the mapped block-ID getter read-only to reject empty cells.
- Uses the tested two-point cuboid geometry, configurable depth, and 4096-cell safety cap.
- Computes and validates the full volume immediately on the second click.

## Important limitations

- Selection requires Pokopia/Recompia's native pointer/mouse mode to be active. The mod does not currently activate that mode itself.
- The current implementation validates and reports the volume; it does **not** break blocks.
- A safe arbitrary block-break function has not yet been verified. Recompia's `0x137e20` set-block routine is not used. The story dig-spot function `0x980ca0` is not a general block-break API.
- No 3D cuboid preview is drawn yet. The inspected third-party Modtopia API has no general world-overlay drawing callback.
- The manifest is build-pinned. Other game builds are refused instead of risking invalid memory access.

## Configuration

Only F6 and excavation depth remain configurable in Modtopia's in-game mod settings. Point selection is intentionally mouse-driven to avoid multiple shortcut keys.

The GitHub Actions workflow builds `bin/linux-x64/mod.so` and packages a `.pkmod` artifact against the SDK bundled in Recompia v0.17.0.
