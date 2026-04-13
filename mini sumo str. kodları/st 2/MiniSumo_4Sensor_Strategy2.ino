// ===== 4 SENSÖR MİNİ SUMO - STRATEJİ 2 =====
// "YANDAN VURUŞ" STRATEJİSİ
// MOTOR YÖNLERİ TERSLENDİ
// Başlangıç : 2ms ileri + yerinde dönüp rakibi tara
// Saldırı   : 45° açıyla yandan vurma (önce %70 sonra %100)
// Tarama    : Geniş spiral, rastgele yön
// Çizgi     : Normal geri + dönüş
// Yan sensör: Keskin dönüş (bir teker durur)

#include <xmotionV3.h>

// ---- Rakip Sensör Pinleri (4 adet) ----
int LeftSensor       = 0;
int FrontLeftSensor  = 1;
int FrontRightSensor = 2;
int RightSensor      = 4;

// ---- Çizgi Sensör Pinleri ----
int LeftLine  = A2;
int RightLine = A4;

// ---- Kontrol Pinleri ----
int StartButton = A0;
int Led1 = 8;
int Led2 = 9;

// ---- Durum Değişkenleri ----
int LastValue = 0;

bool spiralDirection;
unsigned long spiralTimer = 0;
int spiralRadius = 20;

// ========== AYARLAR ==========
const int ATTACK_PHASE1   = 70;
const int ATTACK_PHASE2   = 100;
const int ANGLE_OFFSET    = 35;
const int SEARCH_SPEED    = 40;
const int SHARP_TURN_SPD  = 100;
const int LINE_RETREAT_MS = 150;
const int LINE_TURN_MS    = 180;
const int LINE_THRESHOLD  = 300;
const int SPIRAL_GROW_MS  = 800;

void setup() {
  xmotion.StopMotors(10);
  xmotion.ToggleLeds(100);

  pinMode(Led1, OUTPUT);
  pinMode(Led2, OUTPUT);
  pinMode(LeftSensor, INPUT);
  pinMode(FrontLeftSensor, INPUT);
  pinMode(FrontRightSensor, INPUT);
  pinMode(RightSensor, INPUT);
  pinMode(StartButton, INPUT);

  randomSeed(analogRead(A3));
  Serial.begin(9600);
}

void updateLeds() {
  digitalWrite(Led1, (digitalRead(LeftSensor) || digitalRead(FrontLeftSensor)) ? HIGH : LOW);
  digitalWrite(Led2, (digitalRead(RightSensor) || digitalRead(FrontRightSensor)) ? HIGH : LOW);
}

// ---- BAŞLANGIÇ: 2ms ileri + yerinde dön tarama ----
void startManeuver() {
  // İleri atılım (ters motor: - = ileri)
  xmotion.MotorControl(-100, -100);
  delay(2);

  bool scanDir = random(0, 2);
  unsigned long scanStart = millis();
  while (millis() - scanStart < 500) {
    if (digitalRead(FrontLeftSensor) || digitalRead(FrontRightSensor) ||
        digitalRead(LeftSensor) || digitalRead(RightSensor)) {
      return;
    }
    if (analogRead(LeftLine) < LINE_THRESHOLD || analogRead(RightLine) < LINE_THRESHOLD) {
      return;
    }
    if (scanDir) {
      xmotion.MotorControl(SHARP_TURN_SPD, -SHARP_TURN_SPD); // Sola dön
    } else {
      xmotion.MotorControl(-SHARP_TURN_SPD, SHARP_TURN_SPD); // Sağa dön
    }
    delay(1);
  }
}

// ---- ÇİZGİ YÖNETİMİ ----
bool handleLine() {
  bool left  = analogRead(LeftLine)  < LINE_THRESHOLD;
  bool right = analogRead(RightLine) < LINE_THRESHOLD;

  if (left && right) {
    xmotion.MotorControl(100, 100); // Geri
    delay(LINE_RETREAT_MS);
    if (random(0, 2)) {
      xmotion.MotorControl(100, -100);
    } else {
      xmotion.MotorControl(-100, 100);
    }
    delay(LINE_TURN_MS + 80);
    LastValue = 0;
    resetSpiral();
    return true;
  }
  if (left) {
    xmotion.MotorControl(100, 100); // Geri
    delay(LINE_RETREAT_MS);
    xmotion.MotorControl(-100, 100); // Sağa dön
    delay(LINE_TURN_MS);
    LastValue = 1;
    resetSpiral();
    return true;
  }
  if (right) {
    xmotion.MotorControl(100, 100); // Geri
    delay(LINE_RETREAT_MS);
    xmotion.MotorControl(100, -100); // Sola dön
    delay(LINE_TURN_MS);
    LastValue = 2;
    resetSpiral();
    return true;
  }
  return false;
}

void resetSpiral() {
  spiralRadius = 20;
  spiralTimer = millis();
  spiralDirection = random(0, 2);
}

// ---- 45° AÇILI SALDIRI ----
void angledAttack() {
  if (random(0, 2)) {
    // Sağdan açılı vur
    xmotion.MotorControl(-ATTACK_PHASE1, -(ATTACK_PHASE1 - ANGLE_OFFSET));
  } else {
    // Soldan açılı vur
    xmotion.MotorControl(-(ATTACK_PHASE1 - ANGLE_OFFSET), -ATTACK_PHASE1);
  }
  delay(80);

  // Tam güç itme
  xmotion.MotorControl(-ATTACK_PHASE2, -ATTACK_PHASE2);
  LastValue = 0;
}

// ---- SALDIRI + TARAMA ----
void attackLogic() {
  bool fl = digitalRead(FrontLeftSensor);
  bool fr = digitalRead(FrontRightSensor);
  bool sl = digitalRead(LeftSensor);
  bool sr = digitalRead(RightSensor);

  // İki ön sensör → 45° AÇILI SALDIRI
  if (fl && fr) {
    angledAttack();
    return;
  }

  // Ön-Sol → sola kır
  if (fl) {
    xmotion.MotorControl(-(ATTACK_PHASE1 - ANGLE_OFFSET), -ATTACK_PHASE1);
    LastValue = 2;
    return;
  }

  // Ön-Sağ → sağa kır
  if (fr) {
    xmotion.MotorControl(-ATTACK_PHASE1, -(ATTACK_PHASE1 - ANGLE_OFFSET));
    LastValue = 1;
    return;
  }

  // Sol → keskin sola (sağ teker ileri, sol durur)
  if (sl) {
    xmotion.MotorControl(0, -SHARP_TURN_SPD);
    delay(1);
    LastValue = 2;
    return;
  }

  // Sağ → keskin sağa (sol teker ileri, sağ durur)
  if (sr) {
    xmotion.MotorControl(-SHARP_TURN_SPD, 0);
    delay(1);
    LastValue = 1;
    return;
  }

  // RAKİP YOK → GENİŞ SPİRAL TARAMA
  if (millis() - spiralTimer > SPIRAL_GROW_MS) {
    spiralRadius += 5;
    if (spiralRadius > 60) spiralRadius = 60;
    spiralTimer = millis();
  }

  if (spiralDirection) {
    xmotion.MotorControl(-(SEARCH_SPEED - spiralRadius / 2), -(SEARCH_SPEED + spiralRadius / 2));
  } else {
    xmotion.MotorControl(-(SEARCH_SPEED + spiralRadius / 2), -(SEARCH_SPEED - spiralRadius / 2));
  }
}

void loop() {
  while (digitalRead(StartButton) == 0) {
    updateLeds();
    xmotion.UserLed2(100);
  }

  xmotion.CounterLeds(1000, 5);
  startManeuver();
  resetSpiral();

  while (1) {
    updateLeds();
    if (handleLine()) continue;
    attackLogic();
  }
}
