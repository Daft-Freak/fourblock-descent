#pragma once
#include <string>

#include "types/rect.hpp"
#include "graphics/font.hpp"

class NameEntry final {
public:
    NameEntry(const blit::Font &font);

    void setDisplayRect(blit::Rect r);

    void render();

    void update();

    std::string getName() const;

private:
    static constexpr const char *charList = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.?!_ ";
    static const int numChars = 31;

    const blit::Font &font;
    blit::Rect displayRect;

    int cursor = 0;

    static const int nameLen = 7;
    int8_t name[nameLen]{0};
};