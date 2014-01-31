#include <pebble.h>

static Window     *window;
static TextLayer  *big_time_layer;
static TextLayer  *seconds_time_layer;
static TextLayer  *author_layer;

#define FONT_HOURS    RESOURCE_ID_FONT_DEJAVU_SANS_BOLD_SUBSET_30
#define FONT_SECONDS  RESOURCE_ID_FONT_DEJAVU_SANS_SUBSET_18

static AppTimer* timer        = NULL;
static bool is_timer_started  = false;
static double elapsed_time    = 0, start_time = 0;

static GFont big_font;
static GFont seconds_font;

static void start_timer(void);
static void stop_timer(void);
static double get_time_diff(void);
static void update_timer(void);
static void select_click_handler(ClickRecognizerRef recognizer, void *context);
static void handle_reset_timer(ClickRecognizerRef recognizer, Window *window);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);
static void handle_timer(void* data);
static void init(void);
static void deinit(void);


static void start_timer() {
  is_timer_started = true;
  if(start_time == 0) start_time = get_time_diff();
  timer = app_timer_register(100, handle_timer, NULL);
}

static void stop_timer(void) {
  is_timer_started = false;
  if(timer != NULL) {
      app_timer_cancel(timer);
      timer = NULL;
  }
}

static double get_time_diff(void) {
  time_t seconds;
  uint16_t milliseconds;
  time_ms(&seconds, &milliseconds);
  return (double)seconds + ((double)milliseconds / 1000.0);
}

static void update_timer() {
  static char big_time[] = "00:00";
  static char seconds_time[] = ":00";
  static char deciseconds_time[] = ".0";

  // Now convert to hours/minutes/seconds.
  int tenths = (int)(elapsed_time * 10) % 10;
  int seconds = (int)elapsed_time % 60;
  int minutes = (int)elapsed_time / 60 % 60;
  int hours = (int)elapsed_time / 3600;

  if(hours > 99) {
      stop_timer();
      return;
  }

  if(hours < 1) {
    snprintf(big_time, 6, "%02d:%02d", minutes, seconds);
    snprintf(deciseconds_time, 3, ".%d", tenths);
  } else {
    snprintf(big_time, 6, "%02d:%02d", hours, minutes);
    snprintf(seconds_time, 4, ":%02d", seconds);
  }

  text_layer_set_text(big_time_layer, big_time);
  text_layer_set_text(seconds_time_layer, hours < 1 ? deciseconds_time : seconds_time);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if(is_timer_started) {
        stop_timer();
    } else {
        start_timer();
    }
}

static void handle_reset_timer(ClickRecognizerRef recognizer, Window *window) {
  stop_timer();

  elapsed_time = 0;
  start_time = 0;

  text_layer_set_text(big_time_layer, "00:00");
  text_layer_set_text(seconds_time_layer, ".0" );
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, (ClickHandler)handle_reset_timer, NULL);
}

static void window_load(Window *window) {
  Layer *root_layer = window_get_root_layer(window);

  big_time_layer = text_layer_create(GRect(5, 25, 105, 35));
  text_layer_set_background_color(big_time_layer, GColorBlack);
  text_layer_set_font(big_time_layer, big_font);
  text_layer_set_text_color(big_time_layer, GColorWhite);
  text_layer_set_text(big_time_layer, "00:00");
  text_layer_set_text_alignment(big_time_layer, GTextAlignmentRight);
  layer_add_child(root_layer, (Layer*)big_time_layer);

  seconds_time_layer = text_layer_create(GRect(110, 37, 49, 35));
  text_layer_set_background_color(seconds_time_layer, GColorBlack);
  text_layer_set_font(seconds_time_layer, seconds_font);
  text_layer_set_text_color(seconds_time_layer, GColorWhite);
  text_layer_set_text(seconds_time_layer, ".0");
  layer_add_child(root_layer, (Layer*)seconds_time_layer);

  author_layer = text_layer_create(GRect(50, 100, 50, 35));
  text_layer_set_background_color(author_layer, GColorBlack);
  text_layer_set_font(author_layer, seconds_font);
  text_layer_set_font(author_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_color(author_layer, GColorWhite);
  text_layer_set_text(author_layer, "A.J");
  layer_add_child(root_layer, (Layer*)author_layer);

}

static void window_unload(Window *window) {
  text_layer_destroy(author_layer);
  text_layer_destroy(big_time_layer);
  text_layer_destroy(seconds_time_layer);
}

static void handle_timer(void* data) {
  if(is_timer_started) {
    double now = get_time_diff();
    elapsed_time = now - start_time;
    timer = app_timer_register(100, handle_timer, NULL);
  }
  update_timer();
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, false);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  big_font = fonts_load_custom_font(resource_get_handle(FONT_HOURS));
  seconds_font = fonts_load_custom_font(resource_get_handle(FONT_SECONDS));
  window_stack_push(window, true);
}

static void deinit(void) {
  fonts_unload_custom_font(big_font);
  fonts_unload_custom_font(seconds_font);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
