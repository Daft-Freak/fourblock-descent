#include "game.hpp"
#include "assets.hpp"

using namespace blit;

static const int width = 10, height = 15;
static uint8_t grid[width * height]{0};

struct Block {
    bool pattern[2][4];
    int width = 0, height = 0;
};

Block blocks[]{
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

static int move = 0, rotate = 0;

int score = 0;
bool lastWasTetris = false;

bool gameEnded = false;

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
            Point rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, blockFalling.rot);

            if(block.pattern[y][x]) {
                grid[rotPos.x + rotPos.y * width] = blockFalling.id + 1;
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
                if(rotPos.x + move >= width)
                    return true;

                if(rotPos.x + move < 0)
                    return true;

                //block block beside
                if(rotPos.x + move < width) {
                    if(grid[rotPos.x + move + rotPos.y * width] != 0)
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
                if(rotPos.y >= height - 1)
                    return true;

                //block under
                if(rotPos.y + 1 < height) {
                    if(grid[rotPos.x + (rotPos.y + 1) * width] != 0)
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
                if(grid[rotPos.x + rotPos.y * width] != 0)
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
                if(rotPos.x >= width)
                    blockFalling.pos.x--;
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
    set_screen_mode(ScreenMode::lores);

    screen.sprites = Surface::load(asset_tetris_sprites);
}

// drawing
static void drawTile(int x, int y, bool isFall) {
    int tile = 0;
    if(isFall)
        tile = blockFalling.id;
    else
        tile = grid[x + y * width] - 1;

    int blockSize = 8;
    screen.sprite(tile, Point(x * blockSize, y * blockSize));
}

void render(uint32_t time) {
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.pen = Pen(0xFF,0xFF,0xFF);
    screen.rectangle(Rect(0, 0, width * 8, height * 8));

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
                auto rotPos = blockFalling.pos + rotateIt(Point(x, y), block.width, block.height, blockFalling.rot);

                if(block.pattern[y][x])
                    drawTile(rotPos.x, rotPos.y, true);
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
        blockFalling.pos.y = 0;
        blockFalling.pos.x = 5 - blocks[blockFalling.id].width / 2;
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
                blockFalling.pos.x += move;
                move = 0;
            }
        }

        if(blockFalling.timer >= 20) {
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