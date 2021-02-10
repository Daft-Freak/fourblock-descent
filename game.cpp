#include "game.hpp"

using namespace blit;

static const int width = 10, height = 20;
static uint8_t grid[width * height]{0};

struct Block {
    bool pattern[2][4];
    Pen col;
    int width = 0, height = 0;
};

Block blocks[]{
    //I
    {
        {
            {true, true, true, true}
        },
        Pen{0, 0xFF, 0xFF},
        4, 1
    },
    //J
    {
        {
            {true},
            {true, true, true}
        },
        Pen{0, 0, 0xFF},
        3, 2
    },
    //L
    {
        {
            {false, false, true},
            {true, true, true}
        },
        Pen{0xFF, 0x77, 0},
        3, 2
    },
    //Z
    {
        {
            {true, true, false},
            {false, true, true}
        },
        Pen{0xFF, 0, 0},
        3, 2
    },
    //S
    {
        {
            {false, true, true},
            {true, true, false}
        },
        Pen{0, 0xFF, 0},
        3, 2
    },
    //O
    {
        {
            {true, true},
            {true, true}
        },
        Pen{0xFF, 0xFF, 0},
        2, 2
    },
    //T
    {
        {
            {false, true},
            {true, true, true}
        },
        Pen{0x77, 0, 0xFF},
        3, 2
    }
};

static struct {
    int x = 0, y = 0;
    int id = -1;
    int rot = 0;
    int timer = 0;

} blockFalling;

static int move = 0, rotate = 0;

int score = 0;
bool lastWasTetris = false;

bool gameEnded = false;

static Point rotateIt(int x, int y, int w, int h, int rot) {
    int rX = x;
    int rY = y;

    if(rot == 1) {
        rX = (h - 1) - y;
        rY =  x;
    } else if(rot == 2) {
        rX = (w - 1) - x;
        rY = (h - 1) - y;
    } else if(rot == 3) {
        rX = y;
        rY = (w - 1) - x;
    }

    return Point(rX, rY);
}


void checkLine() {
    //most lines possible at once = 4
    for(int l = 0; l < 4; l++) {
        int addedScore = 0;

        int lines = 0;
        int found = 0;
        bool stop = false;

        for(int y = height - 1; y >= 0; y--) {
            int isLine = true;
            for(int x = 0; x < width; x++) {
                if(grid[x + y * width] == 0) {
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
                lines++;
        }

        if(lines == 0)
            return;


        addedScore = lines * 100;

        //"tetris"
        if(lines == 4)
            addedScore *= lastWasTetris ? 3: 2;

        lastWasTetris = lines == 4;

        //move down
        for(int y = found; y >= 0; y--) {
            for(int x = 0; x < width; x++) {
                int newY = y + lines;
                if(newY > found)
                    continue;

                grid[x + newY * width] = grid[x + y * width];
            }
        }

        //fill top
        for(int i = 0; i < lines; i++) {
            for(int x = 0; x < width; x++) {
                grid[x + i * width] = 0;
            }
        }

        score += addedScore;

        //$("#Score").html(score);
    }
}

void placeBlock() {
    auto &block = blocks[blockFalling.id];

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            Point rotPos = rotateIt(x, y, block.width, block.height, blockFalling.rot);

            if(block.pattern[y][x]) {
                grid[(blockFalling.x + rotPos.x) + (blockFalling.y + rotPos.y) * width] = blockFalling.id + 1;
            }
        }
    }
}

bool blockHitX(int move) {
    auto &block = blocks[blockFalling.id];

    for(int y = 0; y < block.height; y++) {
        for(int x = 0; x < block.width; x++) {
            Point rotPos = rotateIt(x, y, block.width, block.height, blockFalling.rot);

            if(block.pattern[y][x]) {
                //side
                if(blockFalling.x + rotPos.x + move >= width)
                    return true;

                if(blockFalling.x + rotPos.x + move < 0)
                    return true;

                //block block beside
                if(blockFalling.x + rotPos.x + move < width) {
                    if(grid[(blockFalling.x + rotPos.x + move) + (blockFalling.y + rotPos.y) * width] != 0)
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
            Point rotPos = rotateIt(x, y, block.width, block.height, blockFalling.rot);

            if(block.pattern[y][x] != 0) {
                //bottom
                if(blockFalling.y + rotPos.y >= height - 1)
                    return true;

                //block under
                if(blockFalling.y + rotPos.y + 1 < height) {
                    if(grid[(blockFalling.x + rotPos.x) + (blockFalling.y + rotPos.y + 1) * width] != 0)
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
            Point rotPos = rotateIt(x, y, block.width, block.height, newRot);

            //inside block
            if(block.pattern[y][x]) {
                if(grid[(blockFalling.x + rotPos.x) + (blockFalling.y + rotPos.x) * width] != 0)
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
            Point rotPos = rotateIt(x, y, block.width, block.height, blockFalling.rot);

            //inside wall
            if(block.pattern[y][x] != 0) {
                if(blockFalling.x + rotPos.x >= width)
                    blockFalling.x--;
            }
        }
    }
}

bool checkLost() {
    for(int x = 0; x < width; x++) {
        if(grid[x] != 0)
            return true;
    }
    return false;
}

void init() {
    set_screen_mode(ScreenMode::hires);
}

// drawing
static void drawTile(int x, int y, bool isFall) {
    int tile = 0;
    if(isFall)
        tile = blockFalling.id;
    else
        tile = grid[x + y * width] - 1;

    screen.pen = blocks[tile].col;

    int blockSize = 10;
    screen.rectangle(Rect(x * blockSize, y * blockSize, blockSize, blockSize));

    Pen col = blocks[tile].col;

    col.r *= 0.7f;
    col.g *= 0.7f;
    col.b *= 0.7f;

    screen.pen = col;

    screen.h_span(Point( x      * blockSize    ,  y      * blockSize    ), blockSize);
    screen.h_span(Point( x      * blockSize    , (y + 1) * blockSize - 1), blockSize);
    screen.v_span(Point( x      * blockSize    ,  y      * blockSize    ), blockSize);
    screen.v_span(Point((x + 1) * blockSize - 1,  y      * blockSize    ), blockSize);
}

void render(uint32_t time) {
    screen.pen = Pen(0xFF,0xFF,0xFF);
    screen.clear();

    screen.pen = Pen(0, 0, 0);
    screen.h_span(Point(0, 10 * height), 10 *width);

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            if(grid[x + y * width] != 0) {
                drawTile(x, y, false);
            }
        }
    }

    //draw falling block
    if(blockFalling.id != -1) {
        auto &block = blocks[blockFalling.id];

        for(int y = 0; y < 2; y++) {
            for(int x = 0; x < 4; x++) {
                auto rotPos = rotateIt(x, y, block.width, block.height, blockFalling.rot);

                int rX = rotPos.x;
                int rY = rotPos.y;

                if(block.pattern[y][x])
                    drawTile(blockFalling.x + rX, blockFalling.y + rY, true);
            }
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
        blockFalling.id = blit::random() % 7;
        blockFalling.timer = 0;
        blockFalling.y = 0;
        blockFalling.x = 5 - blocks[blockFalling.id].width / 2;
        blockFalling.rot = 0;

        //check if it fits
        if(blockHitRot(0)) {
            blockFalling.id = -1;
        }
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
                blockFalling.x += move;
                move = 0;
            }
        }

        if(blockFalling.timer >= 20) {
            if(blockHit()) {
                placeBlock();

                checkLine();

                blockFalling.id = -1;
            } else {
                blockFalling.y++;
                blockFalling.timer = 0;
            }
        }
        else
            blockFalling.timer++;
    }
}