#ifndef BLACKJACKER_RENDER_CARD_H
#define BLACKJACKER_RENDER_CARD_H

#include <stdbool.h>

#include "table/table.h"

/**
 * Draws a compact card at terminal coordinates.
 *
 * @param x Left column.
 * @param y Top row.
 * @param card Table_Card identity to render.
 * @param faceUp Whether to show the face or card back.
 * @param miniature Whether to use the miniature card format.
 */
void drawCard(int x, int y, Table_Card card, bool faceUp, bool miniature);

#endif
