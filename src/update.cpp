#include "common.hpp"

#include "generated/fxdata.h"
#include "generated/num_game_over_messages.hpp"

static int8_t const DIRS[9 * 2] PROGMEM = {
    0, 1, -1, 1, -1, 0, -1, -1, 0, -1, 1, -1, 1, 0, 1, 1, 0, 0
};

void back_to_map()
{
    if(!(chunks_are_running && run_chunks()))
        change_state(STATE_MAP);
}

static uint8_t solid_mask()
{
#if ARDUINO_ARCH_AVR
    uint16_t tpx, tpy;
    uint8_t m;
    uint16_t zero = 0;
    asm volatile(R"ASM(
        clr  %[m]
        lds  r24, %[ppx]+0
        lds  r25, %[ppx]+1
        adiw r24, 5
        movw %A[px], r24

        lds  r22, %[ppy]+0
        lds  r23, %[ppy]+1
        subi r22, lo8(-5)
        sbci r23, hi8(-5)
        movw %A[py], r22
        movw r20, %[z]

        %~call %x[cs]
        cpse r24, __zero_reg__
        ori  %[m], 1

        movw r24, %[px]
        adiw r24, 5
        movw r22, %[py]
        movw r20, %[z]
        %~call %x[cs]
        cpse r24, __zero_reg__
        ori  %[m], 2

        movw r24, %[px]
        movw r22, %[py]
        subi r22, lo8(-5)
        sbci r23, hi8(-5)
        movw %[py], r22
        movw r20, %[z]
        %~call %x[cs]
        cpse r24, __zero_reg__
        ori  %[m], 4

        movw r24, %[px]
        adiw r24, 5
        movw r22, %[py]
        movw r20, %[z]
        %~call %x[cs]
        cpse r24, __zero_reg__
        ori  %[m], 8
        )ASM"
        :
    [m] "=&d" (m),
        [px]  "=&r" (tpx),
        [py]  "=&r" (tpy)
        :
        [ppx] ""    (&px),
        [ppy] ""    (&py),
        [cs]  "i"   (&check_solid),
        [z]   "r"   (zero)
        :
        "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",
        "r26", "r27", "r30", "r31", "memory"
        );
#else
    uint8_t m = 0;
    if(check_solid(px + 5, py + 5)) m |= 1;
    if(check_solid(px + 10, py + 5)) m |= 2;
    if(check_solid(px + 5, py + 10)) m |= 4;
    if(check_solid(px + 10, py + 10)) m |= 8;
#endif
    return m;
}

static bool rect_intersect(
    uint16_t x0, uint16_t y0, uint8_t w0, uint8_t h0,
    uint16_t x1, uint16_t y1, uint8_t w1, uint8_t h1)
{
    if(x0 + w0 <= x1) return false;
    if(y0 + h0 <= y1) return false;
    if(x1 + w1 <= x0) return false;
    if(y1 + h1 <= y0) return false;
    return true;
}

bool sprite_collides_player(active_chunk_t const& c, sprite_t const& e)
{
    if(!e.active) return false;
    uint16_t ex = c.cx * 128 + e.x;
    uint16_t ey = c.cy * 64 + e.y;
    return rect_intersect(
        ex + 2, ey + 2, 12, 14,
        px + 5, py + 5, 6, 6);
}

bool sprite_contacts_player(active_chunk_t const& c, sprite_t const& e)
{
    if(!e.active) return false;
    uint16_t ex = c.cx * 128 + e.x;
    uint16_t ey = c.cy * 64 + e.y;

    uint8_t flags = pgm_read_byte(&SPRITE_FLAGS[e.type]);

    if(flags & SF_SMALL_RECT)
    {
        return rect_intersect(
            ex, ey + 2, 16, 16,
            px + 4, py + 4, 8, 8);
    }

    // sprite rect is x + 2, y + 4, 12, 12
    // player rect is x + 4, y + 4, 8, 8
    return rect_intersect(
        ex - 2, ey, 20, 20,
        px + 4, py + 4, 8, 8);
}

static inline void update_sprite(active_chunk_t& c, sprite_t& e)
{
    if(!e.active) return;
    uint8_t flags = pgm_read_byte(&SPRITE_FLAGS[e.type]);
    if(!(flags & SF_FAST) && (nframe & 1)) return;

    // no path or just waiting on a single tile
    if(!e.target && e.path_num <= 1)
        return;

    // check collision with player
    if(flags & SF_DONT_STOP)
        e.walking = true;
    else if(state == STATE_MAP || state == STATE_TITLE)
    {
        e.walking = !sprite_contacts_player(c, e);
        if(!e.walking)
            return;
    }

    if(e.dir < 8)
    {
        int8_t const* ptr = &DIRS[e.dir * 2];
        int8_t dx = (int8_t)pgm_read_byte_inc(ptr);
        int8_t dy = (int8_t)pgm_read_byte(ptr);
        e.x += dx;
        e.y += dy;

        if((flags & SF_DONT_STOP) && sprite_collides_player(c, e))
        {
            px += dx;
            py += dy;
            static uint8_t const DIRMASKS[4] PROGMEM = { 12, 5, 3, 10 };
            uint8_t m = pgm_read_byte(&DIRMASKS[lsr(e.dir)]);
            if(solid_mask() & m)
            {
                change_state(STATE_DIE);
                return;
            }
        }
    }

    if(--e.frames_rem != 0)
        return;

#if 1
    if(e.target)
    {
        uint8_t target = e.target & 31;
        uint8_t tx = (target & 7) * 16;
        uint8_t ty = (target >> 3) * 16;
        uint8_t ex = e.x;
        uint8_t ey = e.y;
        uint8_t dir;
        if(((ex ^ tx) | (ey ^ ty)) == 0)
            dir = 0x80;
        else if(ex != tx)
            dir = (ex < tx ? 6 : 2);
        else
            dir = (ey < ty ? 0 : 4);
        e.dir = dir;
        e.frames_rem = 1;
        return;
    }
#endif

    uint8_t path_index = e.path_index;
    if(!(e.dir & 0x80))
    {
        uint8_t t = e.path[path_index];
        if(t & 0xe0)
        {
            // delay
            e.frames_rem = (t >> 5) * 16;
            e.dir = 0x80;
            return;
        }
    }
    if(e.path_dir == 0)
    {
        if(++path_index == e.path_num)
            path_index = 0;
    }
    else if(e.path_dir == 1)
    {
        if(++path_index == e.path_num)
            path_index -= 2, e.path_dir = 2;
    }
    else if(e.path_dir == 2)
    {
        if(path_index-- == 0)
            path_index = 1, e.path_dir = 1;
    }
    e.path_index = path_index;
    uint8_t t = e.path[path_index];
    uint8_t x = nibswap(t & 7); // (t & 7) * 16
    uint8_t y = ((t >> 3) & 3) * 16;
    //uint8_t y = nibswap((nibswap(t) << 1) & 3);
    uint8_t frames_rem, dir;
    uint8_t ex = e.x, ey = e.y;
    if(x < ex)
    {
        dir = 2;
        frames_rem = ex - x;
    }
    else if(x > ex) {
        dir = 6;
        frames_rem = x - ex;
    }
    else if(y < ey) {
        dir = 4;
        frames_rem = ey - y;
    }
    else {
        dir = 0;
        frames_rem = y - ey;
    }
    e.dir = dir;
    e.frames_rem = frames_rem;
}

static void update_sprites()
{
    for(uint8_t i = 0; i < 4; ++i)
        update_sprite(active_chunks[i], chunk_sprites[i]);
}

static void check_explored(int16_t x, int16_t y)
{
    if(x < 0) return;
    if(y < 0) return;
    if(x >= MAP_CHUNK_COLS * 128) return;
    if(y >= MAP_CHUNK_ROWS / 2 * 64) return;
    uint8_t ex = x / (EXPLORED_TILES * 16);
    uint8_t ey = y / (EXPLORED_TILES * 16);
    uint16_t n = ey * EXPLORED_COLS + ex;
    uint8_t i = n >> 3;
    uint8_t m = bitmask((uint8_t)n);
    MY_ASSERT(i < EXPLORED_BYTES);
    savefile.explored[i] |= m;
}

static void update_map()
{
    if(chunks_are_running && run_chunks())
        return;

    if(tilesheet() == 2)
    {
        // check if died by falling into pit when magic floor disappears
        // or by standing on spikes
        uint8_t t = tile_at(px + 8, py + 8);
        if(t == 0 || t == 6 || t == 55)
        {
            change_state(STATE_DIE);
            return;
        }
    }

    if(btns_pressed & BTN_B)
    {
        change_state(STATE_PAUSE);
        return;
    }

    update_sprites();

    selx = sely = uint16_t(-1);
    if(!(btns_down & BTN_A))
        sdata.map.a_pressed = 0;
    else if(sdata.map.a_pressed != 0 && sdata.map.a_pressed < 16)
        ++sdata.map.a_pressed;
    if(btns_pressed & BTN_A)
    {
        int8_t const* ptr = &DIRS[pdir * 2];
        int8_t dx = (int8_t)pgm_read_byte_inc(ptr) * 8;
        int8_t dy = (int8_t)pgm_read_byte(ptr) * 8;
        selx = (px + 8 + dx);
        sely = (py + 8 + dy);
        sdata.map.a_pressed = 1;
    }
    
#if SLOW_DIAG
    bool enable_diag = false;
    {
        uint8_t d = diag_counter;
        d += 7;
        if(d >= 10)
            enable_diag = true, d -= 10;
        diag_counter = d;
    }
#else
    constexpr bool enable_diag = true;
#endif

    int8_t dx = 0, dy = 0;
    if(btns_down & BTN_UP) dy -= 1;
    if(btns_down & BTN_DOWN) dy += 1;
    if(btns_down & BTN_LEFT) dx -= 1;
    if(btns_down & BTN_RIGHT) dx += 1;

    pmoving = !(dx == 0 && dy == 0);
#if SLOW_DIAG
    enable_diag |= (dx == 0 || dy == 0);
#endif
        
    if(pmoving && enable_diag)
    {
        if(dy < 0)
        {
            if(dx < 0) pdir = 3;
            else if(dx == 0) pdir = 4;
            else pdir = 5;
        }
        else if(dy == 0) {
            if(dx < 0) pdir = 2;
            else pdir = 6;
        }
        else {
            if(dx < 0) pdir = 1;
            else if(dx == 0) pdir = 0;
            else pdir = 7;
        }

        int16_t prevx = px;
        int16_t prevy = py;

        // movement
        px += dx;
        py += dy;

        uint8_t m = solid_mask();
        int8_t nx = 0, ny = 0;

        // main collision corrections
        if(m & 1) ++nx, ++ny;
        if(m & 2) --nx, ++ny;
        if(m & 4) ++nx, --ny;
        if(m & 8) --nx, --ny;

        // corrections for running diagonally into an extruding corner
        if(m == 1 && pdir == 3)
        {
            if(check_solid(px + 5, py + 6)) ny = 0;
            else nx = 0;
        }
        if(m == 2 && pdir == 5)
        {
            if(check_solid(px + 11, py + 6)) ny = 0;
            else nx = 0;
        }
        if(m == 4 && pdir == 1)
        {
            if(check_solid(px + 5, py + 10)) ny = 0;
            else nx = 0;
        }
        if(m == 8 && pdir == 7)
        {
            if(check_solid(px + 11, py + 10)) ny = 0;
            else nx = 0;
        }

        // corrections for running diagonally through a tight gap
        if(m == 9)
        {
            nx = -dx, ny = -dy;
            if(uint8_t(pdir - 0) < 3) nx = -1, ny = +1;
            if(uint8_t(pdir - 4) < 3) nx = +1, ny = -1;
        }
        if(m == 6)
        {
            nx = -dx, ny = -dy;
            if(uint8_t(pdir - 2) < 3) nx = -1, ny = -1;
            if(uint8_t(pdir - 1) > 4) nx = +1, ny = +1;
        }

        // limit total movement to one pixel
        if(nx > 1) nx = 1;
        if(nx < -1) nx = -1;
        if(ny > 1) ny = 1;
        if(ny < -1) ny = -1;
        if(nx == dx) nx = 0;
        if(ny == dy) ny = 0;

        // collision correction
        px += nx;
        py += ny;

#if 1
        m = solid_mask();
        if(m != 0 && m != 9 && m != 6)
            px = prevx, py = prevy;
#endif
    }

    // update explored
#if 1
    {
        constexpr uint8_t D = 63; // allows adiw/sbiw
        uint16_t tpx = px;
        uint16_t tpy = py;
        check_explored(tpx - D, tpy - D);
        check_explored(tpx + D, tpy - D);
        check_explored(tpx - D, tpy + D);
        check_explored(tpx + D, tpy + D);
    }
#else
    {
        uint8_t ex = px / (EXPLORED_TILES * 16);
        uint8_t ey = py / (EXPLORED_TILES * 16);
        uint16_t n = ey * EXPLORED_COLS + ex;
        uint8_t i = n >> 3;
        uint8_t m = bitmask((uint8_t)n);
        MY_ASSERT(i < EXPLORED_BYTES);
        savefile.explored[i] |= m;
    }
#endif

    load_chunks();
    run_chunks();

    // nframe update
    ++savefile.chunk_regs[4];
}

static void skip_dialog_animation(uint8_t third_newline)
{
    auto& d = sdata.dialog;
    uint8_t i;
    char c;
    for(i = 0; (c = d.message[i]) != '\0' && c != '|' && i != third_newline; ++i)
    {}
    d.char_progress = i;
}

static void advance_dialog_animation()
{
    auto& d = sdata.dialog;
    uint8_t char_progress = d.char_progress;
    for(uint8_t i = 0; i < 1; ++i)
    {
        char c = d.message[char_progress];
        if(c != '\0') ++char_progress;
        if(c == '|')
        {
            MY_ASSERT(!d.question);
            // replace pipe char
            d.message[char_progress - 1] = '\0';
            // skip following newline
            MY_ASSERT(d.message[char_progress] == '\n');
            d.question_msg = ++char_progress;
            d.question = true;
            // count number of questions
            uint8_t n = 0;
            for(uint8_t j = char_progress; (c = d.message[j]) != '\0'; ++j)
                if(c == '\n') ++n;
            d.numquestions = n;
            MY_ASSERT(n + 1 <= 3);
            goto done;
        }
    }
done:
    d.char_progress = char_progress;
}

static void update_dialog()
{
    auto& d = sdata.dialog;
    if(d.char_progress == 0 && (uint8_t)d.name[0] != 254)
        platform_audio_play_sfx(SFX_CHATTER);
    uint8_t third_newline = 255;
    {
        uint8_t n = 0;
        for(uint8_t i = 0; i < sizeof(d.message) && d.message[i] != '\0'; ++i)
        {
            if(d.message[i] == '\n' && ++n == 3)
            {
                third_newline = i + 1;
                break;
            }
        }
    }
    char current_char = d.message[d.char_progress];
    bool message_done = (current_char == '\0');
    if(message_done)
        platform_audio_stop_sfx();
    if(d.question)
    {
        adjust(d.questioniy, d.questioni * 11);
        if(!d.questiondraw)
        {
            if(btns_pressed & BTN_A)
                d.questiondraw = true;
        }
        else if(message_done)
        {
            d.questiondone = true;
            uint8_t questionfill = d.questionfill;
            if(d.questionpause)
            {
                if(++questionfill >= 48)
                {
                    savefile.chunk_regs[1] = 0;
                    savefile.chunk_regs[2] = 0;
                    savefile.chunk_regs[3] = 0;
                    savefile.chunk_regs[d.questioni + 1] = 1;
                    back_to_map();
                }
            }
            else if(btns_down & BTN_A)
            {
                if(questionfill >= 32)
                    d.questionpause = true;
                else
                    ++questionfill;
            }
            else
            {
                questionfill = 0;
                uint8_t questioni = d.questioni;
                if(btns_pressed & BTN_UP)
                {
                    if(questioni > 0)
                        --questioni;
                }
                else if(btns_pressed & BTN_DOWN)
                {
                    if(questioni < d.numquestions)
                        ++questioni;
                }
                d.questioni = questioni;
            }
            uint8_t t = (questionfill >= 32 ? 255 : questionfill * 8);
            d.questionfill = questionfill;
            adjust(d.questionfillw, t);
        }
        else
        {
            if(btns_down & BTN_B)
                skip_dialog_animation(255);
            advance_dialog_animation();
        }
        return;
    }
    if(btns_down & BTN_B) skip_dialog_animation(third_newline);
    if(btns_pressed & BTN_A)
    {
        if(d.char_progress == third_newline)
        {
#ifdef ARDUINO
            strcpy(&d.message[0], &d.message[third_newline]);
#else
            for(uint8_t i = 0;; ++i)
            {
                d.message[i] = d.message[i + third_newline];
                if(d.message[i] == '\0') break;
            }
#endif
            d.char_progress = 0;
        }
        else if(message_done)
        {
            if(!d.question)
                back_to_map();
        }
    }
    else
    {
        advance_dialog_animation();
        if(d.char_progress > third_newline) d.char_progress = third_newline;
    }
}

static void update_tp()
{
    auto& d = sdata.tp;
    uint8_t frame = ++d.frame;
    if(frame == TELEPORT_TRANSITION_FRAMES)
    {
        px = d.tx * 16;
        py = d.ty * 16;
        // terminate old chunk scripts
        chunks_are_running = false;
        load_chunks();
        run_chunks();
    }
    if(frame == TELEPORT_TRANSITION_FRAMES * 2)
    {
        change_state(STATE_MAP);
        set_music_from_position();
    }
}

static void update_game_over()
{
    auto& d = sdata.game_over;
    if(d.going_to_resume)
    {
        if(d.fade_frame < 24)
            d.fade_frame += FADE_SPEED;
        else
        change_state(STATE_TITLE);
    }
    else
    {
        if(d.fade_frame == 0)
        {
            set_music(music::defeat);
            uint8_t n = u8rand(NUM_GAME_OVER_MESSAGES);
            platform_fx_read_data_bytes(
                GAME_OVER_MESSAGES + n * GAME_OVER_MESSAGE_LEN, d.msg,
                GAME_OVER_MESSAGE_LEN);
            // TODO: bake this logic into data with scripts
            d.msg_lines = 1;
            for(auto& c : d.msg)
            {
                if(c == '\0') break;
                if(c == '\n') ++d.msg_lines, c = '\0';
            }
        }
        if(d.fade_frame < 24)
            d.fade_frame += FADE_SPEED;
        else if(btns_pressed & (BTN_A | BTN_B))
            d.fade_frame = 8, d.going_to_resume = true;
    }
}

#if RECORD_LIPO_DISCHARGE
#include <hardwareSerial.h>
#endif

static uint8_t const TITLE_PATH[] PROGMEM =
{
    8, 64, 7, 48, 0, 60, 8, 60, 0, 20, 8, 30, 2, 64, 4, 8, 8, 40, 7, 20,
    6, 56, 0, 4, 8, 64, 6, 80, 4, 8, 8, 64, 6, 24, 5, 16, 4, 64, 3, 32,
    2, 100, 3, 24, 2, 24,
};
constexpr uint16_t PATH_START_X = 930;
constexpr uint16_t PATH_START_Y = 755;

static void title_advance_path()
{
    auto& d = sdata.title;
    uint8_t pi = d.path_index;
    if(pi >= (uint8_t)sizeof(TITLE_PATH))
    {
        pi = 0;
        px = PATH_START_X;
        py = PATH_START_Y;
    }
    uint8_t const* ptr = &TITLE_PATH[pi];
    uint8_t dir = pgm_read_byte_inc(ptr);
    uint8_t frames = pgm_read_byte(ptr);
    d.path_index = pi + 2;
    d.path_dir = dir;
    d.path_frames = frames;
}

static void title_animation()
{
    auto& d = sdata.title;
    if(!d.path_started)
    {
        d.path_started = true;
        pdir = 0;
        px = PATH_START_X;
        py = PATH_START_Y;
        title_advance_path();
    }
    else if(--d.path_frames == 0)
        title_advance_path();
    uint8_t dir = d.path_dir;
    if(dir < 8) pdir = dir;
    pmoving = (dir < 8);
    int8_t const* ptr = &DIRS[dir * 2];
    int8_t dx = (int8_t)pgm_read_byte_inc(ptr);
    int8_t dy = (int8_t)pgm_read_byte(ptr);
    px += dx;
    py += dy;

    load_chunks();
    run_chunks();
    update_sprites();

}

static void update_title()
{
    static bool first_loaded = false;
    auto& d = sdata.title;

#if DETECT_FX_CHIP
    if(!FX::detect())
    {
        d.no_fx_chip = true;
        if(btns_pressed & (BTN_A | BTN_B))
            a.exitToBootloader();
        return;
    }
#endif

    if(d.going_to_resume)
    {
        if(d.fade_frame < 16)
            d.fade_frame += FADE_SPEED;
        else
        {
            load(false);
            play_music();
            change_state(STATE_RESUME);
            return;
        }
    }
    else
    {
        set_music(music::title);
        if(d.fade_frame == 0)
        {
            if(!first_loaded)
            {
                load(true);
                first_loaded = true;
            }
            new_game();
        }
        if(d.fade_frame < 24)
            d.fade_frame += FADE_SPEED;
        else
        {
#if !TITLE_ONLY_DEMO
            if(btns_pressed & BTN_A)
            {
                d.fade_frame = 0;
                d.going_to_resume = true;
            }
#endif
            if(btns_pressed & BTN_B)
                platform_audio_toggle();
        }
    }

    title_animation();
}

static void update_resume()
{
    auto& d = sdata.resume;
    if(d.fade_frame == 0)
    {
        load_chunks();
        run_chunks();
    }
    if(d.fade_frame < 24)
        d.fade_frame += FADE_SPEED;
    else
    {
        change_state(STATE_MAP);
        set_music_from_position();
    }
}

static void update_die()
{
    auto& d = sdata.die;
    if(d.frame >= 48 + 16 / FADE_SPEED)
        change_state(STATE_GAME_OVER);
    else
        ++d.frame;
}

static inline void process_repeat(uint8_t i, uint8_t btn)
{
    constexpr uint8_t REP_INIT = 16;
    constexpr uint8_t REP_DELAY = 8;
    static uint8_t reps[4];
    uint8_t* r = &reps[i];
    if(!(btns_down & btn))
    {
        *r = 0;
        return;
    }
    uint8_t c = *r;
    uint8_t d = (c & 0x80) ? REP_DELAY + 0x80 : REP_INIT;
    if(c >= d)
        c = 0x80, btns_pressed |= btn;
    else
        ++c;
    *r = c;
}

static void update_badfx()
{
}

void update()
{
    using update_func = void (*)();
    static update_func const FUNCS[] PROGMEM = {
        update_title,
        update_resume,
        update_map,
        update_pause,
        update_dialog,
        update_tp,
        update_battle,
        update_die,
        update_game_over,
        update_badfx,
    };

    // try to play music if no song is playing
    //if(!platform_audio_song_playing() && (savefile.settings.sound & 2))
    //{
    //    uint24_t song = SONG_PEACEFUL;
    //
    //    // decide which song to play
    //    if(state == STATE_GAME_OVER)
    //        song = SONG_DEFEAT;
    //
    //    platform_audio_play_song(song);
    //}

    // arrow button repeat logic
    static_assert(BTN_DOWN  == 0x10, "");
    static_assert(BTN_LEFT  == 0x20, "");
    static_assert(BTN_RIGHT == 0x40, "");
    static_assert(BTN_UP    == 0x80, "");
    for(uint8_t b = 0x10, i = 0; b != 0; b <<= 1, ++i)
        process_repeat(i, b);
    //process_repeat(0, BTN_UP   );
    //process_repeat(1, BTN_DOWN );
    //process_repeat(2, BTN_LEFT );
    //process_repeat(3, BTN_RIGHT);

    pmoving = false;
    (pgmptr(&FUNCS[state]))();

    ++nframe;

#if DEBUG_LIPO_DISCHARGE
    if((nframe & 0x7ff) == 0x7ff)
    {
        uint8_t fade = u8rand() & 15;
        if(fade == 0) fade = 1;
        platform_fade(fade);
    }
#endif
#if RECORD_LIPO_DISCHARGE
    if(state == STATE_TITLE) && (btns_pressed & BTN_B))
    {
        int16_t v[10];
        for(uint24_t i = 0; i < 7200 * 2; i += 2)
        {
            FX::readSaveBytes(i, (uint8_t*)v, 2);
            for(uint8_t i = 0; i < 1; ++i)
            {
                Serial.print(v[i]);
                Serial.print(' ');
            }
            Serial.print('\n');
        }
    }
#endif
}
