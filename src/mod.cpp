#include "selection.h"
#include <modtopia/modtopia.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
constexpr uint64_t kPlayerManager = 0x3561928;
constexpr uint64_t kManagerPlayer = 0x30;
constexpr uint64_t kWorld = 0x34efc98;
constexpr uint64_t kBlockId = 0x17d7c;
constexpr uint64_t kMouseModeOn = 0xaa81;
constexpr uint64_t kMouseHitCell = 0xaa30;
constexpr uint64_t kMouseHitValid = 0xaa82;
constexpr uint64_t kMaxBlocks = 4096;

struct State {
    const MtHost* host = nullptr;
    MtMod* mod = nullptr;
    SdSelection selection{};
    bool enabled = false;
    bool prev_toggle = false;
    bool prev_right_click = false;
};

State g;

bool read_bytes(uint64_t address, void* out, size_t size) {
    return g.host && g.host->mem_read && address && g.host->mem_read(address, out, size) == MT_OK;
}

template <typename T> bool read_value(uint64_t address, T& out) {
    return read_bytes(address, &out, sizeof(out));
}

void notify(const char* message) {
    if (g.host && MT_HOST_HAS(g.host, notify) && g.host->notify)
        g.host->notify(g.mod, message, 3.5);
    mt_logf(g.host, g.mod, MT_LOG_INFO, "%s", message);
}

bool pressed(const char* setting, bool& previous) {
    const char* key = g.host->config_string(g.mod, setting);
    const bool down = key && *key && g.host->key_down(g.mod, key) != 0;
    const bool edge = down && !previous;
    previous = down;
    return edge;
}

bool current_target(SdCell& cell) {
    const uint64_t base = g.host->module_base("main");
    uint64_t manager = 0, player = 0;
    uint8_t mouse_on = 0, valid = 0;
    int16_t xyz[3] = {};
    if (!base || !read_value(base + kPlayerManager, manager) || !manager ||
        !read_value(manager + kManagerPlayer, player) || !player)
        return false;
    if (!read_value(player + kMouseModeOn, mouse_on) || mouse_on == 0 ||
        !read_value(player + kMouseHitValid, valid) || valid == 0 ||
        !read_bytes(player + kMouseHitCell, xyz, sizeof(xyz)))
        return false;
    if (xyz[0] < 0 || xyz[0] > 2047 || xyz[1] < 0 || xyz[1] > 127 ||
        xyz[2] < 0 || xyz[2] > 2047)
        return false;

    uint64_t world = 0;
    if (!read_value(base + kWorld, world) || !world) return false;
    MtCall call{};
    call.x[0] = world;
    call.x[1] = static_cast<uint64_t>(static_cast<int64_t>(xyz[0]));
    call.x[2] = static_cast<uint64_t>(static_cast<int64_t>(xyz[1]));
    call.x[3] = static_cast<uint64_t>(static_cast<int64_t>(xyz[2]));
    if (g.host->call_guest(base + kBlockId, &call) != MT_OK || call.x[0] == 0)
        return false;

    cell = {xyz[0], xyz[1], xyz[2]};
    return true;
}

bool mouse_mode_active() {
    const uint64_t base = g.host->module_base("main");
    uint64_t manager = 0, player = 0;
    uint8_t enabled = 0;
    return base && read_value(base + kPlayerManager, manager) && manager &&
           read_value(manager + kManagerPlayer, player) && player &&
           read_value(player + kMouseModeOn, enabled) && enabled != 0;
}

bool right_click_pressed() {
    if (!MT_HOST_HAS(g.host, pad_read) || !g.host->pad_read) return false;
    MtPad pad{};
    if (g.host->pad_read(g.mod, 0, &pad) != MT_OK) return false;
    const bool down = (pad.buttons & MT_PAD_R) != 0;
    const bool edge = down && !g.prev_right_click;
    g.prev_right_click = down;
    return edge;
}

void block_selection_click(bool block) {
    if (!MT_HOST_HAS(g.host, pad_block) || !g.host->pad_block) return;
    MtPad mask{};
    if (block) mask.buttons = MT_PAD_R;
    g.host->pad_block(g.mod, 0, block ? &mask : nullptr);
}

void report_selection() {
    if (!g.selection.has_point_a) {
        notify("Smart Digging: nessun punto selezionato.");
        return;
    }
    char message[256];
    if (!g.selection.has_point_b) {
        std::snprintf(message, sizeof(message), "Smart Digging: A = %d, %d, %d. Seleziona B.",
                      g.selection.point_a.x, g.selection.point_a.y, g.selection.point_a.z);
        notify(message);
        return;
    }
    SdRegion region{};
    if (!sd_selection_build_region(&g.selection, kMaxBlocks, &region)) {
        notify("Smart Digging: regione non valida o oltre 4096 blocchi.");
        return;
    }
    std::snprintf(message, sizeof(message),
                  "Smart Digging: %llu blocchi, profondita %u. A=(%d,%d,%d), B=(%d,%d,%d).",
                  static_cast<unsigned long long>(region.block_count), g.selection.depth,
                  g.selection.point_a.x, g.selection.point_a.y, g.selection.point_a.z,
                  g.selection.point_b.x, g.selection.point_b.y, g.selection.point_b.z);
    notify(message);
}

int on_frame(MtMod*, const MtEvent*, void* user) {
    auto* s = static_cast<State*>(user);
    if (!s || !s->host || !s->host->config_string || !s->host->key_down || !s->host->config_number) return MT_EVENT_CONTINUE;

    if (pressed("toggle_key", s->prev_toggle)) {
        s->enabled = !s->enabled;
        sd_selection_reset(&s->selection);
        s->prev_right_click = false;
        block_selection_click(false);
        notify(s->enabled
            ? "Smart Digging attivo: clic destro per selezionare il punto A e poi il punto B."
            : "Smart Digging disattivato: selezione azzerata.");
    }
    if (!s->enabled) return MT_EVENT_CONTINUE;

    if (s->selection.has_point_a) {
        const double configured_depth = s->host->config_number(s->mod, "depth");
        const auto depth = static_cast<uint32_t>(configured_depth);
        if (configured_depth >= 1.0 && configured_depth <= 256.0)
            sd_selection_set_depth(&s->selection, depth);
    }

    const bool mouse_mode = mouse_mode_active();
    block_selection_click(mouse_mode);
    if (!mouse_mode) {
        s->prev_right_click = false;
        return MT_EVENT_CONTINUE;
    }

    if (right_click_pressed()) {
        SdCell cell{};
        if (!current_target(cell)) {
            notify("Nessun blocco valido sotto il puntatore.");
        } else if (!s->selection.has_point_a || s->selection.has_point_b) {
            sd_selection_set_point_a(&s->selection, cell);
            s->selection.has_point_b = false;
            report_selection();
        } else if (!sd_selection_set_point_b(&s->selection, cell)) {
            notify("Punto B non valido.");
        } else {
            // Validate the complete volume immediately after the second click.
            report_selection();
        }
    }
    return MT_EVENT_CONTINUE;
}
} // namespace

MT_DECLARE_MOD()
MT_EXTERN_C MT_EXPORT int mod_init(const MtHost* host, MtMod* mod) {
    if (!host || !mod || host->api_version < MT_API_VERSION ||
        !host->game_version || std::strcmp(host->game_version, "2.0.1") != 0 ||
        !host->game_build_id ||
        std::strcmp(host->game_build_id, "76a489f3bd3642fa5e18bd83d32c98ba") != 0) {
        return MT_ERR_VERSION;
    }
    g.host = host;
    g.mod = mod;
    sd_selection_reset(&g.selection);
    if (!host->subscribe || host->subscribe(mod, "FrameStart", MT_PRIORITY_NORMAL, &on_frame, &g) != MT_OK)
        return MT_ERR;
    mt_logf(host, mod, MT_LOG_INFO,
            "Smart Digging prototype loaded (Modtopia API %u.%u, Recompia game %s).",
            host->api_version >> 16, host->api_version & 0xffff, host->game_version);
    if (MT_HOST_HAS(host, notify) && host->notify)
        host->notify(mod, "Smart Digging caricato. F6 attiva/disattiva.", 4.0);
    return MT_OK;
}
