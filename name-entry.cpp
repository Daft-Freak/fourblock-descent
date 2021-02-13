#include "engine/api.hpp"
#include "engine/engine.hpp"

#include "name-entry.hpp"

using namespace blit;

NameEntry::NameEntry(const Font &font) : font(font) {}

void NameEntry::setDisplayRect(Rect r) {
    displayRect = r;
}

void NameEntry::render() {
    int w = nameLen * (font.char_w + 1) - 1;

    int x = displayRect.x + (displayRect.w - w) / 2;
    int y = displayRect.y + (displayRect.h - font.char_h) / 2;

    screen.text("Enter name:", font, Point(displayRect.x + displayRect.w / 2, (displayRect.y + y) / 2), true, TextAlign::center_center);
    screen.text("Press A\nto continue", font, Point(displayRect.x + displayRect.w / 2, (displayRect.y + displayRect.h + y) / 2), true, TextAlign::center_center);

    for(auto &i : name) {
        char c = charList[i];
        screen.text(std::string_view(&c, 1), font, Point(x, y), false);
        x += font.char_w + 1;
    }

    // current letter, up/down indicators
    int halfW = (font.char_w / 2) - 1;

    x = displayRect.x + (displayRect.w - w) / 2; // reset
    x += (font.char_w + 1) * cursor;

    Point tip(x + halfW, y - 4 - halfW);
    screen.line(Point(x, y - 4), tip);
    screen.line(tip, Point(x + halfW * 2, y - 4));

    tip.y = y + font.char_h + 4 + halfW;
    screen.line(Point(x, y + font.char_h + 4), tip);
    screen.line(tip, Point(x + halfW * 2, y + font.char_h + 4));
}

void NameEntry::update() {
    if(buttons.released & Button::DPAD_LEFT)
        cursor = cursor ? cursor - 1 : nameLen - 1;
    else if(buttons.released & Button::DPAD_RIGHT)
        cursor = (cursor + 1) % nameLen;

    if(buttons.released & Button::DPAD_UP)
        name[cursor] = (name[cursor] + 1) % numChars;
    else if(buttons.released & Button::DPAD_DOWN)
        name[cursor] = name[cursor] ? name[cursor] - 1 : numChars - 1;
}

std::string NameEntry::getName() const {
    std::string ret;
    ret.reserve(nameLen);

    for(auto &i : name)
        ret += charList[i];

    // remove trailing spaces
    while(!ret.empty() && ret.back() == ' ')
        ret.pop_back();

    return ret;
}