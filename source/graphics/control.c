#include "graphics/graphics.h"

#include <curses.h>
#include <stdlib.h>
#include <string.h>

static int wrappedIndex(int index, int count) {
    if(count <= 0) {
        return 0;
    }

    if(index < 0) {
        return count - 1;
    }

    if(index >= count) {
        return 0;
    }

    return index;
}

static int controlValue(Graphics_Control control) {
    switch(control.type) {
    case GRAPHICS_CONTROL_BOOL_INPUT:
        return *control.data.boolean.field;
    case GRAPHICS_CONTROL_INT_INPUT:
        return *control.data.integer.field;
    case GRAPHICS_CONTROL_ENUM_INPUT:
        return *control.data.enumeration.field;
    case GRAPHICS_CONTROL_BUTTON:
    default:
        return 0;
    }
}

static void setControlValue(Graphics_Control control, int value) {
    switch(control.type) {
    case GRAPHICS_CONTROL_BOOL_INPUT:
        *control.data.boolean.field = value != 0;
        break;
    case GRAPHICS_CONTROL_INT_INPUT:
        *control.data.integer.field = value;
        break;
    case GRAPHICS_CONTROL_ENUM_INPUT:
        *control.data.enumeration.field = value;
        break;
    case GRAPHICS_CONTROL_BUTTON:
    default:
        break;
    }
}

static const char *controlValueLabel(Graphics_Control control) {
    const int value = controlValue(control);

    if(control.type == GRAPHICS_CONTROL_BOOL_INPUT) {
        return value ? "Yes" : "No";
    }

    if(control.type == GRAPHICS_CONTROL_ENUM_INPUT
        && control.data.enumeration.choices && value >= 0
        && value < control.data.enumeration.choiceCount) {
        return control.data.enumeration.choices[value];
    }

    return "";
}

static Graphics_Alignment controlLabelAlignment(Graphics_Control control) {
    if(control.labelAlignment == GRAPHICS_ALIGN_CENTER
        || control.labelAlignment == GRAPHICS_ALIGN_END
        || control.labelAlignment == GRAPHICS_ALIGN_STRETCH) {
        return control.labelAlignment;
    }

    return GRAPHICS_ALIGN_START;
}

static int alignedTextX(
    Graphics_Box box,
    const char *text,
    Graphics_Alignment alignment
) {
    const int textWidth = (int)strlen(text);
    const int visibleWidth = textWidth > box.width ? box.width : textWidth;

    if(alignment == GRAPHICS_ALIGN_CENTER) {
        return box.x + (box.width - visibleWidth) / 2;
    }

    if(alignment == GRAPHICS_ALIGN_END) {
        return box.x + box.width - visibleWidth;
    }

    return box.x;
}

static Graphics_Size controlRequestedSize(Graphics_Control control) {
    return (Graphics_Size){
        control.width,
        control.height > 0 ? control.height : 1,
    };
}

static int minTabIndex(Graphics_ControlGroup group) {
    int min = group.controls[0].tabIndex;

    for(int index = 1; index < group.count; index += 1) {
        if(group.controls[index].tabIndex < min) {
            min = group.controls[index].tabIndex;
        }
    }

    return min;
}

static int maxTabIndex(Graphics_ControlGroup group) {
    int max = group.controls[0].tabIndex;

    for(int index = 1; index < group.count; index += 1) {
        if(group.controls[index].tabIndex > max) {
            max = group.controls[index].tabIndex;
        }
    }

    return max;
}

static int indexForTab(Graphics_ControlGroup group, int tabIndex) {
    for(int index = 0; index < group.count; index += 1) {
        if(group.controls[index].tabIndex == tabIndex) {
            return index;
        }
    }

    return 0;
}

static void focusNext(Graphics_ControlGroup *group) {
    const int currentTab = group->controls[*group->focusedIndex].tabIndex;
    int nextTab = minTabIndex(*group);

    for(int index = 0; index < group->count; index += 1) {
        const int tabIndex = group->controls[index].tabIndex;

        if(tabIndex > currentTab
            && (nextTab <= currentTab || tabIndex < nextTab)) {
            nextTab = tabIndex;
        }
    }

    *group->focusedIndex = indexForTab(*group, nextTab);
}

static void focusPrevious(Graphics_ControlGroup *group) {
    const int currentTab = group->controls[*group->focusedIndex].tabIndex;
    int previousTab = maxTabIndex(*group);

    for(int index = 0; index < group->count; index += 1) {
        const int tabIndex = group->controls[index].tabIndex;

        if(tabIndex < currentTab
            && (previousTab >= currentTab || tabIndex > previousTab)) {
            previousTab = tabIndex;
        }
    }

    *group->focusedIndex = indexForTab(*group, previousTab);
}

static void changeControl(Graphics_Control control, int direction) {
    int value = controlValue(control);

    if(control.type == GRAPHICS_CONTROL_BOOL_INPUT) {
        setControlValue(control, !value);
        return;
    }

    if(control.type == GRAPHICS_CONTROL_INT_INPUT) {
        value += control.data.integer.step * direction;

        if(value < control.data.integer.min) {
            value = control.data.integer.min;
        } else if(value > control.data.integer.max) {
            value = control.data.integer.max;
        }

        setControlValue(control, value);
        return;
    }

    if(control.type == GRAPHICS_CONTROL_ENUM_INPUT) {
        setControlValue(
            control,
            wrappedIndex(
                value + direction,
                control.data.enumeration.choiceCount
            )
        );
    }
}

static void drawControl(Graphics_Control control, bool focused) {
    const int contentX = control.bounds.x + 1;
    const int contentWidth = control.bounds.width - 1;
    const Graphics_Box contentBox = {
        contentX,
        control.bounds.y,
        contentWidth,
        control.bounds.height,
    };

    if(focused) {
        attron(A_REVERSE);
    }

    Graphics_drawBox(control.bounds);

    if(control.type == GRAPHICS_CONTROL_BUTTON) {
        const int labelX = alignedTextX(
            contentBox,
            control.label,
            controlLabelAlignment(control)
        );

        mvaddnstr(control.bounds.y, labelX, control.label, contentWidth);
    } else if(control.type == GRAPHICS_CONTROL_INT_INPUT) {
        mvprintw(
            control.bounds.y,
            contentX,
            "%-*s %d",
            control.labelWidth,
            control.label,
            controlValue(control)
        );
    } else {
        mvprintw(
            control.bounds.y,
            contentX,
            "%-*s %s",
            control.labelWidth,
            control.label,
            controlValueLabel(control)
        );
    }

    if(focused) {
        attroff(A_REVERSE);
    }
}

Graphics_Control *Graphics_focusedControl(Graphics_ControlGroup *group) {
    if(group->count <= 0) {
        return 0;
    }

    *group->focusedIndex = wrappedIndex(*group->focusedIndex, group->count);
    return &group->controls[*group->focusedIndex];
}

void Graphics_positionControlWithin(
    Graphics_Control *control,
    Graphics_Box parent,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
) {
    control->bounds = Graphics_positionWithin(
        parent,
        controlRequestedSize(*control),
        horizontal,
        vertical
    );
}

void Graphics_arrangeControlsWithin(
    Graphics_Box parent,
    Graphics_Direction direction,
    int gap,
    Graphics_Control *controls,
    int count
) {
    Graphics_Box *boxes;

    if(count <= 0) {
        return;
    }

    boxes = malloc((size_t)count * sizeof(*boxes));

    if(!boxes) {
        return;
    }

    for(int index = 0; index < count; index += 1) {
        const Graphics_Size size = controlRequestedSize(controls[index]);
        boxes[index] = (Graphics_Box){0, 0, size.width, size.height};
    }

    Graphics_arrangeWithin(parent, direction, gap, boxes, count);

    for(int index = 0; index < count; index += 1) {
        controls[index].bounds = boxes[index];
        controls[index].tabIndex = index;
    }

    free(boxes);
}

void Graphics_drawControls(Graphics_ControlGroup group) {
    *group.focusedIndex = wrappedIndex(*group.focusedIndex, group.count);

    for(int index = 0; index < group.count; index += 1) {
        drawControl(group.controls[index], index == *group.focusedIndex);
    }
}

bool Graphics_handleControlInput(
    Graphics_ControlGroup *group,
    int input,
    Runtime_Game *game
) {
    Graphics_Control *control;

    if(group->count <= 0) {
        return false;
    }

    *group->focusedIndex = wrappedIndex(*group->focusedIndex, group->count);
    control = &group->controls[*group->focusedIndex];

    if(input == KEY_UP) {
        focusPrevious(group);
        return true;
    }

    if(input == KEY_DOWN || input == '\t') {
        focusNext(group);
        return true;
    }

    if(input == KEY_LEFT) {
        if(control->type == GRAPHICS_CONTROL_BUTTON) {
            focusPrevious(group);
        } else {
            changeControl(*control, -1);
        }
        return true;
    }

    if(input == KEY_RIGHT) {
        if(control->type == GRAPHICS_CONTROL_BUTTON) {
            focusNext(group);
        } else {
            changeControl(*control, 1);
        }
        return true;
    }

    if(input == '\n' || input == KEY_ENTER || input == ' ') {
        if(control->type == GRAPHICS_CONTROL_BUTTON
            && control->data.button.action) {
            control->data.button.action(game);
        } else {
            changeControl(*control, 1);
        }

        return true;
    }

    return false;
}
