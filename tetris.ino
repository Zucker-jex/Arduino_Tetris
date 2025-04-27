#include <Arduino.h>
#include <U8g2lib.h>

// Display configuration
U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI mainDisp(U8G2_R1, PA4, PB1, PB0);
U8G2_SSD1306_128X64_NONAME_1_HW_I2C infoDisp(U8G2_R1, U8X8_PIN_NONE, PB7, PB6);

// Key definitions
#define BTN_LEFT PE12
#define BTN_RIGHT PE8
#define BTN_DOWN PE11
#define BTN_UP PE9
#define BTN_ROT PE10

// Game parameters
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define BLOCK_SIZE 6

// Tetromino shapes
const uint8_t TETROMINO[7][4][4][4] = {
    {// I
      {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // 0°
      {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}}, // 90°
      {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}}, // 180°
      {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}}  // 270°
    },
    {// O
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}},
    {// T
      {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
    {// L
      {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
      {{1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
    {// J
      {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 1, 1, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {0, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}}},
    {// S
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
      {{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
      {{1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}}},
    {// Z
      {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {1, 1, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}}}};

// Wall Kick data (SRS standard)
const int8_t WALL_KICK[4][5][2] = {
    {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}}, // 0->R
    {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},     // R->2
    {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},    // 2->L
    {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}}   // L->0
};

// Wall Kick for I shape
const int8_t I_WALL_KICK[4][5][2] = {
    {{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}}, // 0->R
    {{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}}, // R->2
    {{0, 0}, {-1, 0}, {2, 0}, {-1, -2}, {2, 1}}, // 2->L
    {{0, 0}, {1, 0}, {-2, 0}, {1, 2}, {-2, -1}}  // L->0
};

struct Tetromino
{
  int8_t x, y;
  uint8_t type;
  uint8_t rotation;
};

enum GameState
{
  SPLASH,
  PLAYING,
  PAUSE,
  GAME_OVER
};
GameState gameState = SPLASH;

uint8_t gameField[FIELD_HEIGHT][FIELD_WIDTH] = {0};
Tetromino current, next, ghost;
uint32_t score = 0;
uint16_t lines = 0;
uint8_t level = 1;
unsigned long lastFall = 0;
bool holdUsed = false;
bool hardDropPressed = false;

// Bag randomizer related variables
uint8_t currentBag[7];
uint8_t currentBagIndex = 7;

// Input control states
bool rotKeyPressed = false;
unsigned long rotKeyLastTime = 0;
bool leftRightCombo = false;

// Function declarations
void generateNewPiece(Tetromino &t);
void drawMainScreen();
void drawInfoScreen();
bool checkCollision(const Tetromino &t);
void mergePiece();
uint8_t clearLines();
void rotatePiece(bool clockwise);
void updateGhost();
void resetGame();
void generateNewBag(uint8_t *bag);

void setup()
{
  mainDisp.begin();
  infoDisp.begin();
  randomSeed(analogRead(PA0));

  pinMode(BTN_LEFT, INPUT_PULLDOWN);
  pinMode(BTN_RIGHT, INPUT_PULLDOWN);
  pinMode(BTN_DOWN, INPUT_PULLDOWN);
  pinMode(BTN_UP, INPUT_PULLDOWN);
  pinMode(BTN_ROT, INPUT_PULLDOWN);

  resetGame();
}

bool checkCollision(const Tetromino &t)
{
  for (int8_t y = 0; y < 4; y++)
  {
    for (int8_t x = 0; x < 4; x++)
    {
      if (TETROMINO[t.type][t.rotation][y][x])
      {
        int8_t fieldX = t.x + x;
        int8_t fieldY = t.y + y;
        if (fieldX < 0 || fieldX >= FIELD_WIDTH ||
            fieldY >= FIELD_HEIGHT ||
            (fieldY >= 0 && gameField[fieldY][fieldX]))
          return true;
      }
    }
  }
  return false;
}

void rotatePiece(bool clockwise)
{
  Tetromino temp = current;
  uint8_t newRotation = clockwise ? (current.rotation + 1) % 4 : (current.rotation + 3) % 4;

  const int8_t (*kickTable)[5][2] = (current.type == 0) ? &I_WALL_KICK[current.rotation] : &WALL_KICK[current.rotation];

  for (uint8_t i = 0; i < 5; i++)
  {
    temp.x = current.x + (*kickTable)[i][0];
    temp.y = current.y + (*kickTable)[i][1];
    temp.rotation = newRotation;

    if (!checkCollision(temp))
    {
      current = temp;
      return;
    }
  }
}

void generateNewPiece(Tetromino &t)
{
  if (currentBagIndex >= 7)
  {
    generateNewBag(currentBag);
    currentBagIndex = 0;
  }
  t.type = currentBag[currentBagIndex];
  currentBagIndex++;
  t.x = FIELD_WIDTH / 2 - 2;
  t.y = -2;
  t.rotation = 0;
}

void mergePiece()
{
  for (uint8_t y = 0; y < 4; y++)
  {
    for (uint8_t x = 0; x < 4; x++)
    {
      if (TETROMINO[current.type][current.rotation][y][x])
      {
        int8_t fieldY = current.y + y;
        if (fieldY >= 0)
          gameField[fieldY][current.x + x] = current.type + 1;
      }
    }
  }
}

uint8_t clearLines()
{
  uint8_t linesCleared = 0;
  for (int8_t y = FIELD_HEIGHT - 1; y >= 0; y--)
  {
    bool full = true;
    for (uint8_t x = 0; x < FIELD_WIDTH; x++)
    {
      if (!gameField[y][x])
      {
        full = false;
        break;
      }
    }

    if (full)
    {
      memmove(&gameField[1], &gameField[0], y * sizeof(gameField[0]));
      memset(&gameField[0], 0, sizeof(gameField[0]));
      linesCleared++;
      y++;
    }
  }
  return linesCleared;
}

void updateGhost()
{
  ghost = current;
  while (!checkCollision(ghost))
  {
    ghost.y++;
  }
  ghost.y--;
}

void resetGame()
{
  memset(gameField, 0, sizeof(gameField));
  generateNewPiece(current);
  generateNewPiece(next);
  score = 0;
  lines = 0;
  level = 1;
  holdUsed = false;
  hardDropPressed = false;
}

void generateNewBag(uint8_t *bag)
{
  for (int i = 0; i < 7; i++)
  {
    bag[i] = i;
  }
  for (int i = 6; i > 0; i--)
  {
    int j = random(i + 1);
    uint8_t temp = bag[i];
    bag[i] = bag[j];
    bag[j] = temp;
  }
}

void drawMainScreen()
{
  mainDisp.firstPage();
  do
  {
    mainDisp.drawFrame(0, 0,
                       FIELD_WIDTH * BLOCK_SIZE,
                       FIELD_HEIGHT * BLOCK_SIZE);

    for (uint8_t y = 0; y < FIELD_HEIGHT; y++)
    {
      for (uint8_t x = 0; x < FIELD_WIDTH; x++)
      {
        if (gameField[y][x])
        {
          mainDisp.drawBox(x * BLOCK_SIZE, y * BLOCK_SIZE,
                           BLOCK_SIZE - 1, BLOCK_SIZE - 1);
        }
      }
    }

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        if (TETROMINO[ghost.type][ghost.rotation][y][x])
        {
          mainDisp.drawFrame(ghost.x * BLOCK_SIZE + x * BLOCK_SIZE,
                             ghost.y * BLOCK_SIZE + y * BLOCK_SIZE,
                             BLOCK_SIZE - 1, BLOCK_SIZE - 1);
        }
      }
    }

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        if (TETROMINO[current.type][current.rotation][y][x])
        {
          mainDisp.drawBox(current.x * BLOCK_SIZE + x * BLOCK_SIZE,
                           current.y * BLOCK_SIZE + y * BLOCK_SIZE,
                           BLOCK_SIZE - 1, BLOCK_SIZE - 1);
        }
      }
    }

    if (gameState == PAUSE)
    {
      mainDisp.setFont(u8g2_font_profont22_tf);
      mainDisp.drawStr(1, 30, "PAUSE");
    }
    else if (gameState == GAME_OVER)
    {
      mainDisp.setFont(u8g2_font_profont22_tf);
      mainDisp.drawStr(8, 30, "GAME");
      mainDisp.drawStr(8, 50, "OVER");
    }
    else if (gameState == SPLASH)
    {
      mainDisp.setFont(u8g2_font_profont12_tf);
      mainDisp.drawStr(11, 30, "Tetris");
    }
  } while (mainDisp.nextPage());
}

void drawInfoScreen()
{
  char buffer[20];
  infoDisp.firstPage();
  do
  {
    infoDisp.setFont(u8g2_font_profont12_tf);

    infoDisp.drawStr(0, 15, "Next:");
    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        if (TETROMINO[next.type][0][y][x])
        {
          infoDisp.drawBox(40 + x * BLOCK_SIZE,
                           y * BLOCK_SIZE + 2,
                           BLOCK_SIZE - 1, BLOCK_SIZE - 1);
        }
      }
    }

    snprintf(buffer, sizeof(buffer), "Score:%lu", score);
    infoDisp.drawStr(0, 50, buffer);
    snprintf(buffer, sizeof(buffer), "Level:%d", level);
    infoDisp.drawStr(0, 60, buffer);

    if (gameState == SPLASH)
    {
      infoDisp.setFont(u8g2_font_profont10_tf);
      infoDisp.drawStr(0, 30, "Press ROT to");
      infoDisp.drawStr(12, 38, "Start");
    }
  } while (infoDisp.nextPage());
}

void loop()
{
  static unsigned long lastInput = 0;
  bool currentRotState = digitalRead(BTN_ROT);
  bool leftState = digitalRead(BTN_LEFT);
  bool rightState = digitalRead(BTN_RIGHT);
  bool upState = digitalRead(BTN_UP);

  // Pause detection
  if (gameState == PLAYING && leftState && rightState && !leftRightCombo)
  {
    gameState = PAUSE;
    leftRightCombo = true;
  }
  else if (!(leftState && rightState))
  {
    leftRightCombo = false;
  }

  // Rotation detection
  if (gameState == PLAYING && currentRotState)
  {
    if (!rotKeyPressed || (millis() - rotKeyLastTime > 200))
    {
      rotatePiece(true);
      rotKeyLastTime = millis();
      rotKeyPressed = true;
    }
  }
  else
  {
    rotKeyPressed = false;
  }

  // Hard drop handling (immediate response)
  if (gameState == PLAYING)
  {
    if (upState && !hardDropPressed)
    {
      Tetromino temp = current;
      bool moved = false;
      while (!checkCollision(temp))
      {
        current = temp;
        temp.y++;
        score += 2;
        moved = true;
      }
      if (moved)
      {
        mergePiece();
        uint8_t cleared = clearLines();
        if (cleared > 0)
        {
          score += cleared * 100 * level;
          lines += cleared;
          level = 1 + lines / 10;
        }
        current = next;
        generateNewPiece(next);
        if (checkCollision(current))
          gameState = GAME_OVER;
      }
      hardDropPressed = true;
    }
    // Reset state when released
    if (!upState)
      hardDropPressed = false;
  }

  // Main input handling (100ms interval)
  if (millis() - lastInput > 100)
  {
    if (gameState == PLAYING)
    {
      Tetromino temp = current;

      // Horizontal movement
      if (leftState && !rightState)
        temp.x--;
      if (rightState && !leftState)
        temp.x++;

      // Soft drop
      if (digitalRead(BTN_DOWN))
      {
        temp.y++;
        score += 1;
      }

      if (!checkCollision(temp))
        current = temp;
    }

    // State control
    if (currentRotState && !rotKeyPressed)
    {
      bool stateChanged = false;
      switch (gameState)
      {
      case SPLASH:
        gameState = PLAYING;
        stateChanged = true;
        break;
      case GAME_OVER:
        resetGame();
        gameState = PLAYING;
        stateChanged = true;
        break;
      case PAUSE:
        gameState = PLAYING;
        stateChanged = true;
        break;
      default:
        break;
      }
      if (stateChanged)
        rotKeyPressed = true;
    }

    lastInput = millis();
  }

  // Auto fall
  if (gameState == PLAYING && millis() - lastFall > (1000 / level))
  {
    Tetromino temp = current;
    temp.y++;

    if (checkCollision(temp))
    {
      mergePiece();
      uint8_t cleared = clearLines();
      if (cleared > 0)
      {
        score += cleared * 100 * level;
        lines += cleared;
        level = 1 + lines / 10;
      }
      current = next;
      generateNewPiece(next);
      if (checkCollision(current))
        gameState = GAME_OVER;
    }
    else
    {
      current = temp;
    }
    lastFall = millis();
  }

  updateGhost();
  drawMainScreen();
  drawInfoScreen();
}