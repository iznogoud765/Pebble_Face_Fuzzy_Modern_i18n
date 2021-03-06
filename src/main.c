// UUID : 48eb8a14-ba93-45c1-87f6-ac117b4a23df
#include <pebble.h>
  
#include "fuzzy_time.h"

#define ANIMATION_DURATION 800
#define LINE_BUFFER_SIZE 50
#define WINDOW_NAME "fuzzy_i18n_modern"

static AppSync s_sync;
static uint8_t s_sync_buffer[32];

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
  char line[3][LINE_BUFFER_SIZE];
  char topline[LINE_BUFFER_SIZE];
  char bottomline[LINE_BUFFER_SIZE];
} TheTime;

static TextLine line[3];
static TextLayer *batterylayer;
static TextLayer *toplayer;
static TextLayer *bottomlayer;
static BitmapLayer *s_bt_bitmap_layer;
static BitmapLayer *s_ch_bitmap_layer;

static struct tm *t;
static TheTime cur_time;
static TheTime new_time;

const int line_y[] = {17, 57, 93};
//const int line1_y = 17;
//const int line2_y = 57;
//const int line3_y = 93;
const int line_h = 55;

enum {
  KEY_BCKGD_COLOR = 0,
  KEY_ALIGN = 1
};

enum {
  TEXT_ALIGN_LEFT = 0,
  TEXT_ALIGN_CENTER,
  TEXT_ALIGN_RIGHT
};

static int text_align = PBL_IF_RECT_ELSE(TEXT_ALIGN_LEFT,TEXT_ALIGN_CENTER);
static bool isBkgdDark = true;


static GTextAlignment lookup_text_alignment(int align_key)
{
	GTextAlignment alignment;
	switch (align_key)
	{
		default:
			alignment = GTextAlignmentLeft;
			break;
		case TEXT_ALIGN_CENTER:
			alignment = GTextAlignmentCenter;
			break;
		case TEXT_ALIGN_RIGHT:
			alignment = GTextAlignmentRight;
			break;
	}
	return alignment;
}

#ifdef PBL_PLATFORM_APLITE
static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "anim_stopped_handler %p", animation);  
#ifndef PBL_COLOR
  // Free the animation
  animation_destroy(animation);
#endif

  // Schedule the next one, unless the app is exiting
  if (finished) {
    APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "anim_stopped_handler finished");  
  }
  else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "anim_stopped_handler not finished");  
  }
}
#endif

void updateLayer(TextLine *animating_line, char* old_line, char* new_line, bool isbold) {
  Layer *root_layer = window_get_root_layer(s_main_window);
	GRect bounds = layer_get_frame(root_layer);

  // animate out current layer
  GRect from_frame_out = layer_get_frame(text_layer_get_layer(animating_line->layer[0]));
  GRect to_frame_out = animating_line->out_rect;
  from_frame_out.origin.x = 2;
  
  if(animating_line->out_rect.origin.x == bounds.size.w) {
    to_frame_out.origin.x = bounds.size.w;
  }
  else {
    to_frame_out.origin.x = -bounds.size.w;
    to_frame_out.origin.y = line_y[1];
  }
  
  // Create the animation
  animating_line->animate_out = property_animation_create_layer_frame(text_layer_get_layer(animating_line->layer[0]), &from_frame_out, &to_frame_out);
  
  // animate in current layer
  GRect from_frame_in = layer_get_frame(text_layer_get_layer(animating_line->layer[1]));
  GRect to_frame_in = layer_get_frame(text_layer_get_layer(animating_line->layer[0]));
  to_frame_in.origin.y = animating_line->out_rect.origin.y;

  if(animating_line->out_rect.origin.x == bounds.size.w) {
    from_frame_in.origin.x = -bounds.size.w;
  }
  else {
    from_frame_in.origin.x = bounds.size.w;
    if(animating_line->out_rect.origin.y==line_y[0])
      from_frame_in.origin.y = line_y[0]-animating_line->out_rect.size.h;
    else
      from_frame_in.origin.y = line_y[2]+animating_line->out_rect.size.h;
  }
  to_frame_in.origin.x = 2;

  // Create the animation
  animating_line->animate_in = property_animation_create_layer_frame(text_layer_get_layer(animating_line->layer[1]), &from_frame_in, &to_frame_in);
  
  int line_no = 0;
  if(animating_line->out_rect.origin.y==line_y[0]) line_no = 1;
  else if(animating_line->out_rect.origin.y==line_y[1]) line_no = 2;
  else if(animating_line->out_rect.origin.y==line_y[2]) line_no = 3;
  GSize size= graphics_text_layout_get_content_size(new_line,
                                        isbold?s_time_font_big:s_time_font,
                                        GRect(0, 0, 200, animating_line->out_rect.size.h),
                                        GTextOverflowModeTrailingEllipsis,
                                        GTextAlignmentLeft);
  APP_LOG(APP_LOG_LEVEL_INFO , "line %d: old text '%s', new text '%s' size %d", line_no, old_line, new_line, size.w);

  text_layer_set_text(animating_line->layer[0], old_line);
  text_layer_set_text(animating_line->layer[1], new_line);

  Animation *animout = property_animation_get_animation(animating_line->animate_out);
  Animation *animin = property_animation_get_animation(animating_line->animate_in);
  // animation duration
  animation_set_duration(animout, ANIMATION_DURATION);
  animation_set_duration(animin, ANIMATION_DURATION);
  // animation curves
  animation_set_curve(animout, AnimationCurveEaseIn);
  animation_set_curve(animin, AnimationCurveEaseOut);
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
  animation_set_delay(animin, ANIMATION_DURATION);
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
  int i;
  
  // Let's get the new text date
  info_lines(t, new_time.topline, new_time.bottomline);

  // Let's update the top bar
  text_layer_set_text(toplayer, new_time.topline);

  // Let's update the bottom bar
  text_layer_set_text(bottomlayer, new_time.bottomline);

  // Let's get the new text time
  int hourline = fuzzy_time(t, new_time.line[0], new_time.line[1], new_time.line[2]);

  // update hour only if changed
  for(i=0; i<3; i++) {
    text_layer_set_font(line[i].layer[1], (hourline==i+1)?s_time_font_big:s_time_font);
    if(strcmp(new_time.line[i], cur_time.line[i]) != 0) updateLayer(&line[i], cur_time.line[i], new_time.line[i], hourline==i+1);
  }

  // set cur_time
  cur_time = new_time;
}


void change_align(int param) {
  GTextAlignment align = lookup_text_alignment(param);
  for(int i=0; i<3; i++) {
    for(int j=0; j<2; j++) {
      text_layer_set_text_alignment(line[i].layer[j], align);
    }
  }
}

void change_background(bool param) {
  int i;

  // background color
#ifdef PBL_COLOR
  if(param) {
    //window_set_background_color(s_main_window, GColorBlue);
    for(i=0; i<3; i++) {
      text_layer_set_text_color(line[i].layer[0], GColorIcterine);
      text_layer_set_text_color(line[i].layer[1], GColorYellow);
    }
    text_layer_set_background_color(toplayer, GColorClear);
    text_layer_set_text_color(toplayer, GColorYellow);
    text_layer_set_background_color(bottomlayer, GColorClear);
    text_layer_set_text_color(bottomlayer, GColorYellow);
  }
  else {
    //window_set_background_color(s_main_window, GColorYellow);
    for(i=0; i<3; i++) {
      text_layer_set_text_color(line[i].layer[0], GColorCobaltBlue);
      text_layer_set_text_color(line[i].layer[1], GColorBlue);
    }
    text_layer_set_background_color(toplayer, GColorClear);
    text_layer_set_text_color(toplayer, GColorBlue);
    text_layer_set_background_color(bottomlayer, GColorClear);
    text_layer_set_text_color(bottomlayer, GColorBlue);
  }
  bitmap_layer_set_compositing_mode(s_bt_bitmap_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_ch_bitmap_layer, GCompOpSet);
#else
  if(param) {
    //window_set_background_color(s_main_window, GColorBlack);
    for(i=0; i<3; i++) {
      text_layer_set_text_color(line[i].layer[0], GColorWhite);
      text_layer_set_text_color(line[i].layer[1], GColorWhite);
    }
    text_layer_set_background_color(toplayer, GColorClear);
    text_layer_set_text_color(toplayer, GColorWhite);
    text_layer_set_background_color(bottomlayer, GColorClear);
    text_layer_set_text_color(bottomlayer, GColorWhite);
    bitmap_layer_set_compositing_mode(s_bt_bitmap_layer, GCompOpAssignInverted);
    bitmap_layer_set_compositing_mode(s_ch_bitmap_layer, GCompOpAssignInverted);
    text_layer_set_background_color(batterylayer, GColorClear);
    text_layer_set_text_color(batterylayer, GColorWhite);
  }
  else {
    //window_set_background_color(s_main_window, GColorWhite);
    for(i=0; i<3; i++) {
      text_layer_set_text_color(line[i].layer[0], GColorBlack);
      text_layer_set_text_color(line[i].layer[1], GColorBlack);
    }
    text_layer_set_background_color(toplayer, GColorClear);
    text_layer_set_text_color(toplayer, GColorBlack);
    text_layer_set_background_color(bottomlayer, GColorClear);
    text_layer_set_text_color(bottomlayer, GColorBlack);
    bitmap_layer_set_compositing_mode(s_bt_bitmap_layer, GCompOpAssign);
    bitmap_layer_set_compositing_mode(s_ch_bitmap_layer, GCompOpAssign);
    text_layer_set_background_color(batterylayer, GColorClear);
    text_layer_set_text_color(batterylayer, GColorBlack);
  }
#endif
}

static void my_layer_draw(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  GColor bkcolor = isBkgdDark?PBL_IF_COLOR_ELSE(GColorBlue, GColorBlack):PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite);
  GColor botopcolor = isBkgdDark?PBL_IF_COLOR_ELSE(GColorDukeBlue, GColorBlack):PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite);
  
  // top text
#ifdef PBL_RECT
  int topwidth = 0;
  if(clock_is_24h_style() == true) {
    topwidth = 40;
  } else {
    topwidth = 60;
  }
#endif

  // Background
  // Draw a black filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, bkcolor);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Top and Bottom bars
  graphics_context_set_fill_color(ctx, botopcolor);
#if defined(PBL_RECT)
  graphics_fill_rect(ctx, GRect((bounds.size.w-topwidth)/2, bounds.origin.y, topwidth, 20), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.size.h-20, bounds.size.w, 20), 0, GCornerNone);
#elif defined(PBL_ROUND)
  // Draw a white filled circles 
  const int16_t half_h = bounds.size.h / 2;
  graphics_fill_circle(ctx, GPoint(half_h, -bounds.size.h/3-5), bounds.size.h/2);
  graphics_fill_circle(ctx, GPoint(half_h, bounds.size.h*4/3+5), bounds.size.h/2);
#endif
}


//===============================================================
//              handlers
//===============================================================
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_watch(tick_time);
}

static void battery_handler(BatteryChargeState charge_state) {
  GBitmap *bitmap_charge;
  static char s_battery_buffer[10];

  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE , "battery charge: %d%%", charge_state.charge_percent);

  if (charge_state.is_charging) {
    bitmap_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING);
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
    text_layer_set_text(batterylayer, s_battery_buffer);
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
  layer_set_hidden (text_layer_get_layer(batterylayer), !(charge_state.is_charging));

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

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "Fuzzy Modern sync_changed_handler");
  // Which key was received?
  switch(key) {
    case KEY_BCKGD_COLOR:
			isBkgdDark = (new_tuple->value->uint8 == 0);
			persist_write_bool(KEY_BCKGD_COLOR, isBkgdDark);
			APP_LOG(APP_LOG_LEVEL_INFO, "Set Background Color: %s", isBkgdDark ? "Dark" : "Light");
      change_background(isBkgdDark);
    break;

    case KEY_ALIGN:
			text_align = new_tuple->value->uint8;
			persist_write_int(KEY_ALIGN, text_align);
			APP_LOG(APP_LOG_LEVEL_INFO, "Set text alignment: %u", text_align);
      change_align(text_align);
    break;

    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)key);
    break;
  }
}

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  // An error occured!
  APP_LOG(APP_LOG_LEVEL_ERROR, "sync error!");
}

//===============================================================
//      Load/Unload
//===============================================================
static void main_window_load(Window *window) {
  int i,j;
  Layer *root_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(root_layer);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "Fuzzy Modern main_window_load");
  
  // locale
#if DEBUG
  setlocale(LC_ALL, "de_DE");
#else
  setlocale(LC_ALL, i18n_get_system_locale());
#endif
  
  // background color
  layer_set_update_proc(root_layer, my_layer_draw);
#ifdef PBL_COLOR
  //window_set_background_color(window, GColorBlue);
#else
  //window_set_background_color(window, GColorBlack);
#endif
  
  // Load GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOMESTIC_NORMAL_SUBSET_35));
  s_time_font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOMESTIC_BOLD_SUBSET_40));
  
  // Init the text layers used to show the time
  for(i=0; i<3; i++) {
    for(j=0; j<2; j++) {
      line[i].layer[j] = text_layer_create(GRect(j==0?2:i%2==1?-1*bounds.size.w:bounds.size.w, line_y[i], bounds.size.w-2, line_h));
      text_layer_set_background_color(line[i].layer[j], GColorClear);
      text_layer_set_font(line[i].layer[j], s_time_font);
      text_layer_set_overflow_mode(line[i].layer[j], GTextOverflowModeWordWrap);
    }
    line[i].out_rect = GRect(i%2==1?bounds.size.w:-1*bounds.size.w, line_y[i], bounds.size.w-2, line_h);
  }
  
  // top text
#ifdef PBL_RECT
  int topwidth = 0;
  if(clock_is_24h_style() == true) {
    topwidth = 40;
  } else {
    topwidth = 60;
  }
#endif
  //  toplayer = text_layer_create(GRect(30, 0, bounds.size.w-2*30, 18));
  toplayer = text_layer_create(GRect(PBL_IF_RECT_ELSE((bounds.size.w-topwidth)/2, 0), -2, PBL_IF_RECT_ELSE(topwidth,bounds.size.w), 18));
  text_layer_set_text_alignment(toplayer, GTextAlignmentCenter);
  text_layer_set_font(toplayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  
  // bottom text
  bottomlayer = text_layer_create(GRect(0, bounds.size.h-22, bounds.size.w, 22));
  text_layer_set_text_alignment(bottomlayer, GTextAlignmentCenter);
  text_layer_set_font(bottomlayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  // battery text
  batterylayer = text_layer_create(GRect(bounds.size.w-21-33-PBL_IF_RECT_ELSE(0,8), PBL_IF_RECT_ELSE(-3,50), 33, 18));
  text_layer_set_background_color(batterylayer, GColorClear);
  text_layer_set_font(batterylayer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(batterylayer, GTextAlignmentRight);
  
  // Create charging GBitmap, then set to created BitmapLayer
  s_ch_bitmap_layer = bitmap_layer_create(GRect(bounds.size.w-21-PBL_IF_RECT_ELSE(0,8), PBL_IF_RECT_ELSE(0,53), 20, 17));
  bitmap_layer_set_background_color(s_ch_bitmap_layer, GColorClear); 
  s_bitmap_charging = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGE10);
  
  // Create bluetooth GBitmap, then set to created BitmapLayer
  s_bitmap_bt_on = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_ON);
  s_bitmap_bt_off = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_OFF);
  s_bt_bitmap_layer = bitmap_layer_create(GRect(PBL_IF_RECT_ELSE(0,8), PBL_IF_RECT_ELSE(0,50), 15, 22));
    
  // apply settings
  change_background(isBkgdDark);
  change_align(text_align);
  
  // add layers
  for(i=0; i<3; i++) {
    for(j=0; j<2; j++) {
      layer_add_child(root_layer, text_layer_get_layer(line[i].layer[j]));
    }
  }
  layer_add_child(root_layer, text_layer_get_layer(batterylayer));
  layer_add_child(root_layer, text_layer_get_layer(toplayer));
  layer_add_child(root_layer, text_layer_get_layer(bottomlayer));
  layer_add_child(root_layer, bitmap_layer_get_layer(s_bt_bitmap_layer));
  layer_add_child(root_layer, bitmap_layer_get_layer(s_ch_bitmap_layer));
  
  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  t = localtime(&now);
  update_watch(t);
    
  battery_handler(battery_state_service_peek());
#ifdef PBL_SDK_2
  bt_handler(bluetooth_connection_service_peek());
#elif PBL_SDK_3
  bt_handler(connection_service_peek_pebble_app_connection());
#endif

  #if DEBUG==0
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
#endif
  // Register with BatteryService
  battery_state_service_subscribe(battery_handler);
  // Register for Bluetooth connection updates
#ifdef PBL_SDK_2
  bluetooth_connection_service_subscribe(bt_handler);
#elif PBL_SDK_3
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bt_handler
});
#endif
  
#if DEBUG
	// Button functionality
	window_set_click_config_provider(s_main_window, (ClickConfigProvider) click_config_provider);
#endif
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
#if defined(PBL_SDK_2)
  bluetooth_connection_service_unsubscribe();
#elif defined(PBL_SDK_3)
  connection_service_unsubscribe();
#endif

  // Destroy TextLayers
  for(int i=0; i<3; i++) {
    for(int j=0; j<2; j++) {
      text_layer_destroy(line[i].layer[j]);
    }
  }
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

//===============================================================
//      Init/Deinit
//===============================================================
static void init() {
  APP_LOG(APP_LOG_LEVEL_DEBUG_VERBOSE, "Fuzzy Modern init");
	// Load settings from persistent storage
	if (persist_exists(KEY_BCKGD_COLOR))
	{
		isBkgdDark = persist_read_bool(KEY_BCKGD_COLOR);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Read Background Color from storage : %s", isBkgdDark ? "Dark" : "Light");
	}
	if (persist_exists(KEY_ALIGN))
	{
		text_align = persist_read_int(KEY_ALIGN);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Read text alignment from storage: %u", text_align);
	}
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
#if DEBUG
  window_set_fullscreen(s_main_window, true);
#endif
  
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Open AppMessage
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);

  // Setup initial values
  Tuplet initial_values[] = {
    TupletInteger(KEY_BCKGD_COLOR, (uint8_t)!isBkgdDark),
    TupletInteger(KEY_ALIGN,       (uint8_t)text_align)
  };

  // Begin using AppSync
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_changed_handler, sync_error_handler, NULL);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);

  // Finish using AppSync
  app_sync_deinit(&s_sync);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
