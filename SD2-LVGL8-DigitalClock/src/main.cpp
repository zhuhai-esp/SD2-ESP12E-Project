#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#define LV_DISP_HOR_RES 240
#define LV_DISP_VER_RES 240

LV_FONT_DECLARE(led_64)

int LCD_BL_PWM = 5;
TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static const uint32_t buf_size = LV_DISP_HOR_RES * 20;
static lv_color_t dis_buf1[buf_size] = {0};

uint64_t ms5 = 0, ms300 = 0;
lv_obj_t *label_date, *label_time, *label_ip;
lv_style_t style;
char buf[256] = {0};

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();
  lv_disp_flush_ready(disp);
}

void inline lv_disp_init() {
  lv_disp_draw_buf_init(&draw_buf, dis_buf1, nullptr, buf_size);
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LV_DISP_HOR_RES;
  disp_drv.ver_res = LV_DISP_VER_RES;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
}

void inline initDisplay() {
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  pinMode(TFT_BL, OUTPUT);
  analogWriteResolution(10);
  analogWriteFreq(25000);
  analogWrite(TFT_BL, 1023 - (LCD_BL_PWM * 10));
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
    tft.fillScreen(TFT_BLACK);
    tft.drawCentreString("Use ESPTouch", 120, 120, 4);
    while (!WiFi.smartConfigDone()) {
      delay(500);
    }
  }
  while (!WiFi.localIP().isSet()) {
    delay(200);
  }
}

void inline startConfigTime() {
  const int timeZone = 8 * 3600;
  configTime(timeZone, 0, "ntp6.aliyun.com", "cn.ntp.org.cn", "ntp.ntsc.ac.cn");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  lv_init();
  initDisplay();
  lv_disp_init();

  tft.drawCentreString("Config WiFi", 120, 120, 4);
  startWifiConfig();
  tft.fillScreen(TFT_BLACK);
  tft.drawCentreString("Config Time", 120, 120, 4);
  startConfigTime();

  label_date = lv_label_create(lv_scr_act());
  lv_obj_align(label_date, LV_ALIGN_TOP_MID, 0, 16);
  lv_label_set_text(label_date, "2024-01-01");

  label_time = lv_label_create(lv_scr_act());
  lv_style_init(&style);
  lv_style_set_text_color(&style, lv_color_hex(0xfffacd));
  lv_obj_add_style(label_time, &style, 0);

  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(label_time, &led_64, LV_PART_MAIN);
  lv_label_set_text(label_time, "00:00:00");

  label_ip = lv_label_create(lv_scr_act());
  lv_obj_align(label_ip, LV_ALIGN_BOTTOM_MID, 0, -16);
  lv_label_set_text(label_ip, "IP: 127.0.0.1");

  lv_obj_t *label_ver = lv_label_create(lv_scr_act());
  lv_obj_align(label_ver, LV_ALIGN_BOTTOM_MID, 0, -48);
  snprintf(buf, 64, "LVGL V%d.%d.%d", lv_version_major(), lv_version_minor(),
           lv_version_patch());
  lv_label_set_text(label_ver, buf);

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
