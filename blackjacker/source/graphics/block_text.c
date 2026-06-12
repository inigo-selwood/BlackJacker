#include "graphics/graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool ensureLineCapacity(char **line, int *capacity, int required) {
    char *resized;
    int nextCapacity = *capacity;

    if(nextCapacity >= required) {
        return true;
    }

    if(nextCapacity <= 0) {
        nextCapacity = 64;
    }

    while(nextCapacity < required) {
        nextCapacity *= 2;
    }

    resized = realloc(*line, (size_t)nextCapacity);

    if(!resized) {
        free(*line);
        *line = 0;
        *capacity = 0;
        return false;
    }

    *line = resized;
    *capacity = nextCapacity;
    return true;
}

static char *readBlockTextLine(FILE *file) {
    char *line = 0;
    int capacity = 0;
    int length = 0;
    int character;

    while((character = fgetc(file)) != EOF) {
        if(character == '\r') {
            continue;
        }

        if(character == '\n') {
            break;
        }

        if(!ensureLineCapacity(&line, &capacity, length + 2)) {
            return 0;
        }

        line[length] = (char)character;
        length += 1;
    }

    if(character == EOF && length == 0) {
        free(line);
        return 0;
    }

    if(!ensureLineCapacity(&line, &capacity, length + 1)) {
        return 0;
    }

    line[length] = '\0';
    return line;
}

static char *copyText(const char *text) {
    const int length = (int)strlen(text);
    char *copy = malloc((size_t)length + 1);

    if(!copy) {
        return 0;
    }

    memcpy(copy, text, (size_t)length + 1);
    return copy;
}

static bool appendBlockTextLine(Graphics_BlockText *blockText, char *line) {
    char **resized;
    const int width = line ? (int)strlen(line) : 0;

    if(!line) {
        return false;
    }

    resized = realloc(
        blockText->lines,
        (size_t)(blockText->lineCount + 1) * sizeof(char *)
    );

    if(!resized) {
        free(line);
        return false;
    }

    blockText->lines = resized;
    blockText->lines[blockText->lineCount] = line;
    blockText->lineCount += 1;

    if(width > blockText->width) {
        blockText->width = width;
    }

    return true;
}

static FILE *openBlockTextFile(Graphics_BlockText *blockText) {
    FILE *file = blockText->path ? fopen(blockText->path, "r") : 0;

    if(file || !blockText->fallbackPath) {
        return file;
    }

    return fopen(blockText->fallbackPath, "r");
}

Graphics_BlockText *Graphics_loadBlockText(Graphics_BlockText *blockText) {
    FILE *file;

    if(blockText->loaded) {
        return blockText;
    }

    blockText->loaded = true;
    file = openBlockTextFile(blockText);

    if(!file) {
        if(blockText->fallbackText) {
            appendBlockTextLine(blockText, copyText(blockText->fallbackText));
        }

        return blockText;
    }

    while(!feof(file)) {
        char *line = readBlockTextLine(file);

        if(!line) {
            break;
        }

        if(!appendBlockTextLine(blockText, line)) {
            break;
        }
    }

    fclose(file);

    if(blockText->lineCount <= 0 && blockText->fallbackText) {
        appendBlockTextLine(blockText, copyText(blockText->fallbackText));
    }

    return blockText;
}

Graphics_Size Graphics_blockTextSize(Graphics_BlockText *blockText) {
    Graphics_BlockText *loaded = Graphics_loadBlockText(blockText);

    return (Graphics_Size){
        loaded->width,
        loaded->lineCount,
    };
}

void Graphics_positionBlockTextWithin(
    Graphics_BlockText *blockText,
    Graphics_Box parent,
    Graphics_Alignment horizontal,
    Graphics_Alignment vertical
) {
    const Graphics_Size size = Graphics_blockTextSize(blockText);

    blockText->box =
        Graphics_positionWithin(parent, size, horizontal, vertical);
}

void Graphics_drawBlockText(Graphics_BlockText *blockText) {
    Graphics_BlockText *loaded = Graphics_loadBlockText(blockText);
    const Graphics_Box block = Graphics_positionWithin(
        loaded->box,
        (Graphics_Size){loaded->width, loaded->lineCount},
        loaded->alignment,
        GRAPHICS_ALIGN_START
    );

    for(int index = 0; index < loaded->lineCount && index < loaded->box.height;
        index += 1) {
        Graphics_Label label = Graphics_label(
            loaded->lines[index],
            GRAPHICS_ALIGN_START,
            loaded->bold ? GRAPHICS_TEXT_BOLD : GRAPHICS_TEXT_NORMAL
        );

        Graphics_positionLabelWithin(
            &label,
            (Graphics_Box){block.x, block.y + index, block.width, 1},
            GRAPHICS_ALIGN_STRETCH,
            GRAPHICS_ALIGN_STRETCH
        );

        Graphics_drawLabel(label);
    }
}
