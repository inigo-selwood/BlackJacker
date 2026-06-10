#include "graphics/graphics.h"

#include <curses.h>
#include <string.h>

static int
alignedOffset(int available, int size, Graphics_Alignment alignment) {
    if(alignment == GRAPHICS_ALIGN_CENTER) {
        return (available - size) / 2;
    }

    if(alignment == GRAPHICS_ALIGN_END) {
        return available - size;
    }

    return 0;
}

Graphics_Label Graphics_label(
    const char *text,
    Graphics_Alignment alignment,
    bool bold,
    bool inverted
) {
    return (Graphics_Label){
        .text = text,
        .box = {0, 0, text ? (int)strlen(text) : 0, 1},
        .width = text ? (int)strlen(text) : 0,
        .height = 1,
        .alignment = alignment,
        .bold = bold,
        .inverted = inverted,
    };
}

void Graphics_positionLabelWithin(
    Graphics_Label *label,
    Graphics_Box parent,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
) {
    label->box = Graphics_positionWithin(
        parent,
        (Graphics_Size){label->width, label->height},
        horizontal,
        vertical
    );
}

static void drawLabelAttrs(Graphics_Label label) {
    if(label.bold) {
        attron(A_BOLD);
    }

    if(label.inverted) {
        attron(A_REVERSE);
    }
}

static void clearLabelAttrs(Graphics_Label label) {
    if(label.inverted) {
        attroff(A_REVERSE);
    }

    if(label.bold) {
        attroff(A_BOLD);
    }
}

void Graphics_drawLabel(Graphics_Label label) {
    const int textWidth = label.text ? (int)strlen(label.text) : 0;
    const int visibleWidth =
        textWidth > label.box.width ? label.box.width : textWidth;
    const int x = label.box.x
        + alignedOffset(label.box.width, visibleWidth, label.alignment);
    const int y = label.box.y + label.box.height / 2;

    drawLabelAttrs(label);
    mvaddnstr(y, x, label.text ? label.text : "", visibleWidth);
    clearLabelAttrs(label);
}

static int wordLength(const char *text) {
    int length = 0;

    while(text[length] && text[length] != ' ') {
        length += 1;
    }

    return length;
}

void Graphics_drawWrappedLabel(Graphics_Label label) {
    const char *text = label.text ? label.text : "";
    int row = 0;

    drawLabelAttrs(label);

    while(*text && row < label.box.height) {
        int col = 0;

        while(*text == ' ') {
            text += 1;
        }

        while(*text && col < label.box.width) {
            const int length = wordLength(text);
            const bool needsSpace = col > 0;

            if(needsSpace && col + 1 + length > label.box.width) {
                break;
            }

            if(needsSpace) {
                mvaddch(label.box.y + row, label.box.x + col, ' ');
                col += 1;
            }

            mvaddnstr(label.box.y + row, label.box.x + col, text, length);
            col += length;
            text += length;

            while(*text == ' ') {
                text += 1;
            }
        }

        row += 1;
    }

    clearLabelAttrs(label);
}
