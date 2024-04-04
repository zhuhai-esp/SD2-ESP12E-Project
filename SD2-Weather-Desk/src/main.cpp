#include "sd2.hpp"

void setup() {
  Serial.begin(115200);
  setupTFT();
  startWifiConfig();
  startConfigTime();
  setupOTAConfig();
  setupJPEG();
  setupTasks();
}

void loop() { handleLoop(); }
