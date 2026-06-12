#include "runtime/runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaml.h>

#include "table/table.h"

enum { SETTINGS_BUFFER_SIZE = 32 };

static const char *SETTINGS_PATH = "resources/settings.yaml";

static bool stringsEqual(const char *left, const char *right) {
    return left && right && strcmp(left, right) == 0;
}

static bool parseBool(const char *text, bool *value) {
    if(stringsEqual(text, "true") || stringsEqual(text, "yes")
        || stringsEqual(text, "1")) {
        *value = true;
        return true;
    }

    if(stringsEqual(text, "false") || stringsEqual(text, "no")
        || stringsEqual(text, "0")) {
        *value = false;
        return true;
    }

    return false;
}

static bool parseInt(const char *text, int *value) {
    char *end = 0;

    if(!text || !*text) {
        return false;
    }

    const long parsed = strtol(text, &end, 10);

    if(!end || *end) {
        return false;
    }

    *value = (int)parsed;
    return true;
}

static const char *boolText(bool value) {
    return value ? "true" : "false";
}

static const char *dealerSoft17RuleText(DealerSoft17Rule rule) {
    switch(rule) {
    case DEALER_HITS_SOFT_17:
        return "hit";
    case DEALER_STANDS_SOFT_17:
    default:
        return "stand";
    }
}

static bool parseDealerSoft17Rule(const char *text, DealerSoft17Rule *rule) {
    if(stringsEqual(text, "hit")) {
        *rule = DEALER_HITS_SOFT_17;
        return true;
    }

    if(stringsEqual(text, "stand")) {
        *rule = DEALER_STANDS_SOFT_17;
        return true;
    }

    return false;
}

static bool scalarValue(yaml_event_t *event, char *buffer, int bufferSize) {
    const char *value;

    if(event->type != YAML_SCALAR_EVENT) {
        return false;
    }

    value = (const char *)event->data.scalar.value;

    if(!value) {
        return false;
    }

    snprintf(buffer, (size_t)bufferSize, "%s", value);
    return true;
}

static void
applySetting(PlaySettings *settings, const char *key, const char *value) {
    int integer;
    bool boolean;

    if(stringsEqual(key, "deck-count") && parseInt(value, &integer)) {
        settings->deckCount = integer;
    } else if(stringsEqual(key, "allow-double-after-split")
        && parseBool(value, &boolean)) {
        settings->allowDoubleAfterSplit = boolean;
    } else if(stringsEqual(key, "allow-surrender")
        && parseBool(value, &boolean)) {
        settings->allowSurrender = boolean;
    } else if(stringsEqual(key, "dealer-soft-17-rule")) {
        parseDealerSoft17Rule(value, &settings->dealerSoft17Rule);
    }
}

static bool isRulesSection(const char *section) {
    return stringsEqual(section, "rules");
}

PlaySettings Runtime_defaultPlaySettings(void) {
    return (PlaySettings){
        .deckCount = 3,
        .cutPercent = 75,
        .allowDoubleDown = true,
        .allowDoubleAfterSplit = true,
        .doubleRule = DOUBLE_ANY_TWO,
        .allowSplit = true,
        .allowResplit = true,
        .allowHitSplitAces = false,
        .allowResplitAces = true,
        .allowSurrender = false,
        .useNoHoleCardRule = false,
        .showTrueCount = true,
        .dealerSoft17Rule = DEALER_STANDS_SOFT_17,
        .blackjackPayout = BLACKJACK_PAYS_3_TO_2,
    };
}

bool Runtime_loadSettings(PlaySettings *settings) {
    FILE *file = fopen(SETTINGS_PATH, "rb");
    yaml_parser_t parser;
    yaml_event_t event;
    char key[SETTINGS_BUFFER_SIZE] = {0};
    char value[SETTINGS_BUFFER_SIZE] = {0};
    char section[SETTINGS_BUFFER_SIZE] = {0};
    int mappingDepth = 0;
    bool hasKey = false;
    bool done = false;
    bool ok = true;

    if(!settings || !file) {
        return false;
    }

    if(!yaml_parser_initialize(&parser)) {
        fclose(file);
        return false;
    }

    yaml_parser_set_input_file(&parser, file);

    while(!done) {
        if(!yaml_parser_parse(&parser, &event)) {
            ok = false;
            break;
        }

        if(event.type == YAML_STREAM_END_EVENT) {
            done = true;
        } else if(event.type == YAML_MAPPING_START_EVENT) {
            mappingDepth += 1;

            if(mappingDepth == 2 && hasKey) {
                snprintf(section, sizeof(section), "%s", key);
                hasKey = false;
            }
        } else if(event.type == YAML_MAPPING_END_EVENT) {
            if(mappingDepth == 2) {
                section[0] = '\0';
            }

            hasKey = false;
            mappingDepth -= 1;
        } else if(event.type == YAML_SCALAR_EVENT) {
            if(!hasKey) {
                hasKey = scalarValue(&event, key, sizeof(key));
            } else if(scalarValue(&event, value, sizeof(value))) {
                if(mappingDepth == 2 && isRulesSection(section)) {
                    applySetting(settings, key, value);
                }

                hasKey = false;
            }
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(file);

    return ok;
}

bool Runtime_saveSettings(const PlaySettings *settings) {
    FILE *file = fopen(SETTINGS_PATH, "wb");

    if(!settings || !file) {
        return false;
    }

    fprintf(file, "rules:\n");
    fprintf(file, "  deck-count: %d\n", settings->deckCount);
    fprintf(
        file,
        "  allow-double-after-split: %s\n",
        boolText(settings->allowDoubleAfterSplit)
    );
    fprintf(
        file,
        "  allow-surrender: %s\n",
        boolText(settings->allowSurrender)
    );
    fprintf(
        file,
        "  dealer-soft-17-rule: %s\n",
        dealerSoft17RuleText(settings->dealerSoft17Rule)
    );

    fclose(file);
    return true;
}

bool Runtime_updateSettings(Runtime_Game *game, PlaySettings settings) {
    if(!game) {
        return false;
    }

    game->playSettings = settings;
    Table_initShoe(
        &game->shoe,
        game->playSettings.deckCount,
        game->playSettings.cutPercent
    );

    return Runtime_saveSettings(&game->playSettings);
}
