#include <Arduino.h>
#include <U8g2lib.h>
#include <HardwareTimer.h>
#include <STM32SD.h>

#ifndef FILE_READ
#define FILE_READ FA_READ
#endif

// Sound feature toggle - set to 0 if no buzzer is available
#define ENABLE_SOUND 1

#if ENABLE_SOUND
// Buzzer configuration
#define BUZZER_PIN PA8
HardwareTimer *buzzerTimer = new HardwareTimer(TIM1);

// Note frequency definitions (Hz)
#define NOTE_REST 0
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#endif

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
uint32_t highScore = 0;
const char* SAVE_FILE = "save.txt";
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

#if ENABLE_SOUND
// Sound state management
struct SoundNote {
  uint32_t frequency;
  uint32_t duration;
};

struct SoundSequence {
  SoundNote notes[8];
  uint8_t noteCount;
  uint8_t currentNote;
  unsigned long noteStartTime;
  bool isPlaying;
};

SoundSequence currentSound = {{{0, 0}}, 0, 0, 0, false};
#endif

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
void loadHighScore();
void saveHighScore();

#if ENABLE_SOUND
// Sound-related functions
void initBuzzer();
void updateSound();
void playSound(const SoundNote notes[], uint8_t noteCount);
void stopSound();
void playMoveSound();
void playRotateSound();
void playDropSound();
void playLineClearSound();
void playGameOverSound();
void playPauseSound();
#endif

void loadHighScore() {
  if (SD.exists(SAVE_FILE)) {
    File file = SD.open(SAVE_FILE, FILE_READ);
    if (file) {
      while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.startsWith("tetris ")) {
          highScore = line.substring(7).toInt();
          break;
        }
      }
      file.close();
    }
  }
}

void saveHighScore() {
  const char* TEMP_FILE = "temp.txt";
  
  if (SD.exists(TEMP_FILE)) SD.remove(TEMP_FILE);
  
  File tempFile = SD.open(TEMP_FILE, FILE_WRITE);
  if (!tempFile) return;

  bool found = false;
  
  if (SD.exists(SAVE_FILE)) {
    File originalFile = SD.open(SAVE_FILE, FILE_READ);
    if (originalFile) {
      while (originalFile.available()) {
        String line = originalFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        if (line.startsWith("tetris ")) {
          tempFile.print("tetris ");
          tempFile.println(highScore);
          found = true;
        } else {
          tempFile.println(line);
        }
      }
      originalFile.close();
    }
  }
  
  if (!found) {
    tempFile.print("tetris ");
    tempFile.println(highScore);
  }
  
  tempFile.close();
  
  SD.remove(SAVE_FILE);
  
  // Copy temp file back to save file
  tempFile = SD.open(TEMP_FILE, FILE_READ);
  File originalFile = SD.open(SAVE_FILE, FILE_WRITE);
  
  if (tempFile && originalFile) {
    uint8_t buf[64];
    while (tempFile.available()) {
      int n = tempFile.read(buf, sizeof(buf));
      originalFile.write(buf, n);
    }
  }
  
  if (tempFile) tempFile.close();
  if (originalFile) originalFile.close();
  
  SD.remove(TEMP_FILE);
}

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

#if ENABLE_SOUND
  initBuzzer();
#endif

  resetGame();
  drawMainScreen();
  drawInfoScreen();

  // Initialize SD Card
  if (SD.begin()) {
    loadHighScore();
  }
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
#if ENABLE_SOUND
      playRotateSound();
#endif
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
  
#if ENABLE_SOUND
  if (linesCleared > 0) {
    playLineClearSound();
  }
#endif
  
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

    snprintf(buffer, sizeof(buffer), "High:%lu", (score > highScore) ? score : highScore);
    infoDisp.drawStr(0, 60, buffer);
    snprintf(buffer, sizeof(buffer), "Score:%lu", score);
    infoDisp.drawStr(0, 70, buffer);
    snprintf(buffer, sizeof(buffer), "Level:%d", level);
    infoDisp.drawStr(0, 80, buffer);

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
#if ENABLE_SOUND
  updateSound();  // Non-blocking sound update
#endif

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
#if ENABLE_SOUND
    playPauseSound();
#endif
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
#if ENABLE_SOUND
        playDropSound();
#endif
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
        if (checkCollision(current)) {
          gameState = GAME_OVER;
          if (score > highScore) {
            highScore = score;
            saveHighScore();
          }
        }
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
      bool moved = false;

      // Horizontal movement
      if (leftState && !rightState) {
        temp.x--;
        moved = true;
      }
      if (rightState && !leftState) {
        temp.x++;
        moved = true;
      }

      // Soft drop
      if (digitalRead(BTN_DOWN))
      {
        temp.y++;
        score += 1;
        moved = true;
      }

      if (!checkCollision(temp)) {
        current = temp;
#if ENABLE_SOUND
        if (moved) {
          playMoveSound();
        }
#endif
      }
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
#if ENABLE_SOUND
        playPauseSound();  // Sound effect when resuming game
#endif
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
      if (checkCollision(current)) {
        gameState = GAME_OVER;
        if (score > highScore) {
          highScore = score;
          saveHighScore();
        }
#if ENABLE_SOUND
        playGameOverSound();
#endif
      }
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

#if ENABLE_SOUND
// Sound-related function implementations

void initBuzzer() {
  // Configure PWM pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize timer - default 1kHz frequency, 50% duty cycle
  buzzerTimer->setPWM(1, BUZZER_PIN, 1000, 50); // Channel 1, pin, frequency, duty cycle
  
  // Pause PWM output in initial state
  buzzerTimer->pause();
}

// Non-blocking sound update function
void updateSound() {
  if (!currentSound.isPlaying) return;
  
  unsigned long currentTime = millis();
  
  // Check if current note has finished playing
  if (currentTime - currentSound.noteStartTime >= currentSound.notes[currentSound.currentNote].duration) {
    // Stop current note
    buzzerTimer->pause();
    digitalWrite(BUZZER_PIN, LOW);
    
    // Move to next note
    currentSound.currentNote++;
    
    // Check if all notes have been played
    if (currentSound.currentNote >= currentSound.noteCount) {
      currentSound.isPlaying = false;
      return;
    }
    
    // Start playing next note
    currentSound.noteStartTime = currentTime + 10; // 10ms interval
    
    uint32_t frequency = currentSound.notes[currentSound.currentNote].frequency;
    if (frequency != NOTE_REST) {
      buzzerTimer->setOverflow(frequency, HERTZ_FORMAT);
      buzzerTimer->setCaptureCompare(1, 50, PERCENT_COMPARE_FORMAT);
      buzzerTimer->resume();
    }
  }
}

// Play sound sequence
void playSound(const SoundNote notes[], uint8_t noteCount) {
  if (noteCount == 0 || noteCount > 8) return;
  
  // Stop current sound effect
  stopSound();
  
  // Copy notes to current playback queue
  for (uint8_t i = 0; i < noteCount; i++) {
    currentSound.notes[i] = notes[i];
  }
  
  currentSound.noteCount = noteCount;
  currentSound.currentNote = 0;
  currentSound.noteStartTime = millis();
  currentSound.isPlaying = true;
  
  // Start playing first note
  uint32_t frequency = currentSound.notes[0].frequency;
  if (frequency != NOTE_REST) {
    buzzerTimer->setOverflow(frequency, HERTZ_FORMAT);
    buzzerTimer->setCaptureCompare(1, 50, PERCENT_COMPARE_FORMAT);
    buzzerTimer->resume();
  }
}

// Stop sound playback
void stopSound() {
  currentSound.isPlaying = false;
  buzzerTimer->pause();
  digitalWrite(BUZZER_PIN, LOW);
}

// Move sound effect - short low tone
void playMoveSound() {
  const SoundNote notes[] = {{NOTE_C4, 50}};
  playSound(notes, 1);
}

// Rotation sound effect - medium tone
void playRotateSound() {
  const SoundNote notes[] = {{NOTE_E4, 80}};
  playSound(notes, 1);
}

// Drop sound effect - quick descending tone
void playDropSound() {
  const SoundNote notes[] = {
    {NOTE_G4, 60},
    {NOTE_C4, 40}
  };
  playSound(notes, 2);
}

// Line clear sound effect - ascending scale
void playLineClearSound() {
  const SoundNote notes[] = {
    {NOTE_C5, 100},
    {NOTE_E5, 100},
    {NOTE_G5, 150}
  };
  playSound(notes, 3);
}

// Game over sound effect - descending scale
void playGameOverSound() {
  const SoundNote notes[] = {
    {NOTE_C5, 200},
    {NOTE_B4, 200},
    {NOTE_A4, 200},
    {NOTE_G4, 200},
    {NOTE_F4, 300},
    {NOTE_E4, 300},
    {NOTE_D4, 400},
    {NOTE_C4, 500}
  };
  playSound(notes, 8);
}

// Pause sound effect - simple two-note sequence
void playPauseSound() {
  const SoundNote notes[] = {
    {NOTE_A4, 150},
    {NOTE_F4, 150}
  };
  playSound(notes, 2);
}

#endif