#include <pebble.h>
#include "animations.h"

#define _NUM_ELEM_ 60
#define _SETTINGS_KEY_ 1

#if defined (PBL_COLOR)

#define _BG_COLOR_ GColorBlack
#define _ACCENT_BG_COLOR_ GColorDukeBlue
#define _TIME_COLOR_ GColorWhite
#define _TEXTGRID_COLOR_ GColorDarkGray

#else

#define _BG_COLOR_ GColorBlack
#define _ACCENT_BG_COLOR_ GColorWhite
#define _TIME_COLOR_ GColorBlack
#define _TEXTGRID_COLOR_ GColorWhite

#endif

//STATIC DECLARATIONS
//-----------------------------------------------------------------------------

static Window *s_window;

typedef struct{
    TextLayer *hours, *minutes, *separator;

} TL;
static TL s_TimeLayer;

typedef struct{
    GColor bg_color, accent_color, time_color, textgrid_color;
} ClaySettings;
static ClaySettings settings;

static Layer *s_TimeLayer_bg;

static TextLayer *s_textgrid_elements[_NUM_ELEM_];

static GFont s_time_font, s_textgrid_font;
//static GFont s_date_font;


//TIMEKEEPING
//-----------------------------------------------------------------------------
static void update_time(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_h_buffer[4];
    static char s_m_buffer[4];

    strftime(s_h_buffer, sizeof(s_h_buffer), "%H", tick_time);
    strftime(s_m_buffer, sizeof(s_m_buffer), "%M", tick_time);

    text_layer_set_text(s_TimeLayer.hours, s_h_buffer);
    text_layer_set_text(s_TimeLayer.minutes, s_m_buffer);
}

/* TODO: put the current date inside the textgrid
   Hopefully that won't involve rewriting the whole thing.
   Surely it won't, right?
   
static void update_date(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    static char s_textgrid_buffer[8][2] = {{'a', '\0'}};

    s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IBM_BOLD_21));

    strftime(s_buffer, sizeof(s_buffer), "%a", tick_time);

    //we have a toupper function at home.
    //the toupper function:
    for(int i = 1; i < 3; i++){
        s_buffer[i] = s_buffer[i] - 32;
    }

    for(int i = 0; i < 3; i++){
        s_textgrid_buffer[i][0] = s_buffer[i];

        text_layer_set_font(s_textgrid_elements[i], s_date_font);
        text_layer_set_text_color(s_textgrid_elements[i], GColorDarkGray);
        text_layer_set_text(s_textgrid_elements[i], s_textgrid_buffer[i]);
    }
}
*/

static void tick_minute_handler(struct tm *tick_time, TimeUnits units_changed){
    animate_grid(s_textgrid_elements);
    select_animate_time(s_TimeLayer.minutes, s_TimeLayer.hours);
}

//LAYER CREATION
//-----------------------------------------------------------------------------

static void shuffle_grid_text(){
    static char s_str[_NUM_ELEM_][2] = {{'a', '\0'}};

    for(int i = 0; i<_NUM_ELEM_; i++){
        s_str[i][0] = rand() % 26 + 97;
        text_layer_set_text(s_textgrid_elements[i], s_str[i]);
    }
}

static void create_text_grid(Layer *target){
    int x_coord = 0;
    int y_coord = 0;
    s_textgrid_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FEATURE_MONO_REGULAR_20));

    for(int i=0; i<_NUM_ELEM_; i++){
        x_coord = 3 + (i % 10) * 14;
        if(i) y_coord = (i % 10)? y_coord : y_coord + 27;

        s_textgrid_elements[i] = text_layer_create(GRect(x_coord, y_coord, 13, 25));
        if(y_coord>55 && y_coord <107 && x_coord< 111) layer_set_hidden(text_layer_get_layer(s_textgrid_elements[i]), true);

        text_layer_set_font(s_textgrid_elements[i], s_textgrid_font);
        text_layer_set_background_color(s_textgrid_elements[i], GColorClear);
        text_layer_set_text_color(s_textgrid_elements[i], settings.textgrid_color);
        text_layer_set_text_alignment(s_textgrid_elements[i], GTextAlignmentLeft);
        layer_add_child(target, text_layer_get_layer(s_textgrid_elements[i])); 
    }

    shuffle_grid_text();
}

static void draw_timebox_canvas(Layer *layer, GContext *ctx){
    graphics_context_set_stroke_color(ctx, settings.accent_color);
    graphics_context_set_fill_color(ctx, settings.accent_color);    

    GRect rect_bounds = layer_get_bounds(layer);
    graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone); 
}

static void create_time(Layer *target){
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FEATURE_MONO_BLACK_35));
    GRect time_box = GRect(0, 59, 112, 48);

    const int time_x = time_box.origin.x + 3;

    const int time_h = 48;
    const int time_y = time_box.origin.y;

    const int time_w = 44;
    const int time_w_separator = 24;

    s_TimeLayer.hours = text_layer_create(GRect(time_x, time_y, time_w, time_h));
    s_TimeLayer.separator =text_layer_create(GRect(time_x + time_w, time_y, time_w_separator, time_h));
    s_TimeLayer.minutes = text_layer_create(GRect(time_x + time_w + time_w_separator - 5, time_y, time_w, time_h));

    s_TimeLayer_bg = layer_create(time_box);
    layer_set_update_proc(s_TimeLayer_bg, draw_timebox_canvas);
    layer_add_child(target, s_TimeLayer_bg);
    layer_mark_dirty(s_TimeLayer_bg);

    text_layer_set_font(s_TimeLayer.hours, s_time_font);
    text_layer_set_text_color(s_TimeLayer.hours, settings.time_color);
    text_layer_set_background_color(s_TimeLayer.hours, GColorClear);
    text_layer_set_text_alignment(s_TimeLayer.hours, GTextAlignmentLeft);

    text_layer_set_font(s_TimeLayer.minutes, s_time_font);
    text_layer_set_text_color(s_TimeLayer.minutes, settings.time_color);
    text_layer_set_background_color(s_TimeLayer.minutes, GColorClear);
    text_layer_set_text_alignment(s_TimeLayer.minutes, GTextAlignmentLeft);

    text_layer_set_font(s_TimeLayer.separator, s_time_font);
    text_layer_set_text_color(s_TimeLayer.separator, settings.time_color );
    text_layer_set_background_color(s_TimeLayer.separator, GColorClear);
    text_layer_set_text_alignment(s_TimeLayer.separator, GTextAlignmentLeft);
    text_layer_set_text(s_TimeLayer.separator, ":");

    layer_add_child(target, text_layer_get_layer(s_TimeLayer.hours));
    layer_add_child(target, text_layer_get_layer(s_TimeLayer.minutes));
    layer_add_child(target, text_layer_get_layer(s_TimeLayer.separator));
}

//CONFIGDATA
//-----------------------------------------------------------------------------

static void default_settings(){
    //might as well use what i've already written
    settings.bg_color = _BG_COLOR_;
    settings.accent_color = _ACCENT_BG_COLOR_;
    settings.time_color = _TIME_COLOR_; 
    settings.textgrid_color = _TEXTGRID_COLOR_; 
}

static void save_settings(){
    persist_write_data(_SETTINGS_KEY_, &settings, sizeof(settings));
}

static void load_settings(){
    default_settings();
    persist_read_data(_SETTINGS_KEY_, &settings, sizeof(settings));
}

static void update_settings(){
    text_layer_set_text_color(s_TimeLayer.hours, settings.time_color);
    text_layer_set_text_color(s_TimeLayer.minutes, settings.time_color);
    text_layer_set_text_color(s_TimeLayer.separator, settings.time_color);

    for(int i = 0; i < _NUM_ELEM_; i++){
        text_layer_set_text_color(s_textgrid_elements[i], settings.textgrid_color);
    }

    window_set_background_color(s_window, settings.bg_color);

    layer_set_update_proc(s_TimeLayer_bg, draw_timebox_canvas);
    layer_mark_dirty(s_TimeLayer_bg);
}

static void config_data_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *accent_color_t = dict_find(iter, MESSAGE_KEY_AccentColor);
    if(accent_color_t) {
        settings.accent_color = GColorFromHEX(accent_color_t->value->int32);
    }

    Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BGColor);
    if(bg_color_t) {
        settings.bg_color = GColorFromHEX(bg_color_t->value->int32);
    }

    Tuple *time_color_t = dict_find(iter, MESSAGE_KEY_TimeColor);
    if(time_color_t) {
        settings.time_color = GColorFromHEX(time_color_t->value->int32);
    }   

    Tuple *textgrid_color_t = dict_find(iter, MESSAGE_KEY_TextGridColor);
    if(textgrid_color_t) {
        settings.textgrid_color = GColorFromHEX(textgrid_color_t->value->int32);
    } 

    save_settings();
    update_settings();
}

//MAIN LOADERS/UNLOADERS
//-----------------------------------------------------------------------------

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    window_set_background_color(s_window, settings.bg_color);

    create_text_grid(window_layer);
    create_time(window_layer);

}

static void main_window_unload(Window *window) {
    for(int i=0; i<_NUM_ELEM_; i++) 
        text_layer_destroy(s_textgrid_elements[i]);

    text_layer_destroy(s_TimeLayer.hours);
    text_layer_destroy(s_TimeLayer.minutes);
    text_layer_destroy(s_TimeLayer.separator);

    layer_destroy(s_TimeLayer_bg);

    fonts_unload_custom_font(s_time_font);
    fonts_unload_custom_font(s_textgrid_font);
    //fonts_unload_custom_font(s_date_font);
}

static void init(void) {
    load_settings();

    srand(time(NULL));
    tick_timer_service_subscribe(MINUTE_UNIT, tick_minute_handler);

    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
    });
    const bool animated = true;
    window_stack_push(s_window, animated);

    app_message_register_inbox_received(config_data_received_handler);
    app_message_open(128,128);

    update_time();
    //update_date();
}

static void deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
