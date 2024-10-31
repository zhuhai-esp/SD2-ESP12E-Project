#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

#define LV_DISP_HOR_RES 240
#define LV_DISP_VER_RES 240

int LCD_BL_PWM = 5;
TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static const uint32_t buf_size = LV_DISP_HOR_RES * 20;
static lv_color_t dis_buf1[buf_size] = {0};

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

void setup() {
  Serial.begin(115200);
  lv_init();
  initDisplay();
  lv_disp_init();
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "LVGL 8.4.0 Simple");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void loop() { lv_timer_handler(); }
