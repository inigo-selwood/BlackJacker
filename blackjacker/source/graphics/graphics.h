#ifndef BLACKJACKER_GRAPHICS_H
#define BLACKJACKER_GRAPHICS_H

#include <stdbool.h>

#include "runtime/runtime.h"

enum { MIN_TERMINAL_WIDTH = 80, MIN_TERMINAL_HEIGHT = 26 };

/*******************************************************************************
 * Core Geometry
 ******************************************************************************/

/** Horizontal or vertical alignment preference. */
typedef enum {
    GRAPHICS_ALIGN_START,
    GRAPHICS_ALIGN_CENTER,
    GRAPHICS_ALIGN_END,
    GRAPHICS_ALIGN_STRETCH
} Graphics_Alignment;

/** Primary axis used by arrangement helpers. */
typedef enum {
    GRAPHICS_DIRECTION_ROW,
    GRAPHICS_DIRECTION_COLUMN
} Graphics_Direction;

/** Width and height pair. */
typedef struct {
    int width;
    int height;
} Graphics_Size;

/** Terminal box expressed as x/y origin plus dimensions. */
typedef struct {
    int x;
    int y;
    int width;
    int height;
} Graphics_Box;

/** Insets applied to a box. */
typedef struct {
    int top;
    int right;
    int bottom;
    int left;
} Graphics_Padding;

/** Returns the current full terminal box. */
Graphics_Box Graphics_screenBox(void);

/** Returns a box inset by padding. */
Graphics_Box Graphics_inset(Graphics_Box box, Graphics_Padding padding);

/** Positions a box of a given size inside a parent box. */
Graphics_Box Graphics_positionWithin(
    Graphics_Box parent,
    Graphics_Size size,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
);

/** Arranges child boxes inside a parent along a primary axis. */
void Graphics_arrangeWithin(
    Graphics_Box parent,
    Graphics_Direction direction,
    int gap,
    Graphics_Box *children,
    int count
);

/** Clears/fills a box with spaces using current attributes. */
void Graphics_drawBox(Graphics_Box box);

/** Draws a horizontal divider across a box's first row. */
void Graphics_drawHorizontalDivider(Graphics_Box box);

/*******************************************************************************
 * Tables
 ******************************************************************************/

/** Fixed-size grid rendered with box-drawing characters. */
typedef struct {
    Graphics_Box box;
    const char **columns;
    int columnCount;
    const char **rows;
    int rowCount;
    const char **cells;
    int *cellAttrs;
    int cellWidth;
    int rowLabelWidth;
} Graphics_Table;

/** Returns the natural size for a table. */
Graphics_Size Graphics_tableSize(Graphics_Table table);

/** Draws a table using its configured box and cell text. */
void Graphics_drawTable(Graphics_Table table);

/*******************************************************************************
 * Labels
 ******************************************************************************/

/** Text label plus simple rendering attributes. */
typedef struct {
    const char *text;
    Graphics_Box box;
    int width;
    int height;
    Graphics_Alignment alignment;
    bool bold;
    bool inverted;
} Graphics_Label;

/** Creates a single-line label, inferring width from text. */
Graphics_Label Graphics_label(
    const char *text,
    Graphics_Alignment alignment,
    bool bold,
    bool inverted
);

/** Positions a label within a parent box. */
void Graphics_positionLabelWithin(
    Graphics_Label *label,
    Graphics_Box parent,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
);

/** Draws a label using its own box. */
void Graphics_drawLabel(Graphics_Label label);

/** Draws wrapped label text using its own box. */
void Graphics_drawWrappedLabel(Graphics_Label label);

/*******************************************************************************
 * Block Text
 ******************************************************************************/

/** Multi-line fixed-width block text loaded from a file. */
typedef struct {
    const char *path;
    const char *fallbackPath;
    const char *fallbackText;
    Graphics_Box box;
    Graphics_Alignment alignment;
    char **lines;
    int lineCount;
    int width;
    bool bold;
    bool loaded;
} Graphics_BlockText;

/** Loads block text once and returns the cached block text. */
Graphics_BlockText *Graphics_loadBlockText(Graphics_BlockText *blockText);

/** Returns the rendered size for block text. */
Graphics_Size Graphics_blockTextSize(Graphics_BlockText *blockText);

/** Positions block text within a parent box. */
void Graphics_positionBlockTextWithin(
    Graphics_BlockText *blockText,
    Graphics_Box parent,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
);

/** Draws block text as one aligned fixed-width block using its own box. */
void Graphics_drawBlockText(Graphics_BlockText *blockText);

/*******************************************************************************
 * Controls
 ******************************************************************************/

/** Supported interactive control types. */
typedef enum {
    GRAPHICS_CONTROL_BUTTON,
    GRAPHICS_CONTROL_BOOL_INPUT,
    GRAPHICS_CONTROL_INT_INPUT,
    GRAPHICS_CONTROL_ENUM_INPUT
} Graphics_ControlType;

typedef struct Graphics_Control Graphics_Control;

/** Callback invoked by button controls. */
typedef void (*Graphics_ButtonAction)(Runtime_Game *game);

/** Payload for a control that triggers a callback. */
typedef struct {
    Graphics_ButtonAction action;
} Graphics_Button;

/** Payload for a control that edits a boolean field. */
typedef struct {
    bool *field;
} Graphics_BoolInput;

/** Payload for a control that edits an integer field. */
typedef struct {
    int *field;
    int min;
    int max;
    int step;
} Graphics_IntInput;

/** Payload for a control that edits an enum-like integer field. */
typedef struct {
    int *field;
    const char **choices;
    int choiceCount;
} Graphics_EnumInput;

/** Typed payload carried by a graphics control. */
typedef union {
    Graphics_Button button;
    Graphics_BoolInput boolean;
    Graphics_IntInput integer;
    Graphics_EnumInput enumeration;
} Graphics_Input;

/** Focusable control with geometry, tab order, label, and typed payload. */
struct Graphics_Control {
    const char *label;
    const char *info;
    int labelWidth;
    Graphics_Alignment labelAlignment;
    int width;
    int height;
    Graphics_Box bounds;
    int tabIndex;
    int shortcut;
    Graphics_ControlType type;
    Graphics_Input data;
};

/** Owning collection that manages focus and delegates input to controls. */
typedef struct {
    Graphics_Control *controls;
    int count;
    int *focusedIndex;
} Graphics_ControlGroup;

/** Returns the currently focused control, or null for an empty group. */
Graphics_Control *Graphics_focusedControl(Graphics_ControlGroup *group);

/** Positions a control within a parent box. */
void Graphics_positionControlWithin(
    Graphics_Control *control,
    Graphics_Box parent,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
);

/** Arranges controls inside a parent, using each control box as its request.
 */
void Graphics_arrangeControlsWithin(
    Graphics_Box parent,
    Graphics_Direction direction,
    int gap,
    Graphics_Control *controls,
    int count
);

/** Draws all controls in a group. */
void Graphics_drawControls(Graphics_ControlGroup group);

/** Handles navigation and activation input for a control group. */
bool Graphics_handleControlInput(
    Graphics_ControlGroup *group,
    int input,
    Runtime_Game *game
);

#endif
