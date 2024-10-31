#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#define LV_DISP_HOR_RES 240
#define LV_DISP_VER_RES 240

int LCD_BL_PWM = 5;
TFT_eSPI tft = TFT_eSPI();

static const uint32_t buf_size = LV_DISP_HOR_RES * 20;
static lv_color_t *dis_buf1;

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

void setup() {
  Serial.begin(115200);
  lv_init();
  initDisplay();
  lv_disp_init();

  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "LVGL 9.2.2 Simple");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void loop() { lv_timer_handler(); }
