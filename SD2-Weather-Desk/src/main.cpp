#include "sd2.hpp"

void setup() {
  Serial.begin(115200);
  setupTFT();
  startWifiConfig();
  startConfigTime();
  setupOTAConfig();
  setupTasks();
  setupInitApi();
  controller.run();
}

void loop() {
  if (controller.shouldRun()) {
    controller.run();
  }
}
