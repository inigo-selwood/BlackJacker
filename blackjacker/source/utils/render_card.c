#include "render_card.h"
#include "terminal/terminal.h"

#ifdef A_ITALIC
#define CARD_ITALIC_ATTR A_ITALIC
#else
#define CARD_ITALIC_ATTR 0
#endif

enum { CARD_WIDTH = 9, CARD_HEIGHT = 5 };
enum { MINI_CARD_WIDTH = 5, MINI_CARD_HEIGHT = 3 };

typedef enum {
    CARD_STYLE_BACK,
    CARD_STYLE_FACE,
    CARD_STYLE_FACE_BOLD,
    CARD_STYLE_FACE_ITALIC
} CardStyle;

static attr_t styleAttrs(CardStyle style) {
    switch(style) {
    case CARD_STYLE_BACK:
        return Terminal_cardBackAttrs();
    case CARD_STYLE_FACE_BOLD:
        return A_REVERSE | A_BOLD;
    case CARD_STYLE_FACE_ITALIC:
        return A_REVERSE | CARD_ITALIC_ATTR;
    case CARD_STYLE_FACE:
    default:
        return A_REVERSE;
    }
}

static void addStyledText(int y, int x, const char *text, CardStyle style) {
    const attr_t attrs = styleAttrs(style);

    attron(attrs);
    mvaddstr(y, x, text);
    attroff(attrs);
}

static void drawCardRows(int x, int y, CardStyle style) {
    for(int row = 0; row < CARD_HEIGHT; row += 1) {
        addStyledText(y + row, x, "         ", style);
    }
}

static void drawStandardCard(int x, int y, Table_Card card, bool faceUp) {
    static const char *backRows[] = {"╳ ╳ ╳ ╳ ╳",
        " ╳ ╳ ╳ ╳ ",
        "╳ ╳ ╳ ╳ ╳",
        " ╳ ╳ ╳ ╳ ",
        "╳ ╳ ╳ ╳ ╳"};
    const char *rankText = Table_rankLabel(card.rank);
    const char *suitLetterText = Table_suitLetter(card.suit);
    const char *suitText = Table_suitSymbol(card.suit);
    const int rankX = card.rank == TABLE_RANK_TEN ? x + 3 : x + 4;

    if(!faceUp) {
        for(int row = 0; row < CARD_HEIGHT; row += 1) {
            addStyledText(y + row, x, backRows[row], CARD_STYLE_BACK);
        }
        return;
    }

    drawCardRows(x, y, CARD_STYLE_FACE);
    addStyledText(y, x + 1, suitLetterText, CARD_STYLE_FACE_ITALIC);
    addStyledText(y, x + CARD_WIDTH - 2, suitText, CARD_STYLE_FACE);
    addStyledText(y + 2, rankX, rankText, CARD_STYLE_FACE_BOLD);
    addStyledText(y + CARD_HEIGHT - 1, x + 1, suitText, CARD_STYLE_FACE);
    addStyledText(
        y + CARD_HEIGHT - 1,
        x + CARD_WIDTH - 2,
        suitLetterText,
        CARD_STYLE_FACE_ITALIC
    );
}

static void drawMiniCardRows(int x, int y, CardStyle style) {
    for(int row = 0; row < MINI_CARD_HEIGHT; row += 1) {
        addStyledText(y + row, x, "     ", style);
    }
}

static void drawMiniCard(int x, int y, Table_Card card, bool faceUp) {
    static const char *backRows[] = {"╳ ╳ ╳", " ╳ ╳ ", "╳ ╳ ╳"};
    const char *rankText = Table_rankLabel(card.rank);
    const char *suitText = Table_suitSymbol(card.suit);
    const int rankWidth = card.rank == TABLE_RANK_TEN ? 2 : 1;

    if(!faceUp) {
        for(int row = 0; row < MINI_CARD_HEIGHT; row += 1) {
            addStyledText(y + row, x, backRows[row], CARD_STYLE_BACK);
        }
        return;
    }

    drawMiniCardRows(x, y, CARD_STYLE_FACE);
    addStyledText(y, x, rankText, CARD_STYLE_FACE);
    addStyledText(y + 1, x + 2, suitText, CARD_STYLE_FACE_BOLD);
    addStyledText(
        y + MINI_CARD_HEIGHT - 1,
        x + MINI_CARD_WIDTH - rankWidth,
        rankText,
        CARD_STYLE_FACE
    );
}

void drawCard(int x, int y, Table_Card card, bool faceUp, bool miniature) {
    if(miniature) {
        drawMiniCard(x, y, card, faceUp);
        return;
    }

    drawStandardCard(x, y, card, faceUp);
}
