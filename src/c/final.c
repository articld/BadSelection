#include <pebble.h>

#define _NUM_ELEM_ 60

static Window *s_window;

static TextLayer *s_time_layer;

static TextLayer *s_textgrid_layer[_NUM_ELEM_];

static GFont s_time_font, s_textgrid_font;


static void update_time(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
    update_time();
}

static void text_grid(Layer *time_layer){
    srand(time(NULL));

    int x_coord = 0;
    int y_coord = 0;

    s_textgrid_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IBM_REGULAR_21));

    for(int i=0; i<_NUM_ELEM_; i++){
            x_coord = 2 + (i % 10) * 14;
        if(i) y_coord = (i % 10)? y_coord : y_coord + 28;

        s_textgrid_layer[i] = text_layer_create(GRect(x_coord, y_coord, 13, 25));

        text_layer_set_font(s_textgrid_layer[i], s_textgrid_font);
        text_layer_set_background_color(s_textgrid_layer[i], GColorClear);
        text_layer_set_text_color(s_textgrid_layer[i], GColorLightGray);
        text_layer_set_text_alignment(s_textgrid_layer[i], GTextAlignmentLeft);
        text_layer_set_text(s_textgrid_layer[i], "a");

        layer_insert_below_sibling(text_layer_get_layer(s_textgrid_layer[i]), time_layer); 
    }
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    window_set_background_color(s_window, GColorClear);


    //the font size is probably right +-1
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IBM_BOLD_35));

    s_time_layer = text_layer_create(GRect(0, 61, 112, 46));

    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_background_color(s_time_layer, GColorBlue);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
    
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    text_grid(text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    for(int i=0; i<_NUM_ELEM_; i++) 
        text_layer_destroy(s_textgrid_layer[i]);

    text_layer_destroy(s_time_layer);
    fonts_unload_custom_font(s_time_font);
    fonts_unload_custom_font(s_textgrid_font);
}

static void init(void) {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
    });
    const bool animated = true;
    window_stack_push(s_window, animated);

    update_time();
}

static void deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
