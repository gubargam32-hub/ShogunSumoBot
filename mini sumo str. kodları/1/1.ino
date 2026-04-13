// ===== BAŞLAMA TESTİ =====
// Kumandaya BİR KEZ bas → 2sn bekle → %30 düz git
// Start pin durumunu LED ile gösterir

#include <xmotionV3.h>

int start = A0;
int led1  = 8;
int led2  = 9;

void setup() {
  xmotion.StopMotors(10);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(start, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  xmotion.StopMotors(1);

  // LED ile start pin durumunu göster (debug)
  // LED1 = pin HIGH ise yanar
  // LED2 = pin LOW ise yanar
  while (true) {
    int val = digitalRead(start);
    Serial.println(val);  // Serial monitörden de takip et

    if (val == 1) {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
    } else {
      digitalWrite(led1, LOW);
      digitalWrite(led2, HIGH);
    }

    // LOW gelince çık (kumandaya basıldı)
    if (val == 0) break;

    delay(10);
  }

  // Kumandaya basıldı! 2 saniye geri sayım
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  delay(2000);

  // %30 güç düz ileri git - kumandaya basılırsa dur
  while (digitalRead(start) == 1) {
    xmotion.Forward(30, 1);
  }

  // Durdu
  xmotion.StopMotors(10);
  delay(500);
}
