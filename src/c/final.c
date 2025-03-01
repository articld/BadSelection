#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;

static GFont s_time_font;

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);


  //the font size is probably right +-1
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IBM_BOLD_35));

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 50));
  text_layer_set_font(s_text_layer, s_time_font);
  text_layer_set_text(s_text_layer, "10:10");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void init(void) {
  s_window = window_create();

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
