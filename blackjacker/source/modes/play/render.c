#include "render.h"

#include <curses.h>

#include "graphics/graphics.h"

static void drawHorizontalBorder(
    Graphics_Box box,
    int y,
    const char *left,
    const char *right
) {
    mvaddstr(y, box.x, left);

    for(int col = 1; col < box.width - 1; col += 1) {
        mvaddstr(y, box.x + col, "─");
    }

    mvaddstr(y, box.x + box.width - 1, right);
}

static void drawZone(Graphics_Box box, const char *label) {
    drawHorizontalBorder(box, box.y, "╭", "╮");

    for(int row = 1; row < box.height - 1; row += 1) {
        mvaddstr(box.y + row, box.x, "│");
        mvaddstr(box.y + row, box.x + box.width - 1, "│");
    }

    drawHorizontalBorder(box, box.y + box.height - 1, "╰", "╯");
    mvaddnstr(box.y, box.x + 2, label, box.width - 4);
}

void drawPlay(Runtime_Game *game) {
    (void)game;

    const Graphics_Box content = Graphics_minimumContentBox();
    const Graphics_Box table =
        Graphics_inset(content, (Graphics_Padding){3, 2, 5, 2});
    Graphics_Box rows[2] = {
        {0, 0, 0, 8},
        {0, 0, 0, 8},
    };

    Graphics_arrangeWithin(table, GRAPHICS_DIRECTION_COLUMN, 2, rows, 2);

    drawZone(
        Graphics_positionWithin(
            rows[0],
            (Graphics_Size){52, 8},
            GRAPHICS_ALIGN_CENTER,
            GRAPHICS_ALIGN_CENTER
        ),
        " Dealer "
    );
    drawZone(
        Graphics_positionWithin(
            rows[1],
            (Graphics_Size){52, 8},
            GRAPHICS_ALIGN_CENTER,
            GRAPHICS_ALIGN_CENTER
        ),
        " Player "
    );
}
