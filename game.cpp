#include "game.hpp"
#include "assets.hpp"

using namespace blit;

static const Font font(asset_font8x8);

static const int gridWidth = 10, gridHeight = 15;
static uint8_t grid[gridWidth * gridHeight]{0};

static const int blockSize = 8;

static const int fallTime = 30;

struct Block {
    bool pattern[2][4];
    int width = 0, height = 0;
};

static Block blocks[]{
    //Z
    {
        {
            {true, true, false},
            {false, true, true}
        },
        3, 2
    },
    //L
    {
        {
            {false, false, true},
            {true, true, true}
        },
        3, 2
    },
    //O
    {
        {
            {true, true},
            {true, true}
        },
        2, 2
    },
    //S
    {
        {
            {false, true, true},
            {true, true, false}
        },
        3, 2
    },
    //I
    {
        {
            {true, true, true, true}
        },
        4, 1
    },
    //J
    {
        {
            {true},
            {true, true, true}
        },
        3, 2
    },
    //T
    {
        {
            {false, true},
            {true, true, true}
        },
        3, 2
    }
};

static struct {
    Point pos;
    int id = -1;
    int rot = 0;
    int timer = 0;

} blockFalling;

static int nextBlock = 0;

static int move = 0, rotate = 0;

static int score = 0;
static int lines = 0;
static bool lastWasTetris = false;

static bool gameEnded = false;

static Point rotateIt(Point pos, int w, int h, int rot) {
    int rX = pos.x;
    int rY = pos.y;

    if(rot == 1) {
        rX = (h - 1) - pos.y;
        rY =  pos.x;
    } else if(rot == 2) {
        rX = (w - 1) - pos.x;
        rY = (h - 1) - pos.y;
    } else if(rot == 3) {
        rX = pos.y;
        rY = (w - 1) - pos.x;
    }

    return Point(rX, rY);
}


void checkLine() {
    //most lines possible at once = 4
    for(int l = 0; l < 4; l++) {
        int addedScore = 0;

        int clearedLines = 0;
        int found = 0;
        bool stop = false;

        for(int y = gridHeight - 1; y >= 0; y--) {
            int isLine = true;
            for(int x = 0; x < gridWidth; x++) {
                if(grid[x + y * gridWidth] == 0) {
                    isLine = false;
                    break;
                }
            }

            if(found && !isLine)
                stop = true;

            if(isLine && found == 0)
                found = y;

            if(stop)
                break;

            if(found != 0)
                clearedLines++;
        }

        if(clearedLines == 0)
            return;

        addedScore = clearedLines * 10 + lines;

        //"tetris"
        if(clearedLines == 4)
            addedScore *= lastWasTetris ? 3: 2;

        lastWasTetris = clearedLines == 4;

        //move down
        for(int y = found; y >= 0; y--) {
            for(int x = 0; x < gridWidth; x++) {
                int newY = y + clearedLines;
                if(newY > found)
                    continue;

                grid[x + newY * gridWidth] = grid[x + y * gridWidth];
            }
        }

        //fill top
        for(int i = 0; i < clearedLines; i++) {
            for(int x = 0; x < gridWidth; x++) {
                grid[x + i * gridWidth] = 0;
            }
        }

        score += addedScore;
        lines += clearedLines;
    }
}

void placeBlock() {
    auto &block = blocks[blockFalling.id];

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            Point rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, blockFalling.rot);

            if(block.pattern[y][x]) {
                grid[rotPos.x + rotPos.y * gridWidth] = blockFalling.id + 1;
            }
        }
    }
}

bool blockHitX(int move) {
    auto &block = blocks[blockFalling.id];

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            Point rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, blockFalling.rot);

            if(block.pattern[y][x]) {
                //side
                if(rotPos.x + move >= gridWidth)
                    return true;

                if(rotPos.x + move < 0)
                    return true;

                //block block beside
                if(rotPos.x + move < gridWidth) {
                    if(grid[rotPos.x + move + rotPos.y * gridWidth] != 0)
                        return true;
                }
            }
        }
    }

    return false;
}

bool blockHit() {
    auto &block = blocks[blockFalling.id];

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            Point rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, blockFalling.rot);

            if(block.pattern[y][x] != 0) {
                //bottom
                if(rotPos.y >= gridHeight - 1)
                    return true;

                //block under
                if(rotPos.y + 1 < gridHeight) {
                    if(grid[rotPos.x + (rotPos.y + 1) * gridWidth] != 0)
                        return true;
                }

            }
        }
    }

    return false;
}

bool blockHitRot(int newRot) {
    auto &block = blocks[blockFalling.id];

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            Point rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, newRot);

            //inside block
            if(block.pattern[y][x]) {
                if(grid[rotPos.x + rotPos.y * gridWidth] != 0)
                    return true;
            }
        }
    }

    return false;
}

void pushAwayFromSide() {
    auto &block = blocks[blockFalling.id];

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            Point rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, blockFalling.rot);

            //inside wall
            if(block.pattern[y][x] != 0) {
                if(rotPos.x >= gridWidth)
                    blockFalling.pos.x--;
            }
        }
    }
}

bool checkLost() {
    for(int x = 0; x < gridWidth; x++) {
        if(grid[x] != 0)
            return true;
    }
    return false;
}

void init() {
    set_screen_mode(ScreenMode::lores);

    screen.sprites = Surface::load(asset_tetris_sprites);
}

// drawing
void render(uint32_t time) {
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.pen = Pen(0xFF,0xFF,0xFF);
    screen.rectangle(Rect(0, 0, gridWidth * blockSize, gridHeight * blockSize));

    for(int y = 0; y < gridHeight; y++) {
        for(int x = 0; x < gridWidth; x++) {
            if(grid[x + y * gridWidth] != 0) {
                screen.sprite(grid[x + y * gridWidth] - 1, Point(x * blockSize, y * blockSize));
            }
        }
    }

    //draw falling block
    if(blockFalling.id != -1) {
        auto &block = blocks[blockFalling.id];

        for(int y = 0; y < 2; y++) {
            for(int x = 0; x < 4; x++) {
                auto rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, blockFalling.rot);

                if(block.pattern[y][x])
                    screen.sprite(blockFalling.id, Point(rotPos.x * blockSize, rotPos.y * blockSize));
            }
        }
    }

    int x = gridWidth * blockSize + 8;
    int infoW = screen.bounds.w - (gridWidth * blockSize + 16);

    screen.text("Score:", font, Point(x, 8));
    screen.text(std::to_string(score), font, Rect(x, 8, infoW, 8), true, TextAlign::top_right);

    screen.text("Lines:", font, Point(x, 20));
    screen.text(std::to_string(lines), font, Rect(x, 20, infoW, 8), true, TextAlign::top_right);

    screen.text("Next:", font, Point(x, 32));

    auto &block = blocks[nextBlock];
    Point nextBlockPos(x + (infoW - block.width * blockSize) / 2, 40 + (24 - block.height * blockSize) / 2);

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            if(block.pattern[y][x])
                screen.sprite(nextBlock, nextBlockPos + Point(x * blockSize, y * blockSize));
        }
    }
}


void update(uint32_t time) {
    if(gameEnded)
        return;

    if(checkLost()) {
        gameEnded = false;
        return;
    }

    // input
    if(buttons.pressed & Button::A)
        rotate = 1;

    if(buttons.pressed & Button::DPAD_LEFT)
        move = -1;
    else if(buttons.pressed & Button::DPAD_RIGHT)
        move = 1;

    if(blockFalling.id == -1) {
        blockFalling.id = nextBlock;
        blockFalling.timer = 0;
        blockFalling.pos.y = 0;
        blockFalling.pos.x = 5 - blocks[blockFalling.id].width / 2;
        blockFalling.rot = 0;

        //check if it fits
        if(blockHitRot(0)) {
            blockFalling.id = -1;
        }

        nextBlock = blit::random() % 7;
    } else {
        if(rotate != 0) {
            if(!blockHitRot(blockFalling.rot + rotate)) {
                blockFalling.rot++;

                blockFalling.rot %= 4;

                pushAwayFromSide();
            }

            rotate = 0;
        }

        if(move != 0) {
            if(!blockHitX(move)) {
                blockFalling.pos.x += move;
                move = 0;
            }
        }

        if(blockFalling.timer >= fallTime) {
            if(blockHit()) {
                placeBlock();

                checkLine();

                blockFalling.id = -1;
            } else {
                blockFalling.pos.y++;
                blockFalling.timer = 0;
            }
        }
        else
            blockFalling.timer++;
    }
}