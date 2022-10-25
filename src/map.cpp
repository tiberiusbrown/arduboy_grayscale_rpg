#include "common.hpp"

#include "generated/fxdata.h"
#include "script_commands.hpp"
#include "tile_solid.hpp"

static bool run_chunk()
{
    auto& c = active_chunks[running_chunk].chunk;
    uint16_t tx = uint8_t(loaded_cx + (running_chunk & 1)) * 8;
    uint16_t ty = uint8_t(loaded_cy + (running_chunk >> 1)) * 4;
    uint8_t walk_tile = 255;
    {
        // TODO: could these be uint8_t
        uint16_t dx = uint16_t((px >> 4) - tx);
        uint16_t dy = uint16_t((py >> 4) - ty);
        if(dx < 8 && dy < 4) walk_tile = dy * 8 + dx;
    }
    uint8_t sel_tile = 255;
    {
        // TODO: could these be uint8_t
        uint16_t dx = uint16_t(selx - tx);
        uint16_t dy = uint16_t(sely - ty);
        if(dx < 8 && dy < 4) sel_tile = dy * 8 + dx;
    }
    while(chunk_instr < CHUNK_SCRIPT_SIZE) {
        uint8_t instr = c.script[chunk_instr++];
        switch(instr) {
        case CMD_END:
            return false;

            // message/dialog
        case CMD_MSG:
        case CMD_DLG:
        case CMD_TMSG:
        case CMD_TDLG: {
            uint8_t t0, t1;
            if(instr != CMD_MSG) t0 = c.script[chunk_instr++];
            if(instr == CMD_TDLG) t1 = c.script[chunk_instr++];
            uint16_t stri = c.script[chunk_instr++];
            stri |= uint16_t(c.script[chunk_instr++]) << 8;
            if((instr == CMD_TMSG || instr == CMD_TDLG) && t0 != sel_tile)
                break;
            change_state(STATE_DIALOG);
            sdata.dialog.portrait = 255;
            if(instr == CMD_DLG) sdata.dialog.portrait = t0;
            if(instr == CMD_TDLG) sdata.dialog.portrait = t1;
            platform_fx_read_data_bytes(STRINGDATA + stri, sdata.dialog.message,
                                        sizeof(sdata.dialog.message));
            wrap_text(sdata.dialog.message, 128);
            return true;
        }

            // teleport
        case CMD_TP:
        case CMD_TTP:
        case CMD_WTP: {
            uint8_t t;
            if(instr != CMD_TP) t = c.script[chunk_instr++];
            uint16_t tx, ty;
            tx = c.script[chunk_instr++];
            tx |= uint16_t(c.script[chunk_instr++]) << 8;
            ty = c.script[chunk_instr++];
            ty |= uint16_t(c.script[chunk_instr++]) << 8;
            if(instr == CMD_TTP && t != sel_tile) break;
            if(instr == CMD_WTP && t != walk_tile) break;
            change_state(STATE_TP);
            sdata.tp.tx = tx;
            sdata.tp.ty = ty;
            return true;
        }

        case CMD_ADD: {
            uint8_t t = c.script[chunk_instr++];
            uint8_t dst = t & 0xf;
            uint8_t src = t >> 4;
            chunk_regs[dst] += chunk_regs[src];
            break;
        }
        case CMD_ADDI: {
            uint8_t t = c.script[chunk_instr++];
            int8_t imm = (int8_t)c.script[chunk_instr++];
            uint8_t dst = t & 0xf;
            uint8_t src = t >> 4;
            chunk_regs[dst] = chunk_regs[src] + imm;
            break;
        }
        case CMD_SUB: {
            uint8_t t = c.script[chunk_instr++];
            uint8_t dst = t & 0xf;
            uint8_t src = t >> 4;
            chunk_regs[dst] -= chunk_regs[src];
            break;
        }
        case CMD_FS: {
            uint16_t f = c.script[chunk_instr++];
            f |= (uint16_t(c.script[chunk_instr++]) << 8);
            story_flag_set(f);
            break;
        }

        case CMD_JMP: chunk_instr = c.script[chunk_instr]; break;
        case CMD_BRZ: {
            uint8_t t = c.script[chunk_instr++];
            uint8_t i = c.script[chunk_instr++];
            if(chunk_regs[t] == 0) chunk_instr = i;
            break;
        }
        case CMD_BRN: {
            uint8_t t = c.script[chunk_instr++];
            uint8_t i = c.script[chunk_instr++];
            if(chunk_regs[t] < 0) chunk_instr = i;
            break;
        }
        case CMD_BRFS:
        case CMD_BRFC: {
            uint16_t f = c.script[chunk_instr++];
            f |= (uint16_t(c.script[chunk_instr++]) << 8);
            uint8_t t = c.script[chunk_instr++];
            bool fs = story_flag_get(f);
            if(instr == CMD_BRFC) fs = !fs;
            if(fs) chunk_instr = t;
            break;
        }
        case CMD_BRNT: {
            uint8_t t = c.script[chunk_instr++];
            uint8_t i = c.script[chunk_instr++];
            if(t != sel_tile) chunk_instr = i;
            break;
        }
        case CMD_BRNW: {
            uint8_t t = c.script[chunk_instr++];
            uint8_t i = c.script[chunk_instr++];
            if(t != walk_tile) chunk_instr = i;
            break;
        }

        default: break;
        }
    }
    return false;
}

bool run_chunks()
{
    if(!chunks_are_running) {
        running_chunk = 0;
        chunk_instr = 0;
        chunks_are_running = true;
        for(auto& r : chunk_regs)
            r = 0;
    }
    while(running_chunk < 4) {
        if(run_chunk()) return true;
        ++running_chunk;
        chunk_instr = 0;
        for(auto& r : chunk_regs)
            r = 0;
    }
    chunks_are_running = false;
    return false;
}

static void load_chunk(uint8_t index, uint8_t cx, uint8_t cy)
{
    map_chunk_t* chunk = &active_chunks[index].chunk;
    if(cx == 255 || cy == 255) {
        for(uint8_t i = 0; i < 32; ++i)
            chunk->tiles_flat[i] = 30;
        for(uint8_t i = 0; i < CHUNK_SCRIPT_SIZE; ++i)
            chunk->script[i] = 0;
        return;
    }
    uint16_t ci = cy * MAP_CHUNK_W + cx;
    uint24_t addr = uint24_t(ci) * sizeof(map_chunk_t);
    platform_fx_read_data_bytes(addr, chunk, sizeof(map_chunk_t));
}

void load_chunks()
{
    uint8_t cx = uint8_t(uint16_t(px - 64) >> 7);
    uint8_t cy = uint8_t(uint16_t(py - 32) >> 6);

    // TODO: optimize shifting map?
    // if(loaded_cx == cx && loaded_cy == cy)
    //	return;

    loaded_cx = cx;
    loaded_cy = cy;

    load_chunk(0, cx + 0, cy + 0);
    load_chunk(1, cx + 1, cy + 0);
    load_chunk(2, cx + 0, cy + 1);
    load_chunk(3, cx + 1, cy + 1);
}

bool tile_is_solid(uint16_t tx, uint16_t ty)
{
    uint8_t cx = uint8_t(tx >> 7);
    uint8_t cy = uint8_t(ty >> 6);
    cx -= loaded_cx;
    cy -= loaded_cy;
    if(cx > 1 || cy > 1) return true;
    uint8_t x = uint8_t(tx & 0x7f) >> 4;
    uint8_t y = uint8_t(ty & 0x3f) >> 4;
    uint8_t ci = cy * 2 + cx;
    uint8_t t = active_chunks[ci].chunk.tiles[y][x];
    t = pgm_read_byte(&TILE_SOLID[t]);
    x = 1;
    if(ty & 0x08) x <<= 2;
    if(tx & 0x08) x <<= 1;
    return (t & x) != 0;
}
