#include <pebble.h>
  
#include "fuzzy_time.h"

#define ANIMATION_DURATION 800
#define LINE_BUFFER_SIZE 50
#define WINDOW_NAME "fuzzy_i18n_modern"

static Window *s_main_window;

static GFont s_time_font;
static GFont s_time_font_big;
static GBitmap *s_bitmap_bt_on;
static GBitmap *s_bitmap_bt_off;
static GBitmap *s_bitmap_charging;

typedef struct {
  TextLayer *layer[2];
  GRect out_rect;
  PropertyAnimation *animate_out;
  PropertyAnimation *animate_in;
} TextLine;

typedef struct {
  char line1[LINE_BUFFER_SIZE];
  char line2[LINE_BUFFER_SIZE];
  char line3[LINE_BUFFER_SIZE];
  char topline[LINE_BUFFER_SIZE];
  char bottomline[LINE_BUFFER_SIZE];
} TheTime;

static TextLine line1;
static TextLine line2;
static TextLine line3;
//static TextLine topbar;
//static TextLine bottombar;
static TextLayer *batterylayer;
static TextLayer *toplayer;
static TextLayer *bottomlayer;
static BitmapLayer *s_bt_bitmap_layer;
static BitmapLayer *s_ch_bitmap_layer;

static struct tm *t;
static TheTime cur_time;
static TheTime new_time;

const int line1_y = 17;
const int line2_y = 57;
const int line3_y = 93;
const int line_h = 55;


static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "anim_stopped_handler %p", animation);  
#ifndef PBL_COLOR
  // Free the animation
  animation_destroy(animation);
#endif

  // Schedule the next one, unless the app is exiting
  if (finished) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "anim_stopped_handler finished");  
  }
  else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "anim_stopped_handler not finished");  
  }
}

void updateLayer(TextLine *animating_line, char* old_line, char* new_line, bool isbold) {
  // animate out current layer
  GRect from_frame_out = layer_get_frame(text_layer_get_layer(animating_line->layer[0]));
  GRect to_frame_out = animating_line->out_rect;
  from_frame_out.origin.x = 0;
  if(to_frame_out.origin.y == line2_y) 
      to_frame_out.origin.x = 144;
  else 
    to_frame_out.origin.x = -144;

  // Create the animation
  animating_line->animate_out = property_animation_create_layer_frame(text_layer_get_layer(animating_line->layer[0]), &from_frame_out, &to_frame_out);
  
  // animate in current layer
  GRect from_frame_in = layer_get_frame(text_layer_get_layer(animating_line->layer[1]));
  GRect to_frame_in = layer_get_frame(text_layer_get_layer(animating_line->layer[0]));
  if (from_frame_in.origin.y == line2_y) 
    from_frame_in.origin.x = -144;
  else 
    from_frame_in.origin.x = 144;
  to_frame_in.origin.x = 0;

  // Create the animation
  animating_line->animate_in = property_animation_create_layer_frame(text_layer_get_layer(animating_line->layer[1]), &from_frame_in, &to_frame_in);
  
  int line_no = 0;
  if(animating_line->out_rect.origin.y==line1_y) line_no = 1;
  else if(animating_line->out_rect.origin.y==line2_y) line_no = 2;
  else if(animating_line->out_rect.origin.y==line3_y) line_no = 3;
  GSize size= graphics_text_layout_get_content_size(new_line,
                                        isbold?s_time_font_big:s_time_font,
                                        GRect(0, 0, 200, animating_line->out_rect.size.h),
                                        GTextOverflowModeTrailingEllipsis,
                                        GTextAlignmentLeft);
  APP_LOG(APP_LOG_LEVEL_INFO , "line %d: old text '%s', new text '%s' size %d", line_no, old_line, new_line, size.w);
/*
  size= graphics_text_layout_get_content_size("quatre h.",
                                        s_time_font_big,
                                        GRect(0, 0, 200, animating_line->out_rect.size.h),
                                        GTextOverflowModeTrailingEllipsis,
                                        GTextAlignmentLeft);
  APP_LOG(APP_LOG_LEVEL_INFO , "line 1: 'quatre h.' size %d", size.w);
*/
  text_layer_set_text(animating_line->layer[0], old_line);
  text_layer_set_text(animating_line->layer[1], new_line);

  Animation *animout = property_animation_get_animation(animating_line->animate_out);
  Animation *animin = property_animation_get_animation(animating_line->animate_in);
  // animation duration
  animation_set_duration(animout, ANIMATION_DURATION);
  animation_set_duration(animin, ANIMATION_DURATION);
  // Schedule to occur ASAP with default settings
#ifdef PBL_COLOR
  // Create the sequence
//  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE , "before animation_sequence_create line %d", line_no);
  Animation *sequence = animation_sequence_create(animout, animin, NULL);
/*
  animation_set_handlers(sequence, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
*/
//  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE , "before animation_schedule line %d", line_no);
  // Play the sequence
  animation_schedule(sequence);
  APP_LOG(APP_LOG_LEVEL_DEBUG , "after animation_schedule %p", sequence);
#else
  animation_set_curve(animout, AnimationCurveEaseIn);
  animation_set_curve(animin, AnimationCurveEaseOut);
  animation_set_handlers(animout, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  animation_set_handlers(animin, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  animation_schedule(animout);
  animation_schedule(animin);
#endif
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE , "end updateLayer line %d", line_no);
}

void update_watch(struct tm* t) {
  // Let's get the new text date
  info_lines(t, new_time.topline, new_time.bottomline);

  // Let's update the top bar
  text_layer_set_text(toplayer, new_time.topline);

  // Let's update the bottom bar
  text_layer_set_text(bottomlayer, new_time.bottomline);

  // Let's get the new text time
  int hourline = fuzzy_time(t, new_time.line1, new_time.line2, new_time.line3);

  text_layer_set_font(line1.layer[1], (hourline==1)?s_time_font_big:s_time_font);
  text_layer_set_font(line2.layer[1], (hourline==2)?s_time_font_big:s_time_font);
  text_layer_set_font(line3.layer[1], (hourline==3)?s_time_font_big:s_time_font);

  // update hour only if changed
  if(strcmp(new_time.line1, cur_time.line1) != 0) updateLayer(&line1, cur_time.line1, new_time.line1, hourline==1);
  // update min1 only if changed
  if(strcmp(new_time.line2, cur_time.line2) != 0) updateLayer(&line2, cur_time.line2, new_time.line2, hourline==2);
  // update min2 only if changed happens on
  if(strcmp(new_time.line3, cur_time.line3) != 0) updateLayer(&line3, cur_time.line3, new_time.line3, hourline==3);

  // set cur_time
  cur_time = new_time;
}

static void battery_handler(BatteryChargeState charge_state) {
  GBitmap *bitmap_charge;

  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE , "battery charge: %d%%", charge_state.charge_percent);

  if (charge_state.is_charging) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING);
  } 
  else if(charge_state.charge_percent >= 95) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE100);
  }
  else if(charge_state.charge_percent >= 85) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE90);
  }
  else if(charge_state.charge_percent >= 75) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE80);
  }
  else if(charge_state.charge_percent >= 65) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE70);
  }
  else if(charge_state.charge_percent >= 55) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE60);
  }
  else if(charge_state.charge_percent >= 45) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE50);
  }
  else if(charge_state.charge_percent >= 35) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE40);
  }
  else if(charge_state.charge_percent >= 25) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE30);
  }
  else if(charge_state.charge_percent >= 15) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE20);
  }
  else {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE10);
  }
  bitmap_layer_set_bitmap(s_ch_bitmap_layer, bitmap_charge);

  gbitmap_destroy(s_bitmap_charging);
  s_bitmap_charging = bitmap_charge;
}

static void bt_handler(bool connected) {

  if (connected) {
    bitmap_layer_set_bitmap(s_bt_bitmap_layer, s_bitmap_bt_on);
  } else {
    bitmap_layer_set_bitmap(s_bt_bitmap_layer, s_bitmap_bt_off);
    vibes_short_pulse();
  }
}

static void main_window_load(Window *window) {
  // UUID : 48eb8a14-ba93-45c1-87f6-ac117b4a23df
  
  // locale
#if DEBUG
  setlocale(LC_ALL, "de_DE");
#else
  setlocale(LC_ALL, i18n_get_system_locale());
#endif
  
  // background color
#ifdef PBL_COLOR
  window_set_background_color(window, GColorBlue);
#else
  window_set_background_color(window, GColorBlack);
#endif
  
  // Load GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOMESTIC_NORMAL_SUBSET_35));
  s_time_font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOMESTIC_BOLD_SUBSET_40));
  
  // Init the text layers used to show the time

  // line1
  line1.layer[0] = text_layer_create(GRect(0, line1_y, 144, line_h));
  text_layer_set_background_color(line1.layer[0], GColorClear);
#ifdef PBL_COLOR
  text_layer_set_text_color(line1.layer[0], GColorPastelYellow);
#else
  text_layer_set_text_color(line1.layer[0], GColorWhite);
#endif
//  text_layer_set_font(line1.layer[0], fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_font(line1.layer[0], s_time_font);
  text_layer_set_text_alignment(line1.layer[0], GTextAlignmentLeft);
  text_layer_set_overflow_mode (line1.layer[0], GTextOverflowModeWordWrap);

  line1.layer[1] = text_layer_create(GRect(144, line1_y, 144, line_h));
  text_layer_set_background_color(line1.layer[1], GColorClear);
#ifdef PBL_COLOR
  text_layer_set_text_color(line1.layer[1], GColorYellow);
#else
  text_layer_set_text_color(line1.layer[1], GColorWhite);
#endif
  text_layer_set_font(line1.layer[1], s_time_font);
  text_layer_set_text_alignment(line1.layer[1], GTextAlignmentLeft);
  text_layer_set_overflow_mode (line1.layer[1], GTextOverflowModeWordWrap);

  line1.out_rect = GRect(-144, line1_y, 144, line_h);
  
  // line2
  line2.layer[0] = text_layer_create(GRect(0, line2_y, 144, line_h));
  text_layer_set_background_color(line2.layer[0], GColorClear);
#ifdef PBL_COLOR
  text_layer_set_text_color(line2.layer[0], GColorPastelYellow);
#else
  text_layer_set_text_color(line2.layer[0], GColorWhite);
#endif
  text_layer_set_font(line2.layer[0], s_time_font);
  text_layer_set_text_alignment(line2.layer[0], GTextAlignmentLeft);
  text_layer_set_overflow_mode (line2.layer[0], GTextOverflowModeWordWrap);

  line2.layer[1] = text_layer_create(GRect(-144, line2_y, 144, line_h));
  text_layer_set_background_color(line2.layer[1], GColorClear);
#ifdef PBL_COLOR
  text_layer_set_text_color(line2.layer[1], GColorYellow);
#else
  text_layer_set_text_color(line2.layer[1], GColorWhite);
#endif
  text_layer_set_font(line2.layer[1], s_time_font);
  text_layer_set_text_alignment(line2.layer[1], GTextAlignmentLeft);
  text_layer_set_overflow_mode (line2.layer[1], GTextOverflowModeWordWrap);

  line2.out_rect = GRect(144, line2_y, 144, line_h);

  // line3
  line3.layer[0] = text_layer_create(GRect(0, line3_y, 180, line_h));
  text_layer_set_background_color(line3.layer[0], GColorClear);
#ifdef PBL_COLOR
  text_layer_set_text_color(line3.layer[0], GColorPastelYellow);
#else
  text_layer_set_text_color(line3.layer[0], GColorWhite);
#endif
  text_layer_set_font(line3.layer[0], s_time_font);
  text_layer_set_text_alignment(line3.layer[0], GTextAlignmentLeft);
//  text_layer_set_overflow_mode (line3.layer[0], GTextOverflowModeWordWrap);

  line3.layer[1] = text_layer_create(GRect(144, line3_y, 180, line_h));
  text_layer_set_background_color(line3.layer[1], GColorClear);
#ifdef PBL_COLOR
  text_layer_set_text_color(line3.layer[1], GColorYellow);
#else
  text_layer_set_text_color(line3.layer[1], GColorWhite);
#endif
  text_layer_set_font(line3.layer[1], s_time_font);
  text_layer_set_text_alignment(line3.layer[1], GTextAlignmentLeft);
//  text_layer_set_overflow_mode (line3.layer[1], GTextOverflowModeWordWrap);

  line3.out_rect = GRect(-144, line3_y, 144, line_h);

  // battery text
  batterylayer = text_layer_create(GRect(144-33, -3, 33, 18));
  text_layer_set_background_color(batterylayer, GColorClear);
  text_layer_set_font(batterylayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(batterylayer, GTextAlignmentRight);
#ifdef PBL_COLOR
  text_layer_set_text_color(batterylayer, GColorYellow);
#else
  text_layer_set_text_color(batterylayer, GColorWhite);
#endif

  // Create charging GBitmap, then set to created BitmapLayer
  s_ch_bitmap_layer = bitmap_layer_create(GRect(144-21, 0, 20, 17));
  bitmap_layer_set_background_color(s_ch_bitmap_layer, GColorClear); 
#ifdef PBL_COLOR
  bitmap_layer_set_compositing_mode(s_ch_bitmap_layer, GCompOpSet);
#else
  bitmap_layer_set_compositing_mode(s_ch_bitmap_layer, GCompOpAssignInverted);
#endif
//  bitmap_layer_set_alignment(s_ch_bitmap_layer, GAlignLeft);
//  layer_set_hidden ((Layer *)s_ch_bitmap_layer, true);
  s_bitmap_charging = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE10);
//  bitmap_layer_set_bitmap(s_ch_bitmap_layer, s_bitmap_charging);
  
  // top text
  toplayer = text_layer_create(GRect(40, 0, 144-40-40, 18));
#ifdef PBL_COLOR
  text_layer_set_background_color(toplayer, GColorBlueMoon);
  text_layer_set_text_color(toplayer, GColorIcterine);
#else
  text_layer_set_background_color(toplayer, GColorClear);
  text_layer_set_text_color(toplayer, GColorWhite);
#endif
  text_layer_set_font(toplayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(toplayer, GTextAlignmentCenter);
  
  // bottom text
  bottomlayer = text_layer_create(GRect(0, 150, 144, 18));
#ifdef PBL_COLOR
  text_layer_set_background_color(bottomlayer, GColorBlueMoon);
  text_layer_set_text_color(bottomlayer, GColorIcterine);
#else
  text_layer_set_background_color(bottomlayer, GColorClear);
  text_layer_set_text_color(bottomlayer, GColorWhite);
#endif
  text_layer_set_font(bottomlayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(bottomlayer, GTextAlignmentCenter);

  // Create GBitmap, then set to created BitmapLayer
  s_bitmap_bt_on = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_ON);
  s_bitmap_bt_off = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_OFF);
  s_bt_bitmap_layer = bitmap_layer_create(GRect(0, 0, 15, 22));
  bitmap_layer_set_background_color(s_bt_bitmap_layer, GColorClear); 
#ifdef PBL_COLOR
  bitmap_layer_set_compositing_mode(s_bt_bitmap_layer, GCompOpSet);
#else
  bitmap_layer_set_compositing_mode(s_bt_bitmap_layer, GCompOpAssignInverted);
#endif
//  bitmap_layer_set_alignment(s_bt_bitmap_layer, GAlignRight);
    
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  t = localtime(&now);
  update_watch(t);
  
  battery_handler(battery_state_service_peek());
  
  bt_handler(bluetooth_connection_service_peek());

  Layer *root_layer = window_get_root_layer(window);
	
  layer_add_child(root_layer, text_layer_get_layer(line3.layer[0]));
  layer_add_child(root_layer, text_layer_get_layer(line3.layer[1]));
  layer_add_child(root_layer, text_layer_get_layer(line2.layer[0]));
  layer_add_child(root_layer, text_layer_get_layer(line2.layer[1]));
  layer_add_child(root_layer, text_layer_get_layer(line1.layer[0]));
  layer_add_child(root_layer, text_layer_get_layer(line1.layer[1]));
  layer_add_child(root_layer, text_layer_get_layer(batterylayer));
  layer_add_child(root_layer, text_layer_get_layer(toplayer));
  layer_add_child(root_layer, text_layer_get_layer(bottomlayer));
  layer_add_child(root_layer, bitmap_layer_get_layer(s_bt_bitmap_layer));
  layer_add_child(root_layer, bitmap_layer_get_layer(s_ch_bitmap_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(line1.layer[0]);
  text_layer_destroy(line1.layer[1]);
  text_layer_destroy(line2.layer[0]);
  text_layer_destroy(line2.layer[1]);
  text_layer_destroy(line3.layer[0]);
  text_layer_destroy(line3.layer[1]);
  text_layer_destroy(batterylayer);
  text_layer_destroy(toplayer);
  text_layer_destroy(bottomlayer);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_bt_bitmap_layer);
  bitmap_layer_destroy(s_ch_bitmap_layer);
  
  // Destroy GBitmap
  gbitmap_destroy(s_bitmap_bt_on);
  gbitmap_destroy(s_bitmap_bt_off);
  gbitmap_destroy(s_bitmap_charging);

  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_time_font_big);
  
  // Stop any animation in progress
  animation_unschedule_all();  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_watch(tick_time);
}
  
/**
 * Debug methods. For quickly debugging enable debug macro on top to transform the watchface into
 * a standard app and you will be able to change the time with the up and down buttons
 */
#if DEBUG

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	(void)recognizer;
	(void)context;
	
	t->tm_min -= 1;
	if (t->tm_min < 0) {
		t->tm_min += 60;
		t->tm_hour -= 1;
		
		if (t->tm_hour < 0) {
			t->tm_hour = 23;
		}
	}
	update_watch(t);
}


static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
	(void)context;
	
	t->tm_min += 5;
	if (t->tm_min >= 60) {
		t->tm_min -= 60;
		t->tm_hour += 1;
		
		if (t->tm_hour >= 24) {
			t->tm_hour = 0;
		}
	}
	update_watch(t);
}

static void click_config_provider(Window *window) {
  (void)window;

  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}
#endif

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  window_set_fullscreen(s_main_window, true);
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
#if DEBUG==0
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
#endif
  // Register with BatteryService
  battery_state_service_subscribe(battery_handler);
  // Register BluetoothService
  bluetooth_connection_service_subscribe(bt_handler);

#if DEBUG
	// Button functionality
	window_set_click_config_provider(s_main_window, (ClickConfigProvider) click_config_provider);
#endif
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
