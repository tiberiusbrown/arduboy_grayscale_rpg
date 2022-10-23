#include "common.hpp"

#include "generated/portrait_img.hpp"

void render()
{
    draw_tiles();
    draw_player();

    // static char const STR[] PROGMEM =
    //     "But what if the guard sees us? He'll\n"
    //     "attack! And then I'll die a horrible\n"
    //     "and painful death...";
    // platform_fillrect(0, 2, 33, 33, BLACK);
    // platform_fillrect(0, 35, 128, 28, BLACK);
    // draw_text(0, 37, STR);
    // platform_drawoverwrite(0, 3, PORTRAIT_IMG, 0);
}
