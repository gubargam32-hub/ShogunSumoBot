// ───────────────────────────────────────────────────────────────────────────
// FAZ 3: Çizgi Sensörü Tepkisi (Pinpon Etkisi Çözüldü)
void handleLineSensors() {
  bool leftLine  = (analogRead(LeftLine)  < LINE_THRESHOLD);
  bool rightLine = (analogRead(RightLine) < LINE_THRESHOLD);

  if (leftLine && rightLine) {
    xmotion.MotorControl(-100, -100); delay(120); // 250'den 120'ye düşürüldü (Ufak geri adım)
    xmotion.MotorControl(-100, 100);  delay(150); // 200'den 150'ye düşürüldü
    LastValue = 2;
    lastSeenTime = millis();
  } else if (leftLine) {
    xmotion.MotorControl(-100, -100); delay(100); // 200'den 100'e düşürüldü
    xmotion.MotorControl(100, -100);  delay(120); // Sağa daha kısa dön
    LastValue = 3;
    lastSeenTime = millis();
  } else if (rightLine) {
    xmotion.MotorControl(-100, -100); delay(100); // 200'den 100'e düşürüldü
    xmotion.MotorControl(-100, 100);  delay(120); // Sola daha kısa dön
    LastValue = 1;
    lastSeenTime = millis();
  }
}

// ───────────────────────────────────────────────────────────────────────────
// FAZ 3: Rakip Arama ve Saldırı (Vurup Kaçma Çözüldü)
void handleEnemySensors() {

  bool enemyDetected = false;

  if (digitalRead(frontleftsens) && digitalRead(frontrightsens)) {
    xmotion.MotorControl(100, 100);
    LastValue = 2;
    enemyDetected = true;
    delay(50); // Sarsıntıda sensör körlüğünü (flicker) yutmak için mini kilitlenme!
  }
  else if (digitalRead(frontleftsens)) {
    xmotion.MotorControl(60, 100); // Sola sert kavis
    LastValue = 1;
    enemyDetected = true;
  }
  else if (digitalRead(frontrightsens)) {
    xmotion.MotorControl(100, 60); // Sağa sert kavis
    LastValue = 3;
    enemyDetected = true;
  }
  else if (digitalRead(leftsens)) {
    xmotion.MotorControl(-100, 100);
    LastValue = 4;
    enemyDetected = true;
    delay(30); // Dönüş momentumunu toplamak için ufak kilit
  }
  else if (digitalRead(rightsens)) {
    xmotion.MotorControl(100, -100);
    LastValue = 5;
    enemyDetected = true;
    delay(30); // Dönüş momentumunu toplamak için ufak kilit
  }

  // --- HAFIZA VE ZAMAN AŞIMI YÖNETİMİ ---
  if (enemyDetected) {
    lastSeenTime = millis();
  }
  else {
    // Hayalet Kovalama Koruması
    if (millis() - lastSeenTime > 1000) {
      LastValue = 2;
    }

    // Hafızaya göre kör arama (Hızlar Optimize Edildi)
    switch (LastValue) {
      case 2: xmotion.MotorControl(100, 100); break; // DÜZELTİLDİ: 40,40 idi! Sensör anlık rakibi kaybederse itmeyi asla bırakma, %100 aban!
      case 1: xmotion.MotorControl(40, 80);   break; // Sola doğru ara
      case 3: xmotion.MotorControl(80, 40);   break; // Sağa doğru ara
      case 4: xmotion.MotorControl(-60, 60);  break; // Sola tankla ara (Hız düşürüldü, sensörün görmesi için)
      case 5: xmotion.MotorControl(60, -60);  break; // Sağa tankla ara (Hız düşürüldü, sensörün görmesi için)
      default: xmotion.MotorControl(60, 60);  break; // Temel ilerleme
    }
  }
}