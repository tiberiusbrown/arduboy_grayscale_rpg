#include "common.hpp"

#include <string.h>

#include "generated/fxdata.h"
#include "script_commands.hpp"
#include "tile_solid.hpp"

static void reset_sprite(sprite_t& e)
{
    e.path_index = 0;
    e.x = (e.path[0] & 7) * 16;
    e.y = ((e.path[0] >> 3) & 3) * 16;
    e.frames_rem = 1;
    e.dir = 0x80;
    e.active = (e.path_num > 0);
    if(e.path_dir != 0) e.path_dir = 1;
}

static bool run_chunk()
{
    auto& ac = active_chunks[running_chunk];
    auto& sprite = chunk_sprites[running_chunk];
    auto& c = ac.chunk;
    {
        uint16_t ci = ac.cy * MAP_CHUNK_W + ac.cx;
        uint24_t addr = MAPDATA + uint24_t(ci) * sizeof(map_chunk_t);
        platform_fx_read_data_bytes(addr, c.tiles_flat, 32);
    }
    uint8_t walk_tile = 255;
    uint8_t sel_tile = 255;
    bool sel_sprite = false;
    {
        uint16_t tx = ac.cx * 8;
        uint16_t ty = ac.cy * 4;
        uint8_t dx, dy;
        dx = uint8_t(((px + 8) >> 4) - tx);
        dy = uint8_t(((py + 8) >> 4) - ty);
        if((dx | dy) < 8) walk_tile = dy * 8 + dx;
        dx = uint8_t((selx >> 4) - tx);
        dy = uint8_t((sely >> 4) - ty);
        if((dx | dy) < 8)
        {
            sel_tile = dy * 8 + dx;
            uint8_t ex = sprite.x;
            uint8_t ey = sprite.y;
            dx = uint8_t((selx & 127) - ex);
            dy = uint8_t((sely &  63) - ey);
            if((dx | dy) < 16)
                sel_sprite = true;
        }
    }
    bool no_state_actions = (
        state == STATE_RESUME ||
        state == STATE_TITLE || 
        state == STATE_TP);
    while(chunk_instr < CHUNK_SCRIPT_SIZE)
    {
        script_command_t instr = (script_command_t)c.script[chunk_instr++];
        switch(instr)
        {
        case CMD_END:
            return false;

            // message/dialog
        case CMD_MSG:
        case CMD_TMSG:
        case CMD_DLG:
        case CMD_TDLG:
        {
            uint8_t t0, t1;
            if(instr != CMD_MSG) t0 = c.script[chunk_instr++];
            if(instr == CMD_TDLG) t1 = c.script[chunk_instr++];
            uint16_t stri = c.script[chunk_instr++];
            stri |= uint16_t(c.script[chunk_instr++]) << 8;
            if(no_state_actions) break;
            static_assert(!((CMD_TMSG | CMD_TDLG) & 1), "");
            if(!(instr & 1) && t0 != sel_tile)
                break;
            change_state(STATE_DIALOG);
            uint8_t portrait = INVALID;
            if(instr == CMD_DLG) portrait = t0;
            if(instr == CMD_TDLG) portrait = t1;
            if(portrait != INVALID)
            {
                platform_fx_read_data_bytes(
                    PORTRAIT_STRINGS + sizeof(sdata.dialog.name) * portrait,
                    sdata.dialog.name,
                    sizeof(sdata.dialog.name));
            }
            sdata.dialog.portrait = portrait;
            platform_fx_read_data_bytes(STRINGDATA + stri, sdata.dialog.message,
                                        sizeof(sdata.dialog.message));
            //wrap_text(sdata.dialog.message, 128);
            return true;
        }
        case CMD_BAT:
        case CMD_EBAT:
        {
            uint8_t f = c.script[chunk_instr++];
            f |= uint16_t(c.script[chunk_instr++]) << 8;
            uint8_t e[4];
            e[0] = c.script[chunk_instr++];
            e[1] = c.script[chunk_instr++];
            e[2] = c.script[chunk_instr++];
            e[3] = c.script[chunk_instr++];
            if(no_state_actions) break;
            if(story_flag_get(f)) break;
            change_state(STATE_BATTLE);
            sdata.battle.remove_enemy = (instr == CMD_EBAT);
            sdata.battle.enemy_chunk = running_chunk;
            for(uint8_t i = 0; i < 4; ++i)
            {
                auto& enemy = sdata.battle.enemies[i];
                uint8_t id = e[i];
                enemy.id = id;
                enemy.hp = pgm_read_byte(&ENEMY_INFO[id].mhp);
            }
            sdata.battle.phase = BPHASE_ALERT;
            sdata.battle.pdef = sdata.battle.edef = INVALID;
            sdata.battle.defender_id = INVALID;
            sdata.battle.flag = f;
            //for(auto& p : party)
            //    p.battle.ap = 0;
            story_flag_set(f);
            return true;
        }

        // teleport
        case CMD_TP:
        case CMD_TTP:
        case CMD_WTP:
        {
            uint8_t t;
            if(instr != CMD_TP) t = c.script[chunk_instr++];
            uint16_t tx, ty;
            tx = c.script[chunk_instr++];
            tx |= uint16_t(c.script[chunk_instr++]) << 8;
            ty = c.script[chunk_instr++];
            ty |= uint16_t(c.script[chunk_instr++]) << 8;
            if(no_state_actions) break;
            if(instr == CMD_TTP && t != sel_tile) break;
            if(instr == CMD_WTP && t != walk_tile) break;
            change_state(STATE_TP);
            sdata.tp.tx = tx;
            sdata.tp.ty = ty;
            return true;
        }

        case CMD_ADD:
        {
            uint8_t t = c.script[chunk_instr++];
            uint8_t dst = t & 0xf;
            uint8_t src = t >> 4;
            savefile.chunk_regs[dst] += savefile.chunk_regs[src];
            break;
        }
        case CMD_ADDI:
        {
            uint8_t t = c.script[chunk_instr++];
            int8_t imm = (int8_t)c.script[chunk_instr++];
            uint8_t dst = t & 0xf;
            uint8_t src = t >> 4;
            int8_t newdst = savefile.chunk_regs[src] + imm;
            int8_t diff = newdst - savefile.chunk_regs[dst];
            savefile.chunk_regs[dst] = newdst;
            if(!no_state_actions && dst >= 8)
            {
                if(diff > 0)
                {
                    // special message
                    change_state(STATE_DIALOG);
                    sdata.dialog.portrait = 254;
                    char* ptr = sdata.dialog.message;
                    *ptr++ = 'x';
                    ptr += dec_to_str(ptr, (uint8_t)diff);
                    *ptr++ = ' ';
                    platform_fx_read_data_bytes(
                        ITEM_STRINGS + ITEM_TOTAL_LEN * NUM_ITEMS + ITEM_TOTAL_LEN * (dst - 8),
                        ptr, ITEM_TOTAL_LEN);
                    for(uint8_t i = 0; i < ITEM_NAME_LEN - 1; ++i)
                    {
                        char& c = ptr[i];
                        if(c == '\0') c = ' ';
                    }
                    ptr[ITEM_NAME_LEN - 1] = '\n';
                    return true;
                }
            }
            break;
        }
        case CMD_SUB:
        {
            uint8_t t = c.script[chunk_instr++];
            uint8_t dst = t & 0xf;
            uint8_t src = t >> 4;
            savefile.chunk_regs[dst] -= savefile.chunk_regs[src];
            break;
        }

        case CMD_FS:
        {
            uint16_t f = c.script[chunk_instr++];
            f |= (uint16_t(c.script[chunk_instr++]) << 8);
            if(no_state_actions) break;
            bool already_have = story_flag_get(f);
            story_flag_set(f);
            if(!no_state_actions && !already_have && f < NUM_ITEMS)
            {
                // special message
                change_state(STATE_DIALOG);
                sdata.dialog.portrait = 254;
                static char const YOU_FOUND[] PROGMEM = "Item: ";
                memcpy_P(sdata.dialog.message, YOU_FOUND, sizeof(YOU_FOUND));
                platform_fx_read_data_bytes(
                    ITEM_STRINGS + ITEM_TOTAL_LEN * f,
                    &sdata.dialog.message[sizeof(YOU_FOUND) - 1],
                    ITEM_TOTAL_LEN);
                for(uint8_t i = 0; i < ITEM_NAME_LEN - 1; ++i)
                {
                    char& c = sdata.dialog.message[sizeof(YOU_FOUND) - 1 + i];
                    if(c == '\0') c = ' ';
                }
                sdata.dialog.message[sizeof(YOU_FOUND) - 1 + ITEM_NAME_LEN - 1] = '\n';
                return true;
            }
            break;
        }
        case CMD_FC:
        {
            uint16_t f = c.script[chunk_instr++];
            f |= (uint16_t(c.script[chunk_instr++]) << 8);
            if(no_state_actions) break;
            story_flag_clr(f);
            break;
        }
        case CMD_FT:
        {
            uint16_t f = c.script[chunk_instr++];
            f |= (uint16_t(c.script[chunk_instr++]) << 8);
            if(no_state_actions) break;
            story_flag_tog(f);
            break;
        }
        case CMD_EP:
        case CMD_EPF:
        {
            uint16_t f;
            if(instr == CMD_EPF)
            {
                f = c.script[chunk_instr++];
                f |= (uint16_t(c.script[chunk_instr++]) << 8);
            }
            uint8_t id = c.script[chunk_instr++];
            uint8_t n = c.script[chunk_instr++];
            uint8_t open = c.script[chunk_instr++];
            if(instr == CMD_EPF && story_flag_get(f))
            {
                sprite.active = false;
                chunk_instr += n;
                break;
            }
            bool reset = false;
            if(sprite.type != id)
                reset = true;
            sprite.type = id;
            for(uint8_t j = 0; j < n; ++j)
            {
                if(sprite.path[j] != c.script[chunk_instr])
                    reset = true;
                sprite.path[j] = c.script[chunk_instr++];
            }
            sprite.path_num = n;
            sprite.active = (n > 0);
            if(!open)
                sprite.path_dir = 0;
            else if(sprite.path_dir == 0)
                sprite.path_dir = 1;
            if(reset) reset_sprite(sprite);
            break;
        }
        case CMD_ST:
        case CMD_STF:
        {
            uint8_t t = c.script[chunk_instr++];
            uint16_t f;
            if(instr == CMD_STF)
            {
                f = c.script[chunk_instr++];
                f |= (uint16_t(c.script[chunk_instr++]) << 8);
            }
            uint8_t i = c.script[chunk_instr++];
            if(instr != CMD_STF || !story_flag_get(f))
                c.tiles_flat[t] = i;
            break;
        }
        case CMD_PA:
            if(nparty >= 4)
                break;
            {
                uint8_t id = c.script[chunk_instr++];
                for(uint8_t u = 0; u < nparty; ++u)
                    if(party[u].battle.id == id)
                        break;
                party[nparty].battle.id = id;
                party[nparty].battle.hp = party_mhp(nparty);
                ++nparty;
                if(no_state_actions) break;
                change_state(STATE_DIALOG);
                sdata.dialog.portrait = 0x80 + pgm_read_byte(&PARTY_INFO[id].portrait);
                char* m = sdata.dialog.message;
                char const* n = pgmptr(&PARTY_INFO[id].name);
                char c;
                do *m++ = c = (char)pgm_read_byte_inc(n);
                while(c != '\0');
                static char const JOINED[] PROGMEM = " has joined the party!";
                memcpy_P(m - 1, JOINED, sizeof(JOINED));
                return true;
            }

        case CMD_JMP:
        {
            int8_t i = c.script[chunk_instr++];
            chunk_instr += i;
            break;
        }
        case CMD_BRZ:
        {
            uint8_t t = c.script[chunk_instr++];
            int8_t i = (int8_t)c.script[chunk_instr++];
            if(savefile.chunk_regs[t] == 0) chunk_instr += i;
            break;
        }
        case CMD_BRN:
        {
            uint8_t t = c.script[chunk_instr++];
            int8_t i = (int8_t)c.script[chunk_instr++];
            if(savefile.chunk_regs[t] < 0) chunk_instr += i;
            break;
        }
        case CMD_BRFS:
        case CMD_BRFC:
        {
            uint16_t f = c.script[chunk_instr++];
            f |= (uint16_t(c.script[chunk_instr++]) << 8);
            int8_t t = (int8_t)c.script[chunk_instr++];
            bool fs = story_flag_get(f);
            if(instr == CMD_BRFC) fs = !fs;
            if(fs) chunk_instr += t;
            break;
        }
        case CMD_BRNT:
        {
            uint8_t t = c.script[chunk_instr++];
            int8_t i = (int8_t)c.script[chunk_instr++];
            if(t != sel_tile) chunk_instr += i;
            break;
        }
        case CMD_BRNW:
        {
            uint8_t t = c.script[chunk_instr++];
            int8_t i = (int8_t)c.script[chunk_instr++];
            if(t != walk_tile) chunk_instr += i;
            break;
        }
        case CMD_BRNE:
        {
            int8_t i = (int8_t)c.script[chunk_instr++];
            if(!sprite_contacts_player(ac, sprite)) chunk_instr += i;
            else sprite.walking = false;
            break;
        }
        case CMD_BRNS:
        {
            int8_t i = (int8_t)c.script[chunk_instr++];
            if(!sel_sprite) chunk_instr += i;
            break;
        }
        case CMD_BRNI:
        {
            uint16_t f = c.script[chunk_instr++];
            f |= (uint16_t(c.script[chunk_instr++]) << 8);
            int8_t t = (int8_t)c.script[chunk_instr++];
            bool jmp = true;
            for(uint8_t i = 0; i < nparty; ++i)
                if(user_is_wearing(i, (item_t)f))
                    jmp = false;
            if(jmp) chunk_instr += t;
            break;
        }

        default: break;
        }
    }
    return false;
}

static void clamp_regs()
{
    savefile.chunk_regs[0] = 0;
    for(uint8_t i = 0; i < NUM_CONSUMABLES; ++i)
    {
        int8_t* xp = &savefile.chunk_regs[8 + i];
        int8_t x = *xp;
        if(x < 0) x = 0;
        if(x > 99) x = 99;
        *xp = x;
    }
}

bool run_chunks()
{
    if(!chunks_are_running)
    {
        running_chunk = 0;
        chunk_instr = 0;
        chunks_are_running = true;
    }
    while(running_chunk < 4)
    {
        if(run_chunk())
        {
            clamp_regs();
            return true;
        }
        ++running_chunk;
        chunk_instr = 0;
    }
    chunks_are_running = false;
    clamp_regs();
    return false;
}

static void load_chunk(uint8_t index, uint8_t cx, uint8_t cy)
{
    active_chunk_t& active_chunk = active_chunks[index];
    map_chunk_t* chunk = &active_chunk.chunk;
    uint16_t ci = cy * MAP_CHUNK_W + cx;
    uint24_t addr = MAPDATA + uint24_t(ci) * sizeof(map_chunk_t);
    if(active_chunk.cx != cx || active_chunk.cy != cy)
    {
        memset(&active_chunk, 0, sizeof(active_chunk));
        if(!savefile.loaded)
            memset(&chunk_sprites[index], 0, sizeof(sprite_t));
        active_chunk.cx = cx;
        active_chunk.cy = cy;
        platform_fx_read_data_bytes(addr, chunk, sizeof(map_chunk_t));
    }
    else if(cx != 255 && cy != 255) {
        //platform_fx_read_data_bytes(addr, chunk->tiles_flat, 32);
        return;
    }
    else
    {
        for(uint8_t i = 0; i < 32; ++i)
            chunk->tiles_flat[i] = 30;
        for(uint8_t i = 0; i < CHUNK_SCRIPT_SIZE; ++i)
            chunk->script[i] = 0;
        chunk_sprites[index].active = false;
        return;
    }
}

static void shift_chunk(uint8_t dst, uint8_t src)
{
    memcpy(&active_chunks[dst], &active_chunks[src],
           sizeof(active_chunk_t));
    memcpy(&chunk_sprites[dst], &chunk_sprites[src],
           sizeof(sprite_t));
}

void load_chunks()
{
    uint8_t cx = uint8_t(uint16_t(px - 64 + 8) >> 7);
    uint8_t cy = uint8_t(uint16_t(py - 32 + 8) >> 6);

    // shift chunks if possible
    // this way, enemies don't visibly reset as the player moves between chunks
    uint8_t pcx = active_chunks[0].cx;
    uint8_t pcy = active_chunks[0].cy;
    if(cx == pcx)
    {
        if(cy == uint8_t(pcy + 1))
        {
            // shift up
            shift_chunk(0, 2);
            shift_chunk(1, 3);
        }
        else if(cy == uint8_t(pcy - 1)) {
            // shift down
            shift_chunk(2, 0);
            shift_chunk(3, 1);
        }
    }
    if(cy == pcy)
    {
        if(cx == uint8_t(pcx + 1))
        {
            // shift left
            shift_chunk(0, 1);
            shift_chunk(2, 3);
        }
        else if(cx == uint8_t(pcx - 1)) {
            // shift right
            shift_chunk(1, 0);
            shift_chunk(3, 2);
        }
    }

    load_chunk(0, cx + 0, cy + 0);
    load_chunk(1, cx + 1, cy + 0);
    load_chunk(2, cx + 0, cy + 1);
    load_chunk(3, cx + 1, cy + 1);
}

bool check_solid(uint16_t tx, uint16_t ty)
{
    // check tile
    uint8_t cx = uint8_t(tx >> 7);
    uint8_t cy = uint8_t(ty >> 6);
    cx -= active_chunks[0].cx;
    cy -= active_chunks[0].cy;
    if(cx > 1 || cy > 1) return true;
    uint8_t ctx = uint8_t(tx) & 127;
    uint8_t cty = uint8_t(ty) & 63;
    uint8_t ci = cy * 2 + cx;
    auto const& c = active_chunks[ci];
    uint8_t t = c.chunk.tiles[cty / 16][ctx / 16];
    t = pgm_read_byte(&TILE_SOLID[t]);
    // identify quarter tiles
    uint8_t q = 1;
    if(ty & 0x08) q <<= 2;
    if(tx & 0x08) q <<= 1;
    if((t & q) != 0) return true;
    // check sprite
    auto const& e = chunk_sprites[ci];
    if(e.active)
    {
        uint8_t ex = e.x;
        uint8_t ey = e.y;
        if(uint8_t(ctx - ex - 2) < 12 && uint8_t(cty - ey - 4) < 12)
            return true;
    }
    return false;
}
