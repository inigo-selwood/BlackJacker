#include "graphics/graphics.h"

#include <curses.h>

static int clampedSize(int value) {
    return value < 0 ? 0 : value;
}

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

Graphics_Box Graphics_screenBox(void) {
    int height;
    int width;

    getmaxyx(stdscr, height, width);

    const Graphics_Box box = {0, 0, width, height};
    return box;
}

Graphics_Box Graphics_inset(Graphics_Box box, Graphics_Padding padding) {
    const Graphics_Box inset = {
        box.x + padding.left,
        box.y + padding.top,
        clampedSize(box.width - padding.left - padding.right),
        clampedSize(box.height - padding.top - padding.bottom),
    };

    return inset;
}

Graphics_Box Graphics_positionWithin(
    Graphics_Box parent,
    Graphics_Size size,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
) {
    const int width = horizontal == GRAPHICS_ALIGN_STRETCH
        ? parent.width
        : clampedSize(size.width);
    const int height = vertical == GRAPHICS_ALIGN_STRETCH
        ? parent.height
        : clampedSize(size.height);
    const Graphics_Box box = {
        parent.x + alignedOffset(parent.width, width, horizontal),
        parent.y + alignedOffset(parent.height, height, vertical),
        width,
        height,
    };

    return box;
}

static int childMainSize(Graphics_Box child, Graphics_Direction direction) {
    return direction == GRAPHICS_DIRECTION_ROW ? child.width : child.height;
}

static int childCrossSize(Graphics_Box child, Graphics_Direction direction) {
    return direction == GRAPHICS_DIRECTION_ROW ? child.height : child.width;
}

static void setChildBox(
    Graphics_Box *child,
    Graphics_Direction direction,
    int mainPosition,
    int mainSize,
    Graphics_Box parent
) {
    if(direction == GRAPHICS_DIRECTION_ROW) {
        *child = (Graphics_Box){
            mainPosition,
            parent.y,
            mainSize,
            child->height > 0 ? child->height : parent.height,
        };
        return;
    }

    *child = (Graphics_Box){
        parent.x,
        mainPosition,
        child->width > 0 ? child->width : parent.width,
        mainSize,
    };
}

void Graphics_arrangeWithin(
    Graphics_Box parent,
    Graphics_Direction direction,
    int gap,
    Graphics_Box *children,
    int count
) {
    const int parentMain =
        direction == GRAPHICS_DIRECTION_ROW ? parent.width : parent.height;
    const int start =
        direction == GRAPHICS_DIRECTION_ROW ? parent.x : parent.y;
    const int totalGap = count > 0 ? gap * (count - 1) : 0;
    int fixedSize = 0;
    int flexibleCount = 0;
    int flexibleSize;
    int cursor = start;

    if(count <= 0) {
        return;
    }

    for(int index = 0; index < count; index += 1) {
        const int size = childMainSize(children[index], direction);

        if(size > 0) {
            fixedSize += size;
        } else {
            flexibleCount += 1;
        }
    }

    flexibleSize = flexibleCount > 0
        ? clampedSize((parentMain - fixedSize - totalGap) / flexibleCount)
        : 0;

    for(int index = 0; index < count; index += 1) {
        const int requestedSize = childMainSize(children[index], direction);
        const int mainSize = requestedSize > 0 ? requestedSize : flexibleSize;
        const int crossSize = childCrossSize(children[index], direction);

        if(crossSize <= 0) {
            setChildBox(&children[index], direction, cursor, mainSize, parent);
        } else if(direction == GRAPHICS_DIRECTION_ROW) {
            children[index] = Graphics_positionWithin(
                (Graphics_Box){cursor, parent.y, mainSize, parent.height},
                (Graphics_Size){mainSize, crossSize},
                GRAPHICS_ALIGN_START,
                GRAPHICS_ALIGN_CENTER
            );
        } else {
            children[index] = Graphics_positionWithin(
                (Graphics_Box){parent.x, cursor, parent.width, mainSize},
                (Graphics_Size){crossSize, mainSize},
                GRAPHICS_ALIGN_CENTER,
                GRAPHICS_ALIGN_START
            );
        }

        cursor += mainSize + gap;
    }
}

void Graphics_drawBox(Graphics_Box box) {
    for(int row = 0; row < box.height; row += 1) {
        for(int col = 0; col < box.width; col += 1) {
            mvaddch(box.y + row, box.x + col, ' ');
        }
    }
}

void Graphics_drawHorizontalDivider(Graphics_Box box) {
    for(int col = 0; col < box.width; col += 1) {
        mvaddstr(box.y, box.x + col, "─");
    }
}
