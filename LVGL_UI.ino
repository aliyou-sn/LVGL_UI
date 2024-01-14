#include <lvgl.h>
#include <TFT_eSPI.h>

#define LVGL_REFRESH_TIME                   (5u)      // 5 milliseconds


static uint32_t lvgl_refresh_timestamp = 0u;

static void LVGL_TaskInit( void );
static void LVGL_TaskMng( void );

// Screen Resolution
static const uint32_t screenWidth  = 320;
static const uint32_t screenHeight = 480;
// LVGL related stuff
static lv_disp_draw_buf_t draw_buf;
// Declare a buffer for 1/10 screen size
static lv_color_t buf[ screenWidth * 10 ];
// TFT Instance
TFT_eSPI tft = TFT_eSPI();

static void Display_Flush(lv_disp_drv_t *disp, const lv_area_t *area, \
                          lv_color_t *color_p );
static void Touch_Read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data );

lv_obj_t *btn1;
lv_obj_t *btn2;
lv_obj_t *screenMain;
lv_obj_t *label;

void setup()
{
  delay(1000);
  Serial.begin(115200);
  Serial.println("LVGL Test");

  // Intialize the LVGL Library
  LVGL_TaskInit();
 
  // Initialize Display and Display Buffers
  Display_Init();

  screenMain = lv_obj_create(NULL);

  label = lv_label_create(screenMain);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(label, "Press a button");
  // lv_label_set_align(label, LV_FLEX_ALIGN_CENTER);
  lv_obj_align( label, LV_ALIGN_TOP_MID, 0u, LV_PCT(5) );
  lv_obj_set_size(label, 240, 40);
  lv_obj_set_pos(label, 0, 15);

  btn1 = lv_btn_create(screenMain);
  // lv_obj_set_event_cb(btn1, event_handler_btn);
  // lv_obj_add_event_cb(btn1, LV_EVENT_CLICKED, event_handler_btn);
  lv_obj_add_event_cb( btn1, event_handler_btn, LV_EVENT_CLICKED, NULL);


  lv_obj_set_width(btn1, 70);
  lv_obj_set_height(btn1, 32);
  lv_obj_set_pos(btn1, 60, 100);

  lv_obj_t * label1 = lv_label_create(btn1);
  lv_label_set_text(label1, "Hello");

  btn2 = lv_btn_create(screenMain);
  // lv_obj_set_event_cb(btn2, event_handler_btn, NULL, NULL);
  lv_obj_add_event_cb( btn2, event_handler_btn, LV_EVENT_CLICKED, NULL);

  lv_obj_set_width(btn2, 78);
  lv_obj_set_height(btn2, 32);
  lv_obj_set_pos(btn2, 190, 100);

  lv_obj_t * label2 = lv_label_create(btn2);
  lv_label_set_text(label2, "Goodbye");

  lv_scr_load(screenMain);

}

void loop()
{
  Display_Mng();
  LVGL_TaskMng();
}

static void LVGL_TaskInit( void )
{
  lv_init();
  lvgl_refresh_timestamp = millis();
}

static void LVGL_TaskMng( void )
{
  uint32_t now = millis();
  // LVGL Refresh Timed Task
  if( (now - lvgl_refresh_timestamp) >= LVGL_REFRESH_TIME )
  {
    lvgl_refresh_timestamp = now;
    // let the GUI does work
    lv_timer_handler();
  }
}


void Display_Init( void )
{
  #if LV_USE_LOG != 0
   // register print function for debugging
  lv_log_register_print_cb( LVGL_Print );
  #endif
  // Initialize TFT
  tft.begin();
  // Set Orientation to Landscape
  tft.setRotation(0);

  // Calibration Values for my Setup
  uint16_t calData[5] = { 399, 2919, 553, 2340, 3 };
  tft.setTouch(calData);

  // Initialize the display buffer
  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

  // Initialize the display
  static lv_disp_drv_t disp_drv;    /* Descriptor of a display driver */
  lv_disp_drv_init( &disp_drv );    /* Basic Inialization */
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;   /* Set the horizonral resolution of the display */
  disp_drv.ver_res = screenHeight;  /* Set the vertical resolution of the display */
  disp_drv.flush_cb = Display_Flush;/* driver function to flush the display */
  disp_drv.draw_buf = &draw_buf;    /* Assign the buffer to the display */
  lv_disp_drv_register( &disp_drv );/* Finally register the driver */

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = Touch_Read;
  lv_indev_drv_register( &indev_drv );
}


void Display_Mng( void )
{
  static uint32_t wait_time = 0u;
  // 
}

static void Touch_Read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch( &touchX, &touchY, 600 );

  if( !touched )
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;
    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;
    // uncomment to debug touch points
    // Serial.print( "Data x " );
    // Serial.println( touchX );
    // Serial.print( "Data y " );
    // Serial.println( touchY );
  }
}

static void Display_Flush(  lv_disp_drv_t *disp, const lv_area_t *area, \
                            lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );
  
  tft.startWrite();
  tft.setAddrWindow( area->x1, area->y1, w, h );
  tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.endWrite();
  
  lv_disp_flush_ready( disp );
}


// static void event_handler_btn(lv_obj_t * obj, lv_event_t event){
static void event_handler_btn(lv_obj_t * obj, lv_event_t *event){
    if(event == LV_EVENT_CLICKED) {
        if (obj == btn1)
        lv_label_set_text(label, "Hello");
        else if (obj == btn2){
          lv_label_set_text(label, "Goodbye");
        }
    }
}


#if LV_USE_LOG != 0
/* Serial debugging */
// void LVGL_Print(  lv_log_level_t level, const char * file, uint32_t line, \
//                   const char * fn_name, const char * dsc )
// {
//   Serial.printf( "%s(%s)@%d->%s\r\n", file, fn_name, line, dsc );
//   Serial.flush();
// }
void LVGL_Print( const char * buffer )
{
  Serial.printf( buffer );
  Serial.flush();
}
#endif




