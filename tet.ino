#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definision：LCD
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

// Definision：ジョイスティック
#define JOY_X 27
#define JOY_Y 28
#define JOY_THRESHOLD 300

bool OnOff=0;

// 定義：シフトレジスタ
#define DATA_PIN 3
#define CLOCK_PIN 6
#define LATCH_PIN 7


// MatrixLED使用関数宣言
void drawPattern_Xms(uint8_t* pattern, int x);
void drawPattern_8ms(uint8_t* pattern);
void clearMatrixLED(void);


// グローバル変数
byte tetrisGrid[8] = {0};  // ゲームのグリッド（8x8）
byte tetrisGridBg[8] = {0}; // 追加；blockを除くBackground (8x8)

const byte tetriminoes[7][4] = {
  {0b0000, 0b1110, 0b0100, 0b0000},  // T-shape
  {0b1100, 0b1100, 0b0000, 0b0000},  // O-shape
  {0b1000, 0b1110, 0b0000, 0b0000},  // L-shape
  {0b0010, 0b1110, 0b0000, 0b0000},  // J-shape
  {0b0110, 0b1100, 0b0000, 0b0000},  // S-shape
  {0b1100, 0b0110, 0b0000, 0b0000},  // Z-shape
  {0b1111, 0b0000, 0b0000, 0b0000}   // I-shape
};
byte block[4];  // 現在のブロック
int blockX = 3, blockY = 0;  // ブロック位置
unsigned long lastFallTime = 0, lastUpdateTime = 0;
int fallSpeed = 300;  // 落下速度
int score = 0;
int timeLimit = 60;  // 残り時間（s）
bool isGameOver = false;

void updateMatrix() {
  // digitalWrite(LATCH_PIN, LOW);
  for (int row = 0; row < 8; row++) {
    digitalWrite(LATCH_PIN, LOW); // コード移動
    // shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ~(1 << row));  // ROW選択
    // shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, tetrisGrid[row]);  // COLデータ
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, (1 << row));  // ROW選択
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, ~tetrisGrid[row]);  // COLデータ
    digitalWrite(LATCH_PIN, HIGH); // コード移動
    delay(1); // 追加
  }
  // digitalWrite(LATCH_PIN, HIGH);
}

// シフトレジスタを使用してLEDマトリックスを更新
/*void updateMatrix() {
  digitalWrite(LATCH_PIN, LOW);
  for (int row = 0; row < 8; row++) {
    // shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ~(1 << row));  // ROW選択
    // shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, tetrisGrid[row]);  // COLデータ
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, (1 << row));  // ROW選択
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, tetrisGrid[row]);  // COLデータ
  }
  digitalWrite(LATCH_PIN, HIGH);
}
*/

// ジョイスティックの状態チェック
void checkJoystick() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  if (xVal < JOY_THRESHOLD) {
    moveBlock(-1);  // 左
  } else if (xVal > 1023 - JOY_THRESHOLD) {
    moveBlock(1);  // 右
  }

  if (yVal < JOY_THRESHOLD) {
    rotateBlock();  // 回転
  }

  if (yVal > 1023 - JOY_THRESHOLD) {
    fallSpeed = 50;  // 高速落下
  } else {
    fallSpeed = 500;  // 通常速度
  }
}

void resetGame() {
  isGameOver = false;
  score = 0;
  timeLimit = 60;
  memset(tetrisGrid, 0, sizeof(tetrisGrid));
  memset(tetrisGridBg, 0, sizeof(tetrisGridBg));
  spawnNewBlock();
}


bool canMove(int newX, int newY, byte newBlock[4] = nullptr) {
  byte *currentBlock = newBlock ? newBlock : block;
  // genTetrisGridBg(currentBlock); // 竹内
  for (int y = 0; y < 4; y++) {
    if (currentBlock[y] == 0) continue;
    for (int x = 0; x < 4; x++) {
      if (currentBlock[y] & (1 << x)) {
        int gridX = newX + x;
        int gridY = newY + y;
        // if (gridX < 0 || gridX >= 8 || gridY >= 8 || (gridY >= 0 && tetrisGrid[gridY] & (1 << gridX))) {
        //   return false;
        // }
        // X方向に移動可能か（分離）
        if (gridX < 0 || gridX >= 8 ) {
          return false;
        }
        // Y方向に移動可能か（分離＋tetrisGridBgでblock動けるかを判定し、動けない場合、最終block含めた新たなbackground作成）
        if (gridY >= 8 || (gridY >= 0 && tetrisGridBg[gridY] & (1 << gridX))) {
          memcpy(tetrisGridBg, tetrisGrid, sizeof(tetrisGrid));
          return false;
        }
      }
    }
  }
  return true;
}

// 移動可能か確認
/* bool canMove(int newX, int newY, byte newBlock[4] = nullptr) {
  byte *currentBlock = newBlock ? newBlock : block;
  for (int y = 0; y < 4; y++) {
    if (currentBlock[y] == 0) continue;
    for (int x = 0; x < 4; x++) {
      if (currentBlock[y] & (1 << x)) {
        int gridX = newX + x;
        int gridY = newY + y;
        if (gridX < 0 || gridX >= 8 || gridY >= 8 || (gridY >= 0 && tetrisGrid[gridY] & (1 << gridX))) {
          return false;
        }
      }
    }
  }
  return true;
}
*/

// ブロックの移動
void moveBlock(int direction) {
  if (canMove(blockX + direction, blockY)) {
    blockX += direction;
  }
}

// ブロックの回転
void rotateBlock() {
  byte rotated[4] = {0};
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (block[y] & (1 << x)) {
        rotated[x] |= (1 << (3 - y));
      }
    }
  }
  if (canMove(blockX, blockY, rotated)) {
    memcpy(block, rotated, sizeof(block));
  }
}


// ブロックを配置
void placeBlock() {
  memcpy(tetrisGrid, tetrisGridBg, sizeof(tetrisGridBg)); // 追加；以下ではBgコピーにblockを追加
  for (int y = 0; y < 4; y++) {
    if (block[y] == 0) continue;
    for (int x = 0; x < 4; x++) {
      if (block[y] & (1 << x)) {
        int gridX = blockX + x;
        int gridY = blockY + y;
        if (gridY >= 0) {
          tetrisGrid[gridY] |= (1 << gridX);
        }
      }
    }
  }
  clearRows();
}

// 行を消去
void clearRows() {
  int rowsCleared = 0;
  for (int y = 0; y < 8; y++) {
    if (tetrisGrid[y] == 0b11111111) {
      rowsCleared++;
      for (int i = y; i > 0; i--) {
        tetrisGrid[i] = tetrisGrid[i - 1];
      }
      tetrisGrid[0] = 0;
    }
  }
  score += rowsCleared * 100;
}

// 新しいブロックを生成
void spawnNewBlock() {
  blockX = 3;
  blockY = 0;
  memcpy(block, tetriminoes[random(0, 7)], sizeof(block));
  if (!canMove(blockX, blockY)) {
    isGameOver = true;
  }
}

// LCDディスプレイを更新
void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(timeLimit);
  lcd.print("s");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
}


void setup() {
  // LCD初期化
  lcd.init();
  lcd.backlight();

  
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  randomSeed(analogRead(A2));  // ランダムシード
  spawnNewBlock();
  placeBlock(); // 追加
  lastFallTime = millis(); // 追加。始まり時刻を設定。
}


void loop() {
  if (isGameOver) {
    lcd.setCursor(0, 0);
    lcd.print(" GAME OVER ");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
    //while (true);
    /*while (true) { // 竹内
    updateMatrix(); // 竹内
    }
    */
    unsigned long gameOverStartTime = millis(); // ゲームオーバーの開始時間を記録
  while (millis() - gameOverStartTime < 1000) { // 10秒待機
    updateMatrix();
  }
  resetGame(); // ゲームをリセット

  }

  if (millis() - lastUpdateTime >= 1000) {
    lastUpdateTime = millis();
    timeLimit--;
    if (timeLimit <= 0) {
      isGameOver = true;
    }
  }

  //checkJoystick();
  if (millis() - lastFallTime >= fallSpeed) {
    lastFallTime = millis();
    if (!canMove(blockX, blockY + 1)) {
      // placeBlock(); //コメントアウト。後ろに移動
      spawnNewBlock();
      placeBlock(); //前から移動
    } else {
      checkJoystick();
      blockY++;
      placeBlock(); //追加
    }
  }

  updateMatrix();
  updateLCD();
}

