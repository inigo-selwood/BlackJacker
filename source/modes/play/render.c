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
    MINI_CARD_WIDTH = 5,
    MINI_CARD_HEIGHT = 3,
    CARD_GAP = 2,
    MINI_CARD_GAP = 1,
    OTHER_HAND_GAP = 3,
    OTHER_HAND_ZONE_HEIGHT = 5,
    TABLE_GROUP_GAP = 3,
    HAND_ZONE_MIN_CARDS = 3,
    HAND_ZONE_HEIGHT = 7,
    HAND_ZONE_GAP = 2
};

static const char *resultLabel(RoundResult result) {
    switch(result) {
    case ROUND_RESULT_PLAYER_BUST:
        return "Player Bust";
    case ROUND_RESULT_DEALER_BUST:
        return "Dealer Bust";
    case ROUND_RESULT_PLAYER_WIN:
        return "Player Wins";
    case ROUND_RESULT_DEALER_WIN:
        return "Dealer Wins";
    case ROUND_RESULT_PUSH:
        return "Push";
    case ROUND_RESULT_SURRENDER:
        return "Surrender";
    case ROUND_RESULT_NONE:
    default:
        return "Player Turn";
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

    if(hand.count <= 0) {
        return;
    }

    const int startX = rect.x;
    const int y = rect.y + (rect.height - CARD_HEIGHT) / 2;

    for(int index = 0; index < hand.count; index += 1) {
        drawCard(
            startX + index * cardStride,
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

static int miniCardRowWidth(int cardCount) {
    if(cardCount <= 0) {
        return 0;
    }

    return cardCount * MINI_CARD_WIDTH + (cardCount - 1) * MINI_CARD_GAP;
}

static int handZoneWidth(int cardCount) {
    const int capacity =
        cardCount > HAND_ZONE_MIN_CARDS ? cardCount : HAND_ZONE_MIN_CARDS;

    return cardRowWidth(capacity) + 4;
}

static int playerZonesWidth(PlayRound *round) {
    int totalWidth = (round->playerHandCount - 1) * HAND_ZONE_GAP;

    for(int index = 0; index < round->playerHandCount; index += 1) {
        totalWidth += handZoneWidth(round->playerHands[index].cards.count);
    }

    return totalWidth;
}

static int otherPlayerZoneWidth(PlayRound *round, int index) {
    return miniCardRowWidth(round->otherPlayerHands[index].cards.count) + 4;
}

static int otherPlayersWidth(PlayRound *round, int startIndex, int endIndex) {
    int totalWidth = 0;

    if(endIndex > round->otherPlayerHandCount) {
        endIndex = round->otherPlayerHandCount;
    }

    for(int index = startIndex; index < endIndex; index += 1) {
        if(totalWidth > 0) {
            totalWidth += OTHER_HAND_GAP;
        }

        totalWidth += otherPlayerZoneWidth(round, index);
    }

    return totalWidth;
}

static int tableRowWidth(PlayRound *round) {
    const int beforeEnd = PLAYER_TABLE_SEAT_INDEX < round->otherPlayerHandCount
        ? PLAYER_TABLE_SEAT_INDEX
        : round->otherPlayerHandCount;
    const int beforeWidth = otherPlayersWidth(round, 0, beforeEnd);
    const int playerWidth = playerZonesWidth(round);
    const int afterWidth = otherPlayersWidth(
        round,
        PLAYER_TABLE_SEAT_INDEX,
        round->otherPlayerHandCount
    );
    int totalWidth = 0;

    if(beforeWidth > 0) {
        totalWidth += beforeWidth;
    }

    if(playerWidth > 0) {
        if(totalWidth > 0) {
            totalWidth += TABLE_GROUP_GAP;
        }
        totalWidth += playerWidth;
    }

    if(afterWidth > 0) {
        if(totalWidth > 0) {
            totalWidth += TABLE_GROUP_GAP;
        }
        totalWidth += afterWidth;
    }

    return totalWidth;
}

static const char *otherPlayerResultLabel(RoundResult result) {
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

static void drawMiniHand(Graphics_Box rect, Table_Hand hand) {
    const int cardStride = MINI_CARD_WIDTH + MINI_CARD_GAP;
    const int width = miniCardRowWidth(hand.count);
    const Graphics_Box row = Graphics_positionWithin(
        rect,
        (Graphics_Size){width, MINI_CARD_HEIGHT},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );

    for(int index = 0; index < hand.count; index += 1) {
        drawCard(
            row.x + index * cardStride,
            row.y,
            hand.cards[index],
            true,
            true
        );
    }
}

static int drawOtherPlayers(
    Graphics_Box rect,
    Runtime_Game *game,
    int startIndex,
    int endIndex,
    int x
) {
    PlayRound *round = &game->playRound;
    const Graphics_Box row = Graphics_positionWithin(
        rect,
        (Graphics_Size){rect.width, OTHER_HAND_ZONE_HEIGHT},
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_CENTER
    );

    if(endIndex > round->otherPlayerHandCount) {
        endIndex = round->otherPlayerHandCount;
    }

    if(startIndex >= endIndex) {
        return x;
    }

    for(int index = startIndex; index < endIndex; index += 1) {
        char label[20];
        const int width = otherPlayerZoneWidth(round, index);
        const Graphics_Box zone = {x, row.y, width, OTHER_HAND_ZONE_HEIGHT};
        const Graphics_Box handRect =
            Graphics_inset(zone, (Graphics_Padding){1, 2, 1, 2});

        if(round->phase == ROUND_COMPLETE
            || round->otherPlayerHands[index].result != ROUND_RESULT_NONE) {
            snprintf(
                label,
                sizeof(label),
                " %s ",
                otherPlayerResultLabel(round->otherPlayerHands[index].result)
            );
        } else {
            snprintf(label, sizeof(label), " Seat %d ", index + 1);
        }

        drawZone(zone, label, false);
        drawMiniHand(handRect, round->otherPlayerHands[index].cards);
        x += width + OTHER_HAND_GAP;
    }

    return x - OTHER_HAND_GAP;
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

static int drawPlayerZones(Graphics_Box rect, Runtime_Game *game, int x) {
    PlayRound *round = &game->playRound;
    int zoneWidths[MAX_PLAYER_HANDS];

    for(int index = 0; index < round->playerHandCount; index += 1) {
        zoneWidths[index] =
            handZoneWidth(round->playerHands[index].cards.count);
    }

    const Graphics_Box zoneRow = Graphics_positionWithin(
        rect,
        (Graphics_Size){rect.width, HAND_ZONE_HEIGHT},
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_CENTER
    );

    for(int index = 0; index < round->playerHandCount; index += 1) {
        char label[40];
        const Graphics_Box zone = {
            x,
            zoneRow.y,
            zoneWidths[index],
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

        x += zoneWidths[index] + HAND_ZONE_GAP;
    }

    return x - HAND_ZONE_GAP;
}

static void drawTableRow(Graphics_Box rect, Runtime_Game *game) {
    PlayRound *round = &game->playRound;
    const int totalWidth = tableRowWidth(round);
    const Graphics_Box row = Graphics_positionWithin(
        rect,
        (Graphics_Size){totalWidth, HAND_ZONE_HEIGHT},
        totalWidth > rect.width ? GRAPHICS_ALIGN_START : GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    int x = row.x;
    const int beforeEnd = PLAYER_TABLE_SEAT_INDEX < round->otherPlayerHandCount
        ? PLAYER_TABLE_SEAT_INDEX
        : round->otherPlayerHandCount;

    if(beforeEnd > 0) {
        x = drawOtherPlayers(rect, game, 0, beforeEnd, x);
        x += TABLE_GROUP_GAP;
    }

    x = drawPlayerZones(rect, game, x);

    if(PLAYER_TABLE_SEAT_INDEX < round->otherPlayerHandCount) {
        x += TABLE_GROUP_GAP;
        drawOtherPlayers(
            rect,
            game,
            PLAYER_TABLE_SEAT_INDEX,
            round->otherPlayerHandCount,
            x
        );
    }
}

static void drawStatus(Graphics_Box rect, Runtime_Game *game) {
    char text[80];
    Graphics_Label label;
    const PlayRound *round = &game->playRound;
    Table_Hand *hand = activePlayerHand(game);
    const int playerValue = hand ? Table_handValue(*hand) : 0;
    const int shoeRemaining = Table_shoeCardsUntilCut(game->shoe);

    if(round->phase == ROUND_COMPLETE) {
        snprintf(
            text,
            sizeof(text),
            "Round %d  |  H%d %s  |  Player %d  Dealer %d  |  Shoe %d",
            round->roundNumber,
            round->activePlayerHand + 1,
            resultLabel(playerHandResult(game, round->activePlayerHand)),
            playerValue,
            Table_handValue(round->dealerHand),
            shoeRemaining
        );
    } else {
        snprintf(
            text,
            sizeof(text),
            "Round %d  |  Hand %d/%d  |  Player %d  |  Dealer showing "
            "%d  |  "
            "Shoe %d",
            round->roundNumber,
            round->activePlayerHand + 1,
            round->playerHandCount,
            playerValue,
            Table_handValue((Table_Hand){{round->dealerHand.cards[0]}, 1}),
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
    const int tableWidth = tableRowWidth(&game->playRound);
    int panelWidth = 78;

    if(tableWidth > panelWidth) {
        panelWidth = tableWidth;
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
        {0, 0, 0, 2},
        {0, 0, 0, 1},
    };

    Graphics_arrangeWithin(panel, GRAPHICS_DIRECTION_COLUMN, 0, sections, 9);

    const Graphics_Box dealerZone = Graphics_positionWithin(
        sections[4],
        (Graphics_Size){handZoneWidth(game->playRound.dealerHand.count),
            HAND_ZONE_HEIGHT},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_ControlGroup controls = playControls(game, sections[8]);
    Graphics_Label title =
        Graphics_label("PLAY", GRAPHICS_ALIGN_CENTER, true, false);
    const bool hideDealerCard = game->playRound.phase != ROUND_COMPLETE;

    Graphics_positionLabelWithin(
        &title,
        sections[0],
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_drawLabel(title);
    drawStatus(sections[2], game);
    drawZone(dealerZone, " Dealer ", false);
    drawHand(
        Graphics_inset(dealerZone, (Graphics_Padding){1, 2, 1, 2}),
        game->playRound.dealerHand,
        hideDealerCard
    );
    drawTableRow(sections[6], game);
    Graphics_drawControls(controls);
}
