#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#define LV_DISP_HOR_RES 240
#define LV_DISP_VER_RES 240

LV_FONT_DECLARE(led_48)

int LCD_BL_PWM = 5;
TFT_eSPI tft = TFT_eSPI();

static const uint32_t buf_size = LV_DISP_HOR_RES * 16;
static lv_color_t *dis_buf1;

uint64_t ms5 = 0, ms300 = 0;
lv_obj_t *label_date, *label_time, *label_ip;
lv_style_t style;
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
  tft.setTextColor(TFT_GREENYELLOW);

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
  sprintf(buf, "IP: %s", WiFi.localIP().toString().c_str());
  lv_label_set_text(label_ip, buf);
}

void inline startWifiConfig() {
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  for (int i = 0; i < 10; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.beginSmartConfig();
    tft.drawCentreString("Use ESPTouch", 120, 120, 4);
    while (!WiFi.smartConfigDone()) {
      delay(500);
    }
  }
  while (!WiFi.localIP().isSet()) {
    delay(200);
  }
}

void setup() {
  Serial.begin(115200);
  lv_init();
  initDisplay();
  lv_disp_init();
  lv_tick_set_cb([]() { return (uint32_t)millis(); });
  tft.drawCentreString("Config WiFi", 120, 120, 4);
  startWifiConfig();
  tft.fillScreen(TFT_BLACK);
  tft.drawCentreString("Config Time", 120, 120, 4);
  startConfigTime();

  label_date = lv_label_create(lv_scr_act());
  lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 8);
  lv_label_set_text(label_date, "2024-01-01");

  label_time = lv_label_create(lv_scr_act());
  lv_style_init(&style);
  lv_style_set_text_color(&style, lv_color_hex(0x00FA9A));
  lv_obj_add_style(label_time, &style, 0);

  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(label_time, &led_48, LV_PART_MAIN);
  lv_label_set_text(label_time, "00:00:00");

  label_ip = lv_label_create(lv_scr_act());
  lv_obj_align(label_ip, LV_ALIGN_BOTTOM_MID, 0, -8);
  lv_label_set_text(label_ip, "IP: 127.0.0.1");

  showClientIP();
  on_300ms_tick();
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
