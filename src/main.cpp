#include <Arduino.h>


// TODO попробовать кириллические шрифты


/*
инструкция по LVGL: https://docs.lvgl.io/latest/en/html/get-started/arduino.html#

esp32 doit devkit 1.0:
ili9341: подключение VSPI и задать прочие пины TFT_CS, TFT_DC, TFT_RST, см. пример Setup1_ILI9341.h
ili9844: подключение VSPI и задать прочие пины по примеру Setup21_ILI9488.h
*/

#include <TFT_eSPI.h>

#include <lvgl.h>
#include <indev/XPT2046.h>

#include <lv_examples.h>

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{
	Serial.printf("%s@%d->%s\r\n", file, line, dsc);
	Serial.flush();
}
#endif

static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
	if(event == LV_EVENT_CLICKED) {
		static uint8_t cnt = 0;
		cnt++;

		/*Get the first child of the button which is the label and change its text*/
		lv_obj_t * label = lv_obj_get_child(btn, NULL);
		lv_label_set_text_fmt(label, "Button: %d", cnt);
	}
}
void my_demo()
{

	/* Create simple label */
	lv_obj_t *label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_text(label, "Hello Arduino! (V7.0.X)");
	lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);


	lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
	lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
	lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
	lv_obj_set_event_cb(btn, btn_event_cb);                 /*Assign a callback to the button*/

	lv_obj_t * label2 = lv_label_create(btn, NULL);          /*Add a label to the button*/
	lv_label_set_text(label2, "Button");                     /*Set the labels text*/

}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);

	tft.startWrite();
	tft.setAddrWindow(area->x1, area->y1, w, h);
	tft.pushColors(&color_p->full, w * h, true);
	tft.endWrite();

	lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
	uint16_t touchX, touchY;

	bool touched = tft.getTouch(&touchX, &touchY, 600);

	if(!touched) {
	  data->state = LV_INDEV_STATE_REL;
	} else {
	  data->state = LV_INDEV_STATE_PR;

	  /*Set the coordinates*/
	  data->point.x = touchX;
	  data->point.y = touchY;

	  Serial.print("Data x");
	  Serial.println(touchX);

	  Serial.print("Data y");
	  Serial.println(touchY);
	}

	return false; /*Return `false` because we are not buffering and no more data to read*/
}

void setup()
{
	Serial.begin(115200); /* prepare for possible serial debug */

	lv_init();

#if USE_LV_LOG != 0
	lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

	tft.init();

  tft.setRotation(1);
#ifdef ILI9341_DRIVER
  uint16_t calData[5] = { 216, 3490, 379, 3290, 1 };
#endif
#ifdef ILI9488_DRIVER
  uint16_t calData[5] = { 159, 3738, 272, 3562, 7 };
#endif
/*
tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
for (uint8_t i = 0; i < 5; i++)
{
	Serial.println(calData[i]);
}
*/
	tft.setTouch(calData);

	lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

	/*Initialize the display*/
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = TFT_HEIGHT;
	disp_drv.ver_res = TFT_WIDTH;
	disp_drv.flush_cb = my_disp_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	/*Initialize the (dummy) input device driver*/
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_touchpad_read;
	lv_indev_drv_register(&indev_drv);


	// Try an example from the lv_examples repository
	// https://github.com/lvgl/lv_examples*/
	lv_demo_widgets();
	// or
	//my_demo();


}

void loop()
{
	lv_task_handler(); /* let the GUI do its work */
	delay(5);
}
