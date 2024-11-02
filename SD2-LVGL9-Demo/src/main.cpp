#include <Arduino.h>
#include <FS.h>
#include <TFT_eSPI.h>
#include <WiFiManager.h>
#include <lvgl.h>

#define LV_DISP_HOR_RES 240
#define LV_DISP_VER_RES 240

LV_FONT_DECLARE(led_48)

int LCD_BL_PWM = 5;
TFT_eSPI tft = TFT_eSPI();

static const uint32_t buf_size = LV_DISP_HOR_RES * 20;
static lv_color_t *dis_buf1;

uint64_t ms5 = 0, ms300 = 0;
lv_obj_t *label_date, *label_time, *label_ip;
char buf[256] = {0};

void inline lv_disp_init() {
  auto *disp = lv_display_create(LV_DISP_HOR_RES, LV_DISP_VER_RES);
  auto f_disp = [](lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(disp);
  };
  lv_display_set_flush_cb(disp, f_disp);
  auto mode = LV_DISPLAY_RENDER_MODE_PARTIAL;
  dis_buf1 = (lv_color_t *)malloc(buf_size);
  lv_display_set_buffers(disp, dis_buf1, nullptr, buf_size, mode);
}

void inline initDisplay() {
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (LCD_BL_PWM * 10));
}

void inline startConfigTime() {
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
  }
}

void inline on_300ms_tick() {
  struct tm info;
  getLocalTime(&info);
  strftime(buf, 32, "%F", &info);
  lv_label_set_text(label_date, buf);
  strftime(buf, 32, "%T", &info);
  lv_label_set_text(label_time, buf);
}

inline void showClientIP() {
  lv_label_set_text(label_ip, WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  lv_init();
  initDisplay();
  lv_disp_init();
  lv_tick_set_cb([]() { return (uint32_t)millis(); });
  WiFiManager wm;
  tft.drawCentreString("Config WiFi", 120, 120, 4);
  wm.autoConnect();
  tft.drawCentreString("Config Time", 120, 120, 4);
  startConfigTime();
  label_date = lv_label_create(lv_scr_act());
  lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 8);
  lv_label_set_text(label_date, "2024-01-01");

  label_time = lv_label_create(lv_scr_act());
  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(label_time, &led_48, LV_PART_MAIN);
  lv_label_set_text(label_time, "00:00:00");

  label_ip = lv_label_create(lv_scr_act());
  lv_obj_align(label_ip, LV_ALIGN_BOTTOM_MID, 0, -8);

  showClientIP();
}

void loop() {
  lv_timer_handler();
  auto cur = millis();
  if (cur - ms5 >= 5) {
    ms5 = cur;
    lv_timer_handler();
  }
  if (cur - ms300 >= 5) {
    ms300 = cur;
    on_300ms_tick();
  }
}
