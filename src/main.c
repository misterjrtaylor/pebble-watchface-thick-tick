#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_battery_layer;
static TextLayer *s_connection_layer;

static TextLayer *s_hours_layer;
static TextLayer *s_minutes_layer;
static TextLayer *s_date_layer;

static GFont s_font_light_48;
static GFont s_font_bold_48;

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_hours_text[] = "00";
  static char s_minutes_text[] = "00";
  static char date_buffer[7];
	
  if(clock_is_24h_style() == true) {
     strftime(s_hours_text, sizeof(s_hours_text), "%H", tick_time);
  } else {
	 strftime(s_hours_text, sizeof(s_hours_text), "%I", tick_time);
  }
	
  strftime(date_buffer, sizeof(date_buffer), "%d %b", tick_time);
	
  strftime(s_minutes_text, sizeof(s_minutes_text), "%M", tick_time);
	
  text_layer_set_text(s_hours_layer, s_hours_text);
  text_layer_set_text(s_minutes_layer, s_minutes_text);
  text_layer_set_text(s_date_layer, date_buffer);
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(s_connection_layer, connected ? "connected" : "disconnected");
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
	
  int fontHeight = 48;
  int gap = 3;
  int offset = 12;
	
  s_hours_layer = text_layer_create(GRect(0, (bounds.size.h - (2 * fontHeight + gap) ) / 2 ,bounds.size.w,48));
  #if defined(PBL_SDK_2)
	text_layer_set_text_color(s_hours_layer, GColorWhite);
  #elif defined(PBL_SDK_3)
	text_layer_set_text_color(s_hours_layer, GColorVividCerulean);
  #endif
  text_layer_set_background_color(s_hours_layer, GColorClear);
	
  s_font_bold_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_SANS_BOLD_48));
  text_layer_set_font(s_hours_layer, s_font_bold_48);
	
  text_layer_set_text_alignment(s_hours_layer, GTextAlignmentCenter);
	
  s_minutes_layer = text_layer_create(GRect(0,(bounds.size.h - (2 * fontHeight + gap) ) / 2 + gap + fontHeight - offset,bounds.size.w,50));
  text_layer_set_text_color(s_minutes_layer, GColorWhite);
  text_layer_set_background_color(s_minutes_layer, GColorClear);
	
  s_font_light_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_SANS_LIGHT_48));
  text_layer_set_font(s_minutes_layer, s_font_light_48);
	
  text_layer_set_text_alignment(s_minutes_layer, GTextAlignmentCenter);

  s_connection_layer = text_layer_create(GRect(0, 20, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorWhite);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentCenter);
	
  s_date_layer = text_layer_create(GRect(0, 130, bounds.size.w, 34));
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	
#if defined(PBL_SDK_2)
  handle_bluetooth(bluetooth_connection_service_peek());
#elif defined(PBL_SDK_3)
  handle_bluetooth(connection_service_peek_pebble_app_connection());
#endif

  s_battery_layer = text_layer_create(GRect(0, 30, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "100% charged");

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  battery_state_service_subscribe(handle_battery);

#if defined(PBL_SDK_2)
  bluetooth_connection_service_subscribe(handle_bluetooth);
#elif defined(PBL_SDK_3)
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_bluetooth
  });
#endif
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_hours_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minutes_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
  handle_battery(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
#if defined(PBL_SDK_2)
  bluetooth_connection_service_unsubscribe();
#elif defined(PBL_SDK_3)
  connection_service_unsubscribe();
#endif
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_hours_layer);
  text_layer_destroy(s_minutes_layer);
  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
  fonts_unload_custom_font(s_font_bold_48);
  fonts_unload_custom_font(s_font_light_48);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}