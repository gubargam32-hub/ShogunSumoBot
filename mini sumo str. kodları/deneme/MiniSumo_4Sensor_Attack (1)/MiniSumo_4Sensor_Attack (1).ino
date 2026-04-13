// ===== 4 SENSÖR MİNİ SUMO - SALDIRI STRATEJİSİ =====
// Shogun kodundaki çalışan motor yönleri baz alındı
// Forward / ArcTurn = ileri yön (Shogun'da çalışıyor)
// MotorControl(-100,-100) = geri (Shogun'da çalışıyor)
// Sony Remote Microstart uyumlu

#include <xmotionV3.h>

// ---- Rakip Sensör Pinleri (Shogun ile aynı) ----
int leftsens       = 0;   // Sol 90°
int frontleftsens  = 1;   // Ön-Sol ~45°
int frontrightsens = 2;   // Ön-Sağ ~45°
int rightsens      = 4;   // Sağ 90°

// ---- Çizgi Sensör Pinleri (Shogun ile aynı) ----
int RightLine = A1;
int LeftLine  = A2;

// ---- Kontrol Pinleri (Shogun ile aynı) ----
int start = A0;
int led1  = 8;
int led2  = 9;


// ---- Durum ----
int LastValue = 0;
unsigned long searchTimer = 0;
bool searchDir = false;

// ========== AYARLAR ==========
const int LINE_THRESHOLD = 300;

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

  Serial.begin(9600);
}

// ---- LED debug (Shogun ile aynı) ----
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

// ===================================================
// START: Sony Remote Microstart
// Normalde HIGH(1), kumandaya basınca LOW(0)
// ===================================================
void waitForStart() {
  xmotion.StopMotors(1);
  while (digitalRead(start) == 1) {
    xmotion.StopMotors(1);
    leds();
  }
  // LOW pulse geldi, basıldı
  delay(100);
}

// ---- ÇİZGİ KONTROLÜ (Shogun mantığı + çift okuma doğrulama) ----
bool handleLine() {
  // Sol çizgi + Sağ çizgi
  if (analogRead(LeftLine) < LINE_THRESHOLD && analogRead(RightLine) < LINE_THRESHOLD) {
    delay(3);
    if (analogRead(LeftLine) < LINE_THRESHOLD && analogRead(RightLine) < LINE_THRESHOLD) {
      xmotion.MotorControl(-100, -100);  // geri
      delay(200);
      xmotion.MotorControl(-100, 100);   // dön
      delay(220);
      LastValue = 0;
      return true;
    }
  }
  // Sadece sol çizgi
  else if (analogRead(LeftLine) < LINE_THRESHOLD) {
    delay(3);
    if (analogRead(LeftLine) < LINE_THRESHOLD) {
      xmotion.MotorControl(-100, -100);
      delay(200);
      xmotion.MotorControl(-100, 100);   // sağa dön
      delay(100);
      LastValue = 1;
      return true;
    }
  }
  // Sadece sağ çizgi
  else if (analogRead(RightLine) < LINE_THRESHOLD) {
    delay(3);
    if (analogRead(RightLine) < LINE_THRESHOLD) {
      xmotion.MotorControl(-100, -100);
      delay(200);
      xmotion.MotorControl(100, -100);   // sola dön
      delay(100);
      LastValue = 2;
      return true;
    }
  }
  return false;
}

// ---- SALDIRI + TARAMA ----
void attackLogic() {
  bool fl = digitalRead(frontleftsens);
  bool fr = digitalRead(frontrightsens);
  bool sl = digitalRead(leftsens);
  bool sr = digitalRead(rightsens);

  // === İki ön sensör → TAM GÜÇ İLERİ ===
  if (fl && fr) {
    xmotion.Forward(100, 1);
    LastValue = 0;
    return;
  }

  // === Ön-Sol → sola kır, saldır ===
  if (fl) {
    xmotion.ArcTurn(70, 100, 1);  // sol yavaş, sağ hızlı
    LastValue = 2;
    return;
  }

  // === Ön-Sağ → sağa kır, saldır ===
  if (fr) {
    xmotion.ArcTurn(100, 70, 1);  // sol hızlı, sağ yavaş
    LastValue = 1;
    return;
  }

  // === Sol sensör → keskin sola ===
  if (sl) {
    xmotion.ArcTurn(0, 100, 10);  // sol durur, sağ tam
    LastValue = 2;
    return;
  }

  // === Sağ sensör → keskin sağa ===
  if (sr) {
    xmotion.ArcTurn(100, 0, 10);  // sol tam, sağ durur
    LastValue = 1;
    return;
  }

  // === RAKİP YOK → TARAMA ===
  if (LastValue == 0) {
    // Zigzag ileri tarama
    if (millis() - searchTimer > 600) {
      searchDir = !searchDir;
      searchTimer = millis();
    }
    if (searchDir) {
      xmotion.ArcTurn(30, 45, 1);
    } else {
      xmotion.ArcTurn(45, 30, 1);
    }
  }
  else if (LastValue == 1) {
    // Son sağda gördü → sağa dönerek ara
    xmotion.ArcTurn(50, 15, 1);
  }
  else if (LastValue == 2) {
    // Son solda gördü → sola dönerek ara
    xmotion.ArcTurn(15, 50, 1);
  }
}

void loop() {
  // 1) Kumanda bekle (TEK BASIŞ)
  waitForStart();

  // 2) 5 saniye geri sayım
  xmotion.CounterLeds(1000, 5);
  searchTimer = millis();

  // 3) Savaş döngüsü - tekrar basılırsa durur
  while (digitalRead(start) == 1) {
    leds();
    if (handleLine()) continue;
    attackLogic();
  }

  // Kumandaya tekrar basıldı → dur
  xmotion.StopMotors(10);
  delay(50);
}
