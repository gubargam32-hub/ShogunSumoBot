// ===== 4 SENSÖR MİNİ SUMO - STRATEJİ 2 =====
// "YANDAN VURUŞ"
// Başlangıç : 2ms ileri + yerinde dönüp rakibi tara
// Saldırı   : 45° açıyla yandan vurma (önce %70 sonra %100)
// Tarama    : Geniş spiral, rastgele yön
// Çizgi     : Normal geri + dönüş
// Yan sensör: Keskin dönüş (bir teker durur)
// Start/Stop: Toggle (1.basış=başla, 2.basış=dur)

#include <xmotionV3.h>

// ---- Rakip Sensör Pinleri (4 adet) ----
int leftsens       = 0;
int frontleftsens  = 1;
int frontrightsens = 2;
int rightsens      = 4;

// ---- Çizgi Sensör Pinleri ----
int RightLine = A1;
int LeftLine  = A2;

// ---- Kontrol Pinleri ----
int start = A0;
int led1  = 8;
int led2  = 9;

// ---- Durum ----
int LastValue = 0;
bool spiralDirection;
unsigned long spiralTimer = 0;
int spiralRadius = 20;

// ========== AYARLAR ==========
const int ATTACK_PHASE1  = 70;
const int ATTACK_PHASE2  = 100;
const int ANGLE_OFFSET   = 35;
const int SEARCH_SPEED   = 40;
const int SHARP_TURN_SPD = 100;
const int LINE_THRESHOLD = 300;
const int SPIRAL_GROW_MS = 800;

void setup() {
  xmotion.StopMotors(10);
  xmotion.ToggleLeds(100);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(leftsens, INPUT);
  pinMode(frontleftsens, INPUT);
  pinMode(frontrightsens, INPUT);
  pinMode(rightsens, INPUT);
  pinMode(RightLine, INPUT);
  pinMode(LeftLine, INPUT);
  pinMode(start, INPUT_PULLUP);

  randomSeed(analogRead(A3));
  Serial.begin(9600);
}

// ---- LED debug ----
void leds() {
  if (digitalRead(leftsens) == 1 || digitalRead(frontleftsens) == 1 || analogRead(LeftLine) < LINE_THRESHOLD)
    digitalWrite(led1, HIGH);
  else
    digitalWrite(led1, LOW);

  if (digitalRead(rightsens) == 1 || digitalRead(frontrightsens) == 1 || analogRead(RightLine) < LINE_THRESHOLD)
    digitalWrite(led2, HIGH);
  else
    digitalWrite(led2, LOW);
}

// ---- Spiral sıfırlama ----
void resetSpiral() {
  spiralRadius = 20;
  spiralTimer = millis();
  spiralDirection = random(0, 2);
}

// ---- BAŞLANGIÇ MANEVRASİ: 2ms ileri + yerinde dön tarama ----
void startManeuver() {
  // Kısa ileri atılım
  xmotion.Forward(100, 2);

  // Rastgele yön seç ve yerinde dönerek tara
  bool scanDir = random(0, 2);
  unsigned long scanStart = millis();

  while (millis() - scanStart < 500) {
    // Rakip görürsen hemen çık
    if (digitalRead(frontleftsens) || digitalRead(frontrightsens) ||
        digitalRead(leftsens) || digitalRead(rightsens)) {
      return;
    }
    // Çizgi görürsen hemen çık
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

// ---- ÇİZGİ KONTROLÜ (çift okuma doğrulama) ----
bool handleLine() {
  if (analogRead(LeftLine) < LINE_THRESHOLD && analogRead(RightLine) < LINE_THRESHOLD) {
    delay(3);
    if (analogRead(LeftLine) < LINE_THRESHOLD && analogRead(RightLine) < LINE_THRESHOLD) {
      xmotion.MotorControl(-100, -100);
      delay(200);
      if (random(0, 2)) {
        xmotion.MotorControl(-100, 100);
      } else {
        xmotion.MotorControl(100, -100);
      }
      delay(250);
      LastValue = 0;
      resetSpiral();
      return true;
    }
  }
  else if (analogRead(LeftLine) < LINE_THRESHOLD) {
    delay(3);
    if (analogRead(LeftLine) < LINE_THRESHOLD) {
      xmotion.MotorControl(-100, -100);
      delay(200);
      xmotion.MotorControl(-100, 100);
      delay(100);
      LastValue = 1;
      resetSpiral();
      return true;
    }
  }
  else if (analogRead(RightLine) < LINE_THRESHOLD) {
    delay(3);
    if (analogRead(RightLine) < LINE_THRESHOLD) {
      xmotion.MotorControl(-100, -100);
      delay(200);
      xmotion.MotorControl(100, -100);
      delay(100);
      LastValue = 2;
      resetSpiral();
      return true;
    }
  }
  return false;
}

// ---- 45° AÇILI SALDIRI ----
void angledAttack() {
  // Faz 1: %70 güçle açılı yaklaşma
  if (random(0, 2)) {
    // Sağdan açılı vur
    xmotion.ArcTurn(ATTACK_PHASE1, ATTACK_PHASE1 - ANGLE_OFFSET, 1);
  } else {
    // Soldan açılı vur
    xmotion.ArcTurn(ATTACK_PHASE1 - ANGLE_OFFSET, ATTACK_PHASE1, 1);
  }
  delay(80);

  // Faz 2: Tam güç itme
  xmotion.Forward(ATTACK_PHASE2, 1);
  LastValue = 0;
}

// ---- SALDIRI + TARAMA ----
void attackLogic() {
  bool fl = digitalRead(frontleftsens);
  bool fr = digitalRead(frontrightsens);
  bool sl = digitalRead(leftsens);
  bool sr = digitalRead(rightsens);

  // === İki ön sensör → 45° AÇILI SALDIRI ===
  if (fl && fr) {
    angledAttack();
    return;
  }

  // === Ön-Sol → sola kır, saldır ===
  if (fl) {
    xmotion.ArcTurn(ATTACK_PHASE1 - ANGLE_OFFSET, ATTACK_PHASE1, 1);
    LastValue = 2;
    return;
  }

  // === Ön-Sağ → sağa kır, saldır ===
  if (fr) {
    xmotion.ArcTurn(ATTACK_PHASE1, ATTACK_PHASE1 - ANGLE_OFFSET, 1);
    LastValue = 1;
    return;
  }

  // === Sol → keskin sola (sol durur, sağ tam) ===
  if (sl) {
    xmotion.ArcTurn(0, SHARP_TURN_SPD, 10);
    LastValue = 2;
    return;
  }

  // === Sağ → keskin sağa (sol tam, sağ durur) ===
  if (sr) {
    xmotion.ArcTurn(SHARP_TURN_SPD, 0, 10);
    LastValue = 1;
    return;
  }

  // === RAKİP YOK → GENİŞ SPİRAL TARAMA ===
  if (millis() - spiralTimer > SPIRAL_GROW_MS) {
    spiralRadius += 5;
    if (spiralRadius > 60) spiralRadius = 60;
    spiralTimer = millis();
  }

  if (spiralDirection) {
    xmotion.ArcTurn(SEARCH_SPEED - spiralRadius / 2, SEARCH_SPEED + spiralRadius / 2, 1);
  } else {
    xmotion.ArcTurn(SEARCH_SPEED + spiralRadius / 2, SEARCH_SPEED - spiralRadius / 2, 1);
  }
}

void loop() {
  // 1) Kumanda bekle (pin HIGH iken bekle)
  xmotion.StopMotors(1);
  while (digitalRead(start) == 1) {
    xmotion.StopMotors(1);
    leds();
  }
  delay(100);

  // 2) 2 saniye geri sayım
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  delay(2000);

  // 3) Başlangıç manevrası (2ms ileri + dönüp tara)
  startManeuver();
  resetSpiral();

  // 4) Savaş (pin LOW iken çalış, tekrar basılınca HIGH → dur)
  while (digitalRead(start) == 0) {
    leds();
    if (handleLine()) continue;
    attackLogic();
  }

  // Durdu
  xmotion.StopMotors(10);
  delay(500);
}
