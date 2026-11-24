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
    bool prev_a = false;
    bool prev_b = false;
    bool prev_reset = false;
    bool prev_confirm = false;
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
    if (!s || !s->host || !s->host->config_string || !s->host->key_down) return MT_EVENT_CONTINUE;

    if (pressed("toggle_key", s->prev_toggle)) {
        s->enabled = !s->enabled;
        notify(s->enabled
            ? "Smart Digging attivo. Usa la modalita' mouse di Pokopia per puntare i blocchi."
            : "Smart Digging disattivato.");
    }
    if (!s->enabled) return MT_EVENT_CONTINUE;

    if (pressed("point_a_key", s->prev_a)) {
        SdCell cell{};
        if (!current_target(cell)) notify("Punto A non acquisito: attiva la modalita' mouse e punta un blocco valido.");
        else {
            sd_selection_set_point_a(&s->selection, cell);
            const auto depth = static_cast<uint32_t>(s->host->config_number(s->mod, "depth"));
            sd_selection_set_depth(&s->selection, depth >= 1 && depth <= 256 ? depth : 1);
            report_selection();
        }
    }
    if (pressed("point_b_key", s->prev_b)) {
        SdCell cell{};
        if (!current_target(cell)) notify("Punto B non acquisito: attiva la modalita' mouse e punta un blocco valido.");
        else if (!sd_selection_set_point_b(&s->selection, cell)) notify("Seleziona prima il punto A.");
        else report_selection();
    }
    if (pressed("reset_key", s->prev_reset)) {
        sd_selection_reset(&s->selection);
        notify("Smart Digging: selezione azzerata.");
    }
    if (pressed("confirm_key", s->prev_confirm)) {
        SdRegion region{};
        if (!sd_selection_build_region(&s->selection, kMaxBlocks, &region))
            notify("Smart Digging: selezione incompleta o non valida.");
        else
            notify("Selezione valida. Scavo disabilitato: la chiamata nativa di rottura non e' ancora verificata.");
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
