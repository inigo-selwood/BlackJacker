#include "render.h"

#include <curses.h>
#include <stdio.h>

#include "graphics/graphics.h"
#include "modes/play/controls.h"
#include "modes/play/game.h"
#include "table/table.h"
#include "utils/render_card.h"

enum {
    CARD_WIDTH = 9,
    CARD_HEIGHT = 5,
    CARD_GAP = 2,
    HAND_ZONE_MIN_CARDS = 3,
    HAND_ZONE_HEIGHT = 7,
    HAND_ZONE_GAP = 2,
};

static const char *resultLabel(RoundResult result) {
    switch(result) {
    case ROUND_RESULT_PLAYER_BUST:
        return "Bust";
    case ROUND_RESULT_DEALER_BUST:
    case ROUND_RESULT_PLAYER_WIN:
        return "Win";
    case ROUND_RESULT_DEALER_WIN:
        return "Lose";
    case ROUND_RESULT_PUSH:
        return "Push";
    case ROUND_RESULT_SURRENDER:
        return "Surrender";
    case ROUND_RESULT_NONE:
    default:
        return "Playing";
    }
}

static void drawHorizontalBorder(
    Graphics_Box rect,
    int y,
    const char *left,
    const char *right
) {
    mvaddstr(y, rect.x, left);

    for(int col = 1; col < rect.width - 1; col += 1) {
        mvaddstr(y, rect.x + col, "─");
    }

    mvaddstr(y, rect.x + rect.width - 1, right);
}

static void drawZone(Graphics_Box rect, const char *label, bool active) {
    const Graphics_Box content =
        Graphics_inset(rect, (Graphics_Padding){1, 1, 1, 1});

    if(active) {
        attron(A_BOLD);
    }

    drawHorizontalBorder(rect, rect.y, "╭", "╮");

    for(int row = 1; row < rect.height - 1; row += 1) {
        mvaddstr(rect.y + row, rect.x, "│");
        mvaddstr(rect.y + row, rect.x + rect.width - 1, "│");
    }

    drawHorizontalBorder(rect, rect.y + rect.height - 1, "╰", "╯");

    if(label) {
        mvaddnstr(rect.y, rect.x + 2, label, rect.width - 4);
    }

    if(active) {
        attroff(A_BOLD);
    }

    Graphics_drawBox(content);
}

static void drawHand(Graphics_Box rect, Table_Hand hand, bool hideSecondCard) {
    const int cardStride = CARD_WIDTH + CARD_GAP;
    const int y = rect.y + (rect.height - CARD_HEIGHT) / 2;

    for(int index = 0; index < hand.count; index += 1) {
        drawCard(
            rect.x + index * cardStride,
            y,
            hand.cards[index],
            !(hideSecondCard && index == 1),
            false
        );
    }
}

static int cardRowWidth(int cardCount) {
    if(cardCount <= 0) {
        return 0;
    }

    return cardCount * CARD_WIDTH + (cardCount - 1) * CARD_GAP;
}

static int handZoneWidth(int cardCount) {
    const int capacity =
        cardCount > HAND_ZONE_MIN_CARDS ? cardCount : HAND_ZONE_MIN_CARDS;

    return cardRowWidth(capacity) + 4;
}

static int playerRowWidth(PlayRound *round) {
    int width = (round->playerHandCount - 1) * HAND_ZONE_GAP;

    for(int index = 0; index < round->playerHandCount; index += 1) {
        width += handZoneWidth(round->playerHands[index].cards.count);
    }

    return width;
}

static void playerZoneLabel(
    Runtime_Game *game,
    int handIndex,
    char *label,
    int labelSize
) {
    PlayRound *round = &game->playRound;
    const RoundResult result = playerHandResult(game, handIndex);

    if(round->playerHandCount == 1) {
        if(round->phase == ROUND_COMPLETE || result != ROUND_RESULT_NONE) {
            snprintf(label, labelSize, " Player - %s ", resultLabel(result));
            return;
        }

        snprintf(label, labelSize, " Player ");
        return;
    }

    if(round->phase == ROUND_COMPLETE || result != ROUND_RESULT_NONE) {
        snprintf(
            label,
            labelSize,
            " Hand %d - %s ",
            handIndex + 1,
            resultLabel(result)
        );
        return;
    }

    snprintf(label, labelSize, " Hand %d ", handIndex + 1);
}

static void drawDealerRow(Graphics_Box rect, Runtime_Game *game) {
    PlayRound *round = &game->playRound;
    const bool hideDealerCard = round->phase != ROUND_COMPLETE;
    const Graphics_Box zone = Graphics_positionWithin(
        rect,
        (Graphics_Size){handZoneWidth(round->dealerHand.count),
            HAND_ZONE_HEIGHT},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );

    drawZone(zone, " Dealer ", false);
    drawHand(
        Graphics_inset(zone, (Graphics_Padding){1, 2, 1, 2}),
        round->dealerHand,
        hideDealerCard
    );
}

static void drawPlayerRow(Graphics_Box rect, Runtime_Game *game) {
    PlayRound *round = &game->playRound;
    const int totalWidth = playerRowWidth(round);
    const Graphics_Box row = Graphics_positionWithin(
        rect,
        (Graphics_Size){totalWidth, HAND_ZONE_HEIGHT},
        totalWidth > rect.width ? GRAPHICS_ALIGN_START : GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    int x = row.x;

    for(int index = 0; index < round->playerHandCount; index += 1) {
        char label[40];
        const Graphics_Box zone = {
            x,
            row.y,
            handZoneWidth(round->playerHands[index].cards.count),
            HAND_ZONE_HEIGHT,
        };
        const bool active = round->phase == ROUND_PLAYER_TURN
            && index == round->activePlayerHand;

        playerZoneLabel(game, index, label, sizeof(label));
        drawZone(zone, label, active);
        drawHand(
            Graphics_inset(zone, (Graphics_Padding){1, 2, 1, 2}),
            round->playerHands[index].cards,
            false
        );

        x += zone.width + HAND_ZONE_GAP;
    }
}

static void drawStatus(Graphics_Box rect, Runtime_Game *game) {
    char text[96];
    Graphics_Label label;
    PlayRound *round = &game->playRound;
    Table_Hand *hand = activePlayerHand(game);
    const int playerValue = hand ? Table_handValue(*hand) : 0;
    const int dealerShowing = round->dealerHand.count > 0
        ? Table_handValue((Table_Hand){{round->dealerHand.cards[0]}, 1})
        : 0;
    const int shoeRemaining = Table_shoeCardsUntilCut(game->shoe);

    if(round->phase == ROUND_COMPLETE) {
        snprintf(
            text,
            sizeof(text),
            "Round %d  |  Result %s  |  Player %d  Dealer %d  |  Shoe %d",
            round->roundNumber,
            resultLabel(playerHandResult(game, round->activePlayerHand)),
            playerValue,
            Table_handValue(round->dealerHand),
            shoeRemaining
        );
    } else {
        snprintf(
            text,
            sizeof(text),
            "Round %d  |  Hand %d/%d  |  Player %d  Dealer %d  |  Shoe %d",
            round->roundNumber,
            round->activePlayerHand + 1,
            round->playerHandCount,
            playerValue,
            dealerShowing,
            shoeRemaining
        );
    }

    label = Graphics_label(text, GRAPHICS_ALIGN_CENTER, false, false);
    Graphics_positionLabelWithin(
        &label,
        rect,
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_drawLabel(label);
}

void drawPlay(Runtime_Game *game) {
    const Graphics_Box screen = Graphics_screenBox();
    const int playerWidth = playerRowWidth(&game->playRound);
    int panelWidth = 76;

    if(playerWidth > panelWidth) {
        panelWidth = playerWidth;
    }

    if(panelWidth > screen.width) {
        panelWidth = screen.width;
    }

    const Graphics_Box panel = Graphics_positionWithin(
        screen,
        (Graphics_Size){panelWidth, 22},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_Box sections[9] = {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, HAND_ZONE_HEIGHT},
        {0, 0, 0, 1},
        {0, 0, 0, HAND_ZONE_HEIGHT},
        {0, 0, 0, 1},
        {0, 0, 0, 3},
    };
    Graphics_ControlGroup controls;
    Graphics_Label title =
        Graphics_label("TRAIN", GRAPHICS_ALIGN_CENTER, true, false);

    Graphics_arrangeWithin(panel, GRAPHICS_DIRECTION_COLUMN, 0, sections, 9);
    controls = playControls(game, sections[8]);

    Graphics_positionLabelWithin(
        &title,
        sections[0],
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );

    Graphics_drawLabel(title);
    drawStatus(sections[2], game);
    drawDealerRow(sections[4], game);
    drawPlayerRow(sections[6], game);
    Graphics_drawControls(controls);
}
