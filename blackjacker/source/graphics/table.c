#include "graphics/graphics.h"

#include <curses.h>
#include <string.h>

static void drawTextCentered(Graphics_Box box, const char *text) {
    const int textWidth = text ? (int)strlen(text) : 0;
    const int visibleWidth = textWidth > box.width ? box.width : textWidth;
    const int x = box.x + (box.width - visibleWidth) / 2;

    if(!text || box.width <= 0) {
        return;
    }

    mvaddnstr(box.y, x, text, visibleWidth);
}

static int cellAttr(Graphics_Table table, int row, int column) {
    if(!table.cellAttrs) {
        return 0;
    }

    return table.cellAttrs[row * table.columnCount + column];
}

static void drawHorizontalLine(
    Graphics_Table table,
    int y,
    const char *left,
    const char *join,
    const char *right
) {
    mvaddstr(y, table.box.x, left);

    for(int col = 0; col < table.rowLabelWidth; col += 1) {
        mvaddstr(y, table.box.x + 1 + col, "─");
    }

    mvaddstr(y, table.box.x + 1 + table.rowLabelWidth, join);

    for(int column = 0; column < table.columnCount; column += 1) {
        const int x = table.box.x + 2 + table.rowLabelWidth
            + column * (table.cellWidth + 1);

        for(int col = 0; col < table.cellWidth; col += 1) {
            mvaddstr(y, x + col, "─");
        }

        mvaddstr(
            y,
            x + table.cellWidth,
            column + 1 == table.columnCount ? right : join
        );
    }
}

Graphics_Size Graphics_tableSize(Graphics_Table table) {
    return (Graphics_Size){
        table.rowLabelWidth + 2 + table.columnCount * (table.cellWidth + 1),
        table.rowCount + 4,
    };
}

void Graphics_drawTable(Graphics_Table table) {
    const int headerY = table.box.y + 1;
    const int dividerY = table.box.y + 2;

    if(table.columnCount <= 0 || table.rowCount <= 0) {
        return;
    }

    drawHorizontalLine(table, table.box.y, "╭", "┬", "╮");
    mvaddstr(headerY, table.box.x, "│");
    drawTextCentered(
        (Graphics_Box){
            table.box.x + 1,
            headerY,
            table.rowLabelWidth,
            1,
        },
        ""
    );
    mvaddstr(headerY, table.box.x + table.rowLabelWidth + 1, "│");

    for(int column = 0; column < table.columnCount; column += 1) {
        const int x = table.box.x + 2 + table.rowLabelWidth
            + column * (table.cellWidth + 1);

        drawTextCentered(
            (Graphics_Box){x, headerY, table.cellWidth, 1},
            table.columns[column]
        );
        mvaddstr(headerY, x + table.cellWidth, "│");
    }

    drawHorizontalLine(table, dividerY, "├", "┼", "┤");

    for(int row = 0; row < table.rowCount; row += 1) {
        const int y = table.box.y + 3 + row;

        mvaddstr(y, table.box.x, "│");
        drawTextCentered(
            (Graphics_Box){table.box.x + 1, y, table.rowLabelWidth, 1},
            table.rows[row]
        );
        mvaddstr(y, table.box.x + table.rowLabelWidth + 1, "│");

        for(int column = 0; column < table.columnCount; column += 1) {
            const int x = table.box.x + 2 + table.rowLabelWidth
                + column * (table.cellWidth + 1);
            const char *cell = table.cells[row * table.columnCount + column];
            const int attr = cellAttr(table, row, column);

            if(attr) {
                attron(attr);
                Graphics_drawBox((Graphics_Box){x, y, table.cellWidth, 1});
            }

            drawTextCentered(
                (Graphics_Box){x, y, table.cellWidth, 1},
                cell ? cell : ""
            );

            if(attr) {
                attroff(attr);
            }

            mvaddstr(y, x + table.cellWidth, "│");
        }
    }

    drawHorizontalLine(table, table.box.y + table.rowCount + 3, "╰", "┴", "╯");
}
