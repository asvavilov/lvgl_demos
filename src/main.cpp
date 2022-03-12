#include <Arduino.h>

#include <TFT_eSPI.h>

#include <lvgl.h>
#include <indev/XPT2046.h>

#include "lv_demo.h"

// пример инициализации для Arduino: lvgl/examples/arduino/LVGL_Arduino

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */

#define DISP_HOR_RES TFT_HEIGHT
#define DISP_VER_RES TFT_WIDTH
#define MY_DISP_HOR_RES TFT_HEIGHT
#define MY_DISP_VER_RES TFT_WIDTH
static lv_disp_draw_buf_t draw_buf;
//static lv_color_t buf1[ DISP_HOR_RES * 10 ];
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES / 10];                        /*Declare a buffer for 1/10 screen size*/

#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{
	Serial.printf("%s@%d->%s\r\n", file, line, dsc);
	Serial.flush();
}
#endif

static void btn_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		//LV_LOG_USER("Clicked");
		static uint8_t cnt = 0;
		cnt++;

		/*Get the first child of the button which is the label and change its text*/
		lv_obj_t * btn = lv_event_get_target(e);
		lv_obj_t * label = lv_obj_get_child(btn, 0);
		lv_label_set_text_fmt(label, "Button: %d", cnt);
	}
}
void my_demo()
{

	/* Create simple label */
	lv_obj_t *label = lv_label_create(lv_scr_act());
	lv_label_set_text_fmt(label, "Hello Arduino! V%d.%d.%d%s", lv_version_major(), lv_version_minor(), lv_version_patch(), lv_version_info());
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);


	lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
	lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
	lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
	lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t * label2 = lv_label_create(btn);          /*Add a label to the button*/
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
void my_touchpad_read(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
	uint16_t touchX, touchY;

	bool touched = tft.getTouch(&touchX, &touchY, 600);

	if(!touched) {
	  data->state = LV_INDEV_STATE_RELEASED;
	} else {
	  data->state = LV_INDEV_STATE_PRESSED;

	  /*Set the coordinates*/
	  data->point.x = touchX;
	  data->point.y = touchY;

	  Serial.print("Data x");
	  Serial.println(touchX);

	  Serial.print("Data y");
	  Serial.println(touchY);
	}
}

void setup()
{
	Serial.begin(115200); /* prepare for possible serial debug */

	Serial.println("serial...");

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

	// Create a draw buffer: LVGL will render the graphics here first, and send the rendered image to the display. The buffer size can be set freely but 1/10 screen size is a good starting point.
	lv_disp_draw_buf_init(&draw_buf, buf1, NULL, MY_DISP_HOR_RES * MY_DISP_VER_RES / 10);  /*Initialize the display buffer.*/

	// Implement and register a function which can copy the rendered image to an area of your display:
	static lv_disp_drv_t disp_drv;        /*Descriptor of a display driver*/
	lv_disp_drv_init(&disp_drv);          /*Basic initialization*/
	disp_drv.flush_cb = my_disp_flush;    /*Set your driver function*/
	disp_drv.draw_buf = &draw_buf;        /*Assign the buffer to the display*/
	disp_drv.hor_res = MY_DISP_HOR_RES;   /*Set the horizontal resolution of the display*/
	disp_drv.ver_res = MY_DISP_VER_RES;   /*Set the vertical resolution of the display*/
	lv_disp_drv_register(&disp_drv);      /*Finally register the driver*/

	// Implement and register a function which can read an input device. E.g. for a touchpad:
	static lv_indev_drv_t indev_drv;           /*Descriptor of a input device driver*/
	lv_indev_drv_init(&indev_drv);             /*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
	indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
	lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/


	// FIXME исправить стандарные примеры (см. папку demos в репозитории lvgl)
	//lv_demo_widgets();
	// or
	my_demo();


}

void loop()
{
	lv_task_handler(); /* let the GUI do its work */
	delay(5);
}
