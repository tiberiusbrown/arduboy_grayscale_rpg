#include "common.hpp"

#include <string.h>

#include "generated/fxdata.h"
#include "script_commands.hpp"

#include "generated/tile_solid.hpp"

static void reset_sprite(sprite_t& e)
{
    e.path_index = 0;
    e.x = (e.path[0] & 7) * 16;
    e.y = ((e.path[0] >> 3) & 3) * 16;
    e.frames_rem = 2;
    e.dir = 0x80;
    e.active = (e.path_num > 0);
    if(e.path_dir != 0) e.path_dir = 1;
}

static bool chunk_sprite_defined;

static bool run_chunk()
{
    auto& ac = active_chunks[running_chunk];
    auto& sprite = chunk_sprites[running_chunk];
    auto& c = ac.chunk;
    // reset tiles and sprite, and set regs at the beginning of chunk script
    if(!chunks_are_running)
    {
        uint16_t ci = ac.cy * MAP_CHUNK_COLS + ac.cx;
        uint24_t addr = MAPDATA + uint24_t(ci) * sizeof(map_chunk_t);
        platform_fx_read_data_bytes(addr, c.tiles_flat, 32);
        chunk_sprite_defined = false;
    }
    uint8_t walk_tile = INVALID;
    uint8_t sel_tile = INVALID;
    bool sel_sprite = false;
    {
        static_assert(MAP_CHUNK_COLS <= 32, "expand tx to 16-bit");
        static_assert(MAP_CHUNK_ROWS <= 64, "expand ty to 16-bit");
        uint8_t tx = ac.cx * 8;
        uint8_t ty = ac.cy * 4;
        uint8_t dx, dy;
        {
            uint16_t tpx = px + 8;
            uint16_t tpy = py + 8;
            dual_shift_u16<4>(tpx, tpy);
            dx = (uint8_t)tpx - tx;
            dy = (uint8_t)tpy - ty;
        }
        //dx = div16_u16(px + 8) - tx;
        //dy = div16_u16(py + 8) - ty;
        if(dx < 8 && dy < 4) walk_tile = dy * 8 + dx;
        {
            uint16_t tpx = selx;
            uint16_t tpy = sely;
            dual_shift_u16<4>(tpx, tpy);
            dx = (uint8_t)tpx - tx;
            dy = (uint8_t)tpy - ty;
        }
        //dx = div16_u16(selx) - tx;
        //dy = div16_u16(sely) - ty;
        if(dx < 8 && dy < 4)
        {
            sel_tile = dy * 8 + dx;
            uint8_t ex = sprite.x;
            uint8_t ey = sprite.y;
            dx = uint8_t(uint8_t(selx & 127) - ex);
            dy = uint8_t(uint8_t(sely &  63) - ey);
            if((dx | dy) < 16)
                sel_sprite = true;
        }
    }
    if(!chunks_are_running)
    {
        savefile.chunk_regs[1] = sel_tile;
        savefile.chunk_regs[2] = walk_tile;
    }
    // TODO: optimize no_state_actions line below
    bool no_state_actions = (
        state == STATE_RESUME ||
        state == STATE_TITLE || 
        state == STATE_TP);
    uint8_t const* instr_ptr = &c.script[chunk_instr];
    while(instr_ptr < &c.script[0] + sizeof(c.script))
    {
        script_command_t instr = (script_command_t)deref_inc(instr_ptr);
        switch(instr)
        {
        case CMD_END:
            goto finish_chunk;

            // message/dialog
        case CMD_MSG:
        case CMD_TMSG:
        case CMD_DLG:
        case CMD_TDLG:
        {
            uint8_t t = sel_tile, portrait = INVALID;
            static_assert(!((CMD_TMSG | CMD_TDLG) & 1), "change next line");
            if(!(instr & 1)) t = deref_inc(instr_ptr);
            if(instr >= CMD_DLG) portrait = deref_inc(instr_ptr);
            uint16_t stri = deref_inc(instr_ptr);
            stri |= uint16_t(deref_inc(instr_ptr)) << 8;
            if(no_state_actions) break;
            if(t != sel_tile)
                break;
            change_state(STATE_DIALOG);
            sdata.dialog.name[0] = (char)INVALID;
            if(portrait != INVALID)
            {
                platform_fx_read_data_bytes(
                    PORTRAIT_STRINGS + sizeof(sdata.dialog.name) * portrait,
                    sdata.dialog.name,
                    sizeof(sdata.dialog.name));
            }
            platform_fx_read_data_bytes(STRINGDATA + stri, sdata.dialog.message,
                                        sizeof(sdata.dialog.message));
            //wrap_text(sdata.dialog.message, 128);
            goto pause_chunk;
        }
        case CMD_BAT:
        case CMD_EBAT:
        {
            uint8_t f = deref_inc(instr_ptr);
            f |= uint16_t(deref_inc(instr_ptr)) << 8;
            if(no_state_actions || story_flag_get(f))
            {
                instr_ptr += 4;
                break;
            }
            uint8_t e[4];
            e[0] = deref_inc(instr_ptr);
            e[1] = deref_inc(instr_ptr);
            e[2] = deref_inc(instr_ptr);
            e[3] = deref_inc(instr_ptr);
            change_state(STATE_BATTLE);
            sdata.battle.remove_enemy = (instr == CMD_EBAT);
            uint8_t const* eptr = &e[0];
            for(uint8_t i = 0; i < 4; ++i)
            {
                auto& enemy = sdata.battle.enemies[i];
                uint8_t id = deref_inc(eptr);
                enemy.id = id;
                enemy.hp = pgm_read_byte(&ENEMY_INFO[id].mhp);
            }
            sdata.battle.phase = BPHASE_ALERT;
            sdata.battle.pdef = sdata.battle.edef = INVALID;
            sdata.battle.defender_id = INVALID;
            sdata.battle.flag = f;
            story_flag_set(f);
            goto pause_chunk;
        }

        // teleport
        case CMD_TP:
        case CMD_TTP:
        case CMD_WTP:
        {
            uint8_t t;
            if(instr != CMD_TP) t = deref_inc(instr_ptr);
            static_assert(MAP_CHUNK_COLS <= 32, "expand to 16-bit");
            static_assert(MAP_CHUNK_ROWS <= 64, "expand to 16-bit");
            uint8_t tx, ty;
            tx = deref_inc(instr_ptr);
            //tx |= uint16_t(deref_inc(instr_ptr)) << 8;
            ty = deref_inc(instr_ptr);
            //ty |= uint16_t(deref_inc(instr_ptr)) << 8;
            if(no_state_actions) break;
            if(instr == CMD_TTP && t != sel_tile) break;
            if(instr == CMD_WTP && t != walk_tile) break;
            change_state(STATE_TP);
            sdata.tp.tx = tx;
            sdata.tp.ty = ty;
            goto pause_chunk;
        }

        case CMD_DIE:
            change_state(STATE_DIE);
            goto pause_chunk;

        case CMD_ADD:
        case CMD_SUB:
        {
            uint8_t t = deref_inc(instr_ptr);
            uint8_t dst = t & 0xf;
            uint8_t src = nibswap(t) & 0xf;
            src = savefile.chunk_regs[src];
            if(instr == CMD_SUB)
                src = uint8_t(-src);
            savefile.chunk_regs[dst] += src;
            break;
        }
        case CMD_ADDI:
        case CMD_ANDI:
        {
            uint8_t t = deref_inc(instr_ptr);
            int8_t imm = (int8_t)deref_inc(instr_ptr);
            uint8_t dst = t & 0xf;
            uint8_t src = nibswap(t) & 0xf;
            src = savefile.chunk_regs[src];
            if(instr == CMD_ANDI)
            {
                savefile.chunk_regs[dst] = src & imm;
                break;
            }
            int8_t newdst = (int8_t)src + imm;
            int8_t diff = newdst - (int8_t)savefile.chunk_regs[dst];
            savefile.chunk_regs[dst] = (uint8_t)newdst;
            if(!no_state_actions && dst >= 8 && diff > 0)
            {
                // special message
                change_state(STATE_DIALOG);
                sdata.dialog.name[0] = (char)254;
                char* ptr = sdata.dialog.message;
                store_inc(ptr, 'x');
                ptr += dec_to_str(ptr, (uint8_t)diff);
                store_inc(ptr, ' ');
                platform_fx_read_data_bytes(
                    ITEM_INFO + sizeof(item_info_t) * NUM_ITEMS +
                    (ITEM_NAME_LEN + ITEM_DESC_LEN) * (dst - 8),
                    ptr, ITEM_NAME_LEN + ITEM_DESC_LEN);
                for(uint8_t i = 0; i < ITEM_NAME_LEN - 1; ++i)
                {
                    char& c = ptr[i];
                    if(c == '\0') c = ' ';
                }
                ptr[ITEM_NAME_LEN - 1] = '\n';
                goto pause_chunk;
            }
            break;
        }
        //case CMD_ANDI:
        //{
        //    uint8_t t = deref_inc(instr_ptr);
        //    uint8_t imm = deref_inc(instr_ptr);
        //    uint8_t dst = t & 0xf;
        //    uint8_t src = nibswap(t) & 0xf;
        //    savefile.chunk_regs[dst] = savefile.chunk_regs[src] & imm;
        //    break;
        //}

        case CMD_FS:
        case CMD_FC:
        case CMD_FT:
        {
            uint16_t f = deref_inc(instr_ptr);
            f |= (uint16_t(deref_inc(instr_ptr)) << 8);
            if(no_state_actions) break;
            uint8_t i = f >> 3;
            uint8_t m = bitmask((uint8_t)f);
            uint8_t* p = &story_flags[i];
            i = *p;
            if(instr == CMD_FC)
            {
                *p = i & ~m;
                break;
            }
            if(instr == CMD_FT)
            {
                *p = i ^ m;
                break;
            }
            uint8_t already_have = (i & m);
            *p = i | m;
            if(!no_state_actions && !already_have && f < NUM_ITEMS)
            {
                // special message
                change_state(STATE_DIALOG);
                sdata.dialog.name[0] = (char)254;
                static char const YOU_FOUND[] PROGMEM = "Item: ";
                memcpy_P(sdata.dialog.message, YOU_FOUND, sizeof(YOU_FOUND));
                platform_fx_read_data_bytes(
                    ITEM_INFO + 9 + sizeof(item_info_t) * f,
                    &sdata.dialog.message[sizeof(YOU_FOUND) - 1],
                    ITEM_NAME_LEN + ITEM_DESC_LEN);
                for(uint8_t i = 0; i < ITEM_NAME_LEN - 1; ++i)
                {
                    char& c = sdata.dialog.message[sizeof(YOU_FOUND) - 1 + i];
                    if(c == '\0') c = ' ';
                }
                sdata.dialog.message[sizeof(YOU_FOUND) - 1 + ITEM_NAME_LEN - 1] = '\n';
                goto pause_chunk;
            }
            break;
        }
        case CMD_EP:
        case CMD_EPF:
        {
            uint16_t f;
            if(instr == CMD_EPF)
            {
                f = deref_inc(instr_ptr);
                f |= (uint16_t(deref_inc(instr_ptr)) << 8);
            }
            uint8_t id = deref_inc(instr_ptr);
            uint8_t n = deref_inc(instr_ptr);
            uint8_t open = deref_inc(instr_ptr);
            if(instr == CMD_EPF && story_flag_get(f))
            {
                instr_ptr += n;
                break;
            }
            bool reset = false;
            sprite_t s = sprite;
            if(s.type != id)
                reset = true;
            s.type = id;
            for(uint8_t j = 0; j < n; ++j)
            {
                uint8_t p = deref_inc(instr_ptr);
                if(s.path[j] != p)
                    reset = true;
                s.path[j] = p;
            }
            s.path_num = n;
            s.active = (n > 0);
            if(open == 0)
                s.path_dir = 0;
            else if(s.path_dir == 0)
                s.path_dir = 1;
            if(reset)
                reset_sprite(s);
            sprite = s;
            chunk_sprite_defined = true;
            break;
        }
        case CMD_EPT:
        case CMD_EPTR:
        {
            uint8_t t = deref_inc(instr_ptr);
            if(instr == CMD_EPTR)
                t = savefile.chunk_regs[t];
            sprite.target = t;
            break;
        }
        case CMD_ST:
        case CMD_STF:
        case CMD_STR:
        {
            uint8_t t = deref_inc(instr_ptr);
            if(instr == CMD_STR)
            {
                MY_ASSERT(t < 8);
                t = savefile.chunk_regs[t];
            }
            MY_ASSERT(t < 32);
            uint16_t f;
            if(instr == CMD_STF)
            {
                f = deref_inc(instr_ptr);
                f |= (uint16_t(deref_inc(instr_ptr)) << 8);
            }
            uint8_t i = deref_inc(instr_ptr);
            if(instr != CMD_STF || story_flag_get(f))
                c.tiles_flat[t] = i;
            break;
        }
        case CMD_PA:
        {
            uint8_t id = deref_inc(instr_ptr);
#if 0
            if(nparty >= 4) break;
            for(uint8_t u = 0; u < nparty; ++u)
                if(party[u].battle.id == id)
                    break;
#else
            MY_ASSERT(nparty < 4);
            for(uint8_t u = 0; u < nparty; ++u)
                MY_ASSERT(party[u].battle.id != id);
#endif
            party[nparty].battle.id = id;
            party[nparty].battle.hp = party_mhp(nparty);
            ++nparty;
            if(no_state_actions) break;
            change_state(STATE_DIALOG);
            sdata.dialog.name[0] = char(0x80 + pgm_read_byte(&PARTY_INFO[id].portrait));
            char* m = sdata.dialog.message;
            char const* n = pgmptr(&PARTY_INFO[id].name);
            char c;
            do *m++ = c = (char)pgm_read_byte_inc(n);
            while(c != '\0');
            static char const JOINED[] PROGMEM = " has joined the party!";
            memcpy_P(m - 1, JOINED, sizeof(JOINED));
            goto pause_chunk;
        }
        case CMD_OBJ:
        {
            savefile.objx = deref_inc(instr_ptr);
            savefile.objy = deref_inc(instr_ptr);
            break;
        }
        case CMD_HEAL:
            for(uint8_t i = 0; i < nparty; ++i)
                party[i].battle.hp = party_mhp(i);
            break;
        case CMD_SOLVED:
            platform_audio_play_sfx(SFX_SOLVED, 0);
            break;

        case CMD_JMP:
        {
            int8_t i = deref_inc(instr_ptr);
            instr_ptr += i;
            break;
        }
        case CMD_BZ:
        {
            uint8_t t = deref_inc(instr_ptr);
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(savefile.chunk_regs[t] == 0) instr_ptr += i;
            break;
        }
        case CMD_BNZ:
        {
            uint8_t t = deref_inc(instr_ptr);
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(savefile.chunk_regs[t] != 0) instr_ptr += i;
            break;
        }
        case CMD_BNEQ:
        {
            uint8_t t = deref_inc(instr_ptr);
            uint8_t imm = deref_inc(instr_ptr);
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(savefile.chunk_regs[t] != imm)
                instr_ptr += i;
            break;
        }
        case CMD_BGEQ:
        {
            uint8_t t = deref_inc(instr_ptr);
            uint8_t imm = deref_inc(instr_ptr);
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(savefile.chunk_regs[t] >= imm)
                instr_ptr += i;
            break;
        }
        case CMD_BFS:
        case CMD_BFC:
        {
            uint16_t f = deref_inc(instr_ptr);
            f |= (uint16_t(deref_inc(instr_ptr)) << 8);
            int8_t t = (int8_t)deref_inc(instr_ptr);
            bool fs = story_flag_get(f);
            if(instr == CMD_BFC) fs = !fs;
            if(fs) instr_ptr += t;
            break;
        }
        case CMD_BNST:
        {
            uint8_t t = deref_inc(instr_ptr);
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(t != sel_tile) instr_ptr += i;
            break;
        }
        case CMD_BNWT:
        {
            uint8_t t = deref_inc(instr_ptr);
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(t != walk_tile) instr_ptr += i;
            break;
        }
        case CMD_BNWE:
        {
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(!sprite_contacts_player(ac, sprite)) instr_ptr += i;
            else sprite.walking = false;
            break;
        }
        case CMD_BNSE:
        {
            int8_t i = (int8_t)deref_inc(instr_ptr);
            if(!sel_sprite) instr_ptr += i;
            break;
        }
        case CMD_BNI:
        case CMD_BNAI:
        {
            uint16_t f = deref_inc(instr_ptr);
            f |= (uint16_t(deref_inc(instr_ptr)) << 8);
            int8_t t = (int8_t)deref_inc(instr_ptr);
            bool jmp;
            if(instr == CMD_BNAI)
                jmp = !user_is_wearing(0, (item_t)f);
            else
            {
                jmp = true;
                for(uint8_t i = 0; i < nparty; ++i)
                    if(user_is_wearing(i, (item_t)f))
                        jmp = false;
            }
            if(jmp) instr_ptr += t;
            break;
        }
        case CMD_BPF:
        {
            int8_t t = (int8_t)deref_inc(instr_ptr);
            if(nparty >= 4)
                instr_ptr += t;
            break;
        }
        case CMD_BNET:
        {
            int8_t t = (int8_t)deref_inc(instr_ptr);
            if(sprite.target == 0 || !(sprite.dir & 0x80))
                instr_ptr += t;
            break;
        }

        default: break;
        }
    }

finish_chunk:
    if(!chunk_sprite_defined)
        sprite.active = false;
    return false;

pause_chunk:
    chunk_instr = uint8_t(ptrdiff_t(instr_ptr - &c.script[0]));
    return true;
}

static void clamp_regs()
{
    savefile.chunk_regs[0] = 0;
    for(uint8_t i = 0; i < NUM_CONSUMABLES; ++i)
    {
        uint8_t* xp = &consumables[i];
        int8_t x = int8_t(*xp);
        if(x < 0) x = 0;
        if(x > 99) x = 99;
        *xp = (uint8_t)x;
    }
}

bool run_chunks()
{
    if(!chunks_are_running)
    {
        running_chunk = 0;
        chunk_instr = 0;
        uint8_t t = savefile.chunk_regs[6];
        if(t > 0) t -= 1;
        savefile.chunk_regs[6] = t;
    }
    while(running_chunk < 4)
    {
        if(run_chunk())
        {
            clamp_regs();
            chunks_are_running = true;
            return true;
        }
        ++running_chunk;
        chunk_instr = 0;
    }
    clamp_regs();
    chunks_are_running = false;
    return false;
}

static void load_chunk(uint8_t index, uint8_t cx, uint8_t cy)
{
    active_chunk_t& active_chunk = active_chunks[index];
    if(active_chunk.cx != cx || active_chunk.cy != cy)
    {
        map_chunk_t* chunk = &active_chunk.chunk;
        uint16_t ci = cy * MAP_CHUNK_COLS + cx;
        uint24_t addr = MAPDATA + uint24_t(ci) * sizeof(map_chunk_t);
        static_assert(sizeof(active_chunk) ==
            sizeof(map_chunk_t) + sizeof(active_chunk.cx) + sizeof(active_chunk.cy),
            "revisit commented line below");
        //memset(&active_chunk, 0, sizeof(active_chunk));
        if(!savefile.loaded)
            memset(&chunk_sprites[index], 0, sizeof(sprite_t));
        active_chunk.cx = cx;
        active_chunk.cy = cy;
        platform_fx_read_data_bytes(addr, chunk, sizeof(map_chunk_t));
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
    // diagonal shifts
    if(cx == uint8_t(pcx + 1) && cy == uint8_t(pcy + 1)) // SE
        shift_chunk(0, 3);
    if(uint8_t(cx + 1) == pcx && uint8_t(cy + 1) == pcy) // SW
        shift_chunk(3, 0);
    if(cx == uint8_t(pcx + 1) && uint8_t(cy + 1) == pcy) // NE
        shift_chunk(2, 1);
    if(uint8_t(cx + 1) == pcx && cy == uint8_t(pcy + 1)) // SW
        shift_chunk(1, 2);

    for(uint8_t i = 0; i < 4; ++i)
        load_chunk(i, cx + (i & 1), cy + lsr(i));
}

uint8_t tile_at(uint16_t tx, uint16_t ty)
{
    uint8_t t = 0;
    check_solid(tx, ty, &t);
    return t;
}

bool check_solid(uint16_t tx, uint16_t ty, uint8_t* tp)
{
    // check tile
    uint8_t cx = uint8_t(tx >> 7);
    uint8_t cy = uint8_t(ty >> 6);
    cx -= active_chunks[0].cx;
    cy -= active_chunks[0].cy;
    if((cx | cy) > 1) return true;
    uint8_t ctx = uint8_t(tx) & 127;
    uint8_t cty = uint8_t(ty) & 63;
    uint8_t ci = cy * 2 + cx;

    // fetch tile
    auto const& c = active_chunks[ci];
#ifndef ARDUINO
    uint8_t t = c.chunk.tiles[cty / 16][ctx / 16];
#else
    uint8_t t;
    {
        uint8_t tmpx = ctx;
        uint8_t tmpy = cty;
        uint8_t const* ptr = c.chunk.tiles_flat;
        asm volatile(R"ASM(
                lsr  %[tmpy]
                andi %[tmpy], 0xf8
                swap %[tmpx]
                andi %[tmpx], 0x0f
                add  %[tmpx], %[tmpy]
                add  %A[ptr], %[tmpx]
                adc  %B[ptr], __zero_reg__
                ld   %[t], %a[ptr]
            )ASM"
            :
            [tmpx] "+&d" (tmpx),
            [tmpy] "+&d" (tmpy),
            [t]    "=&r" (t),
            [ptr]  "+&e" (ptr)
            );
    }
#endif
    if(tp) *tp = t;

    // check sprite
    auto const& e = chunk_sprites[ci];
    if(e.active)
    {
        uint8_t ex = e.x;
        uint8_t ey = e.y;
        if(uint8_t(ctx - ex - 2) < 12 && uint8_t(cty - ey - 2) < 14)
            return true;
    }
    uint8_t const* ptr = &TILE_SOLID[0];
    if(ty >= MAP_CHUNK_ROWS / 2 * 64)
    {
        ptr += 128;
        if(tx >= MAP_CHUNK_COLS / 2 * 128)
            ptr += 128;
    }
    uint8_t f = pgm_read_byte(ptr + (t >> 1));
    if(t & 1) f = nibswap(f);
    // identify quarter tiles
    uint8_t q = 1;
    if(ty & 0x08) q <<= 2;
    if(tx & 0x08) q <<= 1;
    if((f & q) != 0) return true;
    return false;
}
