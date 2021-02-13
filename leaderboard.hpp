#pragma once

#include "types/rect.hpp"
#include "graphics/font.hpp"

class Leaderboard final {
public:
    Leaderboard(const blit::Font &font = blit::minimal_font);

    void load();

    void setDisplayRect(blit::Rect r);

    void render();

    bool canAddScore(int score) const;
    void addScore(const char *name, int score);

private:
    static const int numEntries = 10;

    struct Entry {
        char name[8]{0};
        int score = 0;
    };

    Entry entries[numEntries];

    const blit::Font &font;
    blit::Rect displayRect;
};