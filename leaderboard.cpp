#include <cstring>

#include "engine/engine.hpp"
#include "engine/save.hpp"

#include "leaderboard.hpp"

using namespace blit;

Leaderboard::Leaderboard(const Font &font) : font(font) {}

void Leaderboard::load() {
    read_save(entries);
}

void Leaderboard::setDisplayRect(Rect r) {
    displayRect = r;
}

void Leaderboard::render() {
    auto oldClip = screen.clip;
    screen.clip = displayRect;

    screen.text("Scores", font, displayRect, true, TextAlign::top_center);

    int y = displayRect.y + font.char_h + font.spacing_y;

    int i = 0;
    for(auto &entry : entries) {
        // highlight new entry
        screen.pen = i == lastUpdatedEntry ? Pen{255, 0, 0} : Pen{255, 255, 255};

        Rect lineRect(displayRect.x, y, displayRect.w, font.char_h);
        screen.text(entry.name, font, lineRect);
        screen.text(std::to_string(entry.score), font, lineRect, true, TextAlign::top_right);

        y += font.char_h + font.spacing_y;
        i++;
    }

    screen.clip = oldClip;
}

bool Leaderboard::canAddScore(int score) const {
    return score > entries[numEntries - 1].score;
}

void Leaderboard::addScore(const char *name, int score) {
    int i = 0;
    for(; i < numEntries; i++) {
        if(entries[i].score < score)
            break;
    }

    // someone didn't check first...
    if(i == numEntries)
        return;

    // move every score down
    for(int j = numEntries - 1; j > i; j--)
        entries[j] = entries[j - 1];

    entries[i].score = score;
    strncpy(entries[i].name, name, 8);

    lastUpdatedEntry = i;

    write_save(entries);
}