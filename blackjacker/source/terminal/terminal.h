#ifndef BLACKJACKER_TERMINAL_H
#define BLACKJACKER_TERMINAL_H

#include <curses.h>

/** Initializes locale, ncurses, keyboard behavior, cursor state, and colors.
 */
void Terminal_init(void);

/** Returns the attributes used for card backs. */
attr_t Terminal_cardBackAttrs(void);

#endif
