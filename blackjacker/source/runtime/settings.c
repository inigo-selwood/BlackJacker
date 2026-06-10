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

static const char *doubleRuleText(DoubleRule rule) {
    switch(rule) {
    case DOUBLE_9_10_11:
        return "9-10-11";
    case DOUBLE_10_11:
        return "10-11";
    case DOUBLE_ANY_TWO:
    default:
        return "any-two";
    }
}

static bool parseDoubleRule(const char *text, DoubleRule *rule) {
    if(stringsEqual(text, "any-two") || stringsEqual(text, "any_two")) {
        *rule = DOUBLE_ANY_TWO;
        return true;
    }

    if(stringsEqual(text, "9-10-11") || stringsEqual(text, "9_10_11")) {
        *rule = DOUBLE_9_10_11;
        return true;
    }

    if(stringsEqual(text, "10-11") || stringsEqual(text, "10_11")) {
        *rule = DOUBLE_10_11;
        return true;
    }

    return false;
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

static const char *blackjackPayoutText(BlackjackPayout payout) {
    switch(payout) {
    case BLACKJACK_PAYS_6_TO_5:
        return "6-to-5";
    case BLACKJACK_PAYS_3_TO_2:
    default:
        return "3-to-2";
    }
}

static bool parseBlackjackPayout(const char *text, BlackjackPayout *payout) {
    if(stringsEqual(text, "3-to-2") || stringsEqual(text, "3_to_2")) {
        *payout = BLACKJACK_PAYS_3_TO_2;
        return true;
    }

    if(stringsEqual(text, "6-to-5") || stringsEqual(text, "6_to_5")) {
        *payout = BLACKJACK_PAYS_6_TO_5;
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

    if((stringsEqual(key, "deck-count") || stringsEqual(key, "deck_count"))
        && parseInt(value, &integer)) {
        settings->deckCount = integer;
    } else if((stringsEqual(key, "cut-percent")
                  || stringsEqual(key, "cut_percent"))
        && parseInt(value, &integer)) {
        settings->cutPercent = integer;
    } else if((stringsEqual(key, "allow-double-down")
                  || stringsEqual(key, "allow_double_down"))
        && parseBool(value, &boolean)) {
        settings->allowDoubleDown = boolean;
    } else if((stringsEqual(key, "allow-double-after-split")
                  || stringsEqual(key, "allow_double_after_split"))
        && parseBool(value, &boolean)) {
        settings->allowDoubleAfterSplit = boolean;
    } else if(stringsEqual(key, "double-rule")
        || stringsEqual(key, "double_rule")) {
        parseDoubleRule(value, &settings->doubleRule);
    } else if((stringsEqual(key, "allow-split")
                  || stringsEqual(key, "allow_split"))
        && parseBool(value, &boolean)) {
        settings->allowSplit = boolean;
    } else if((stringsEqual(key, "allow-resplit")
                  || stringsEqual(key, "allow_resplit"))
        && parseBool(value, &boolean)) {
        settings->allowResplit = boolean;
    } else if((stringsEqual(key, "allow-hit-split-aces")
                  || stringsEqual(key, "allow_hit_split_aces"))
        && parseBool(value, &boolean)) {
        settings->allowHitSplitAces = boolean;
    } else if((stringsEqual(key, "allow-resplit-aces")
                  || stringsEqual(key, "allow_resplit_aces"))
        && parseBool(value, &boolean)) {
        settings->allowResplitAces = boolean;
    } else if((stringsEqual(key, "allow-surrender")
                  || stringsEqual(key, "allow_surrender"))
        && parseBool(value, &boolean)) {
        settings->allowSurrender = boolean;
    } else if((stringsEqual(key, "use-no-hole-card-rule")
                  || stringsEqual(key, "use_no_hole_card_rule"))
        && parseBool(value, &boolean)) {
        settings->useNoHoleCardRule = boolean;
    } else if((stringsEqual(key, "show-true-count")
                  || stringsEqual(key, "show_true_count"))
        && parseBool(value, &boolean)) {
        settings->showTrueCount = boolean;
    } else if(stringsEqual(key, "dealer-soft-17-rule")
        || stringsEqual(key, "dealer_soft_17_rule")) {
        parseDealerSoft17Rule(value, &settings->dealerSoft17Rule);
    } else if(stringsEqual(key, "blackjack-payout")
        || stringsEqual(key, "blackjack_payout")) {
        parseBlackjackPayout(value, &settings->blackjackPayout);
    }
}

PlaySettings Runtime_defaultPlaySettings(void) {
    return (PlaySettings){
        .deckCount = 6,
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
        } else if(event.type == YAML_SCALAR_EVENT) {
            if(!hasKey) {
                hasKey = scalarValue(&event, key, sizeof(key));
            } else if(scalarValue(&event, value, sizeof(value))) {
                applySetting(settings, key, value);
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

    fprintf(file, "deck-count: %d\n", settings->deckCount);
    fprintf(file, "cut-percent: %d\n", settings->cutPercent);
    fprintf(
        file,
        "allow-double-down: %s\n",
        boolText(settings->allowDoubleDown)
    );
    fprintf(
        file,
        "allow-double-after-split: %s\n",
        boolText(settings->allowDoubleAfterSplit)
    );
    fprintf(file, "double-rule: %s\n", doubleRuleText(settings->doubleRule));
    fprintf(file, "allow-split: %s\n", boolText(settings->allowSplit));
    fprintf(file, "allow-resplit: %s\n", boolText(settings->allowResplit));
    fprintf(
        file,
        "allow-hit-split-aces: %s\n",
        boolText(settings->allowHitSplitAces)
    );
    fprintf(
        file,
        "allow-resplit-aces: %s\n",
        boolText(settings->allowResplitAces)
    );
    fprintf(file, "allow-surrender: %s\n", boolText(settings->allowSurrender));
    fprintf(
        file,
        "use-no-hole-card-rule: %s\n",
        boolText(settings->useNoHoleCardRule)
    );
    fprintf(file, "show-true-count: %s\n", boolText(settings->showTrueCount));
    fprintf(
        file,
        "dealer-soft-17-rule: %s\n",
        dealerSoft17RuleText(settings->dealerSoft17Rule)
    );
    fprintf(
        file,
        "blackjack-payout: %s\n",
        blackjackPayoutText(settings->blackjackPayout)
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
