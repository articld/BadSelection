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

enum TimeBoxPosition{
    LEFT,
    MIDDLE,
    RIGHT
};

typedef struct{
    GColor bg_color, accent_color, time_color, textgrid_color;
    bool textgrid_animation;
    enum TimeBoxPosition time_box_position;
} ClaySettings;
static ClaySettings settings;

typedef struct{
    TextLayer *hours, *minutes, *separator;

} TL;

static TL s_TimeLayers;
static Layer *s_TimeLayer_bg;
static TextLayer *s_textgrid_elements[_NUM_ELEM_];

static GFont s_time_font, s_textgrid_font;
//static GFont s_date_font;


static GRect s_time_box;

//UTILITY FUNCTIONS
//-----------------------------------------------------------------------------

static void text_layer_set_style(TextLayer *text, GFont font, GColor text_color, GColor bg_color){
        text_layer_set_font(text, font);
        text_layer_set_text_color(text, text_color);
        text_layer_set_background_color(text, bg_color);
}

static void shuffle_textgrid(){
    static char s_str[_NUM_ELEM_][2] = {{'a', '\0'}};

    for(uint i = 0; i<_NUM_ELEM_; i++){
        s_str[i][0] = rand() % 26 + 97;
        text_layer_set_text(s_textgrid_elements[i], s_str[i]);
    }
}



//TIMEKEEPING
//-----------------------------------------------------------------------------

static void update_time(){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_h_buffer[4];
    static char s_m_buffer[4];

    strftime(s_h_buffer, sizeof(s_h_buffer), "%H", tick_time);
    strftime(s_m_buffer, sizeof(s_m_buffer), "%M", tick_time);

    text_layer_set_text(s_TimeLayers.hours, s_h_buffer);
    text_layer_set_text(s_TimeLayers.minutes, s_m_buffer);
}

/* 

    !!!!!!! WARNING !!!!!!!!!!
    RICORDATI DI INIZIALIZZARE LE VARIABILI

static void update_date(){
    //TODO: settings key for killing last row of textgrid
    //Animate indipendently the date from the rest of the grid
    //Put text accordingly

    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FEATURE_MONO_BOLD_20));

    static char s_mday[3];
    static char s_month[3];
    static char s_year[5];

    strftime(s_mday, sizeof(s_mday), "%d", tick_time);
    strftime(s_month, sizeof(s_month), "%m", tick_time);
    strftime(s_year, sizeof(s_year), "%Y", tick_time);

    uint x_coord = 0;
    uint y_coord = 0;

    for(uint i=0; i < sizeof(s_dategrid_elements); i++){
        x_coord = 3 + i * 14;
        y_coord = 135;

        s_dategrid_elements[i] = text_layer_create(GRect(x_coord, y_coord, 13, 25));
        text_layer_set_style(s_dategrid_elements[i], s_textgrid_font, settings.textgrid_color, GColorClear);
        layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_dategrid_elements[i])); 
    }    

}
   

*/

static void tick_minute_handler(struct tm *tick_time, TimeUnits units_changed){
    if(settings.textgrid_animation)
        animate_grid(s_textgrid_elements);
    else
        shuffle_textgrid();

    select_animate_time(s_TimeLayers.minutes, s_TimeLayers.hours, settings.textgrid_animation);
}

//LAYER CREATION
//-----------------------------------------------------------------------------

static void update_textgrid_visibility(){
    int x_coord = 0;
    int y_coord = 0;

    for(uint i=0; i<_NUM_ELEM_; i++){
        Layer *current_element_layer = text_layer_get_layer(s_textgrid_elements[i]);
        x_coord = layer_get_frame(current_element_layer).origin.x;
        y_coord = layer_get_frame(current_element_layer).origin.y;

        if(y_coord > s_time_box.origin.y && y_coord < s_time_box.size.h && x_coord > s_time_box.origin.x && x_coord < s_time_box.size.w) 
            layer_set_hidden(text_layer_get_layer(s_textgrid_elements[i]), true);

        layer_mark_dirty(text_layer_get_layer(s_textgrid_elements[i]));
    }
}

static void create_textgrid(Layer *target){
    uint x_coord = 0;
    uint y_coord = 0;
    s_textgrid_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FEATURE_MONO_REGULAR_20));

    for(uint i=0; i<_NUM_ELEM_; i++){
        x_coord = 3 + (i % 10) * 14;
        if(i) y_coord = (i % 10)? y_coord : y_coord + 27;

        s_textgrid_elements[i] = text_layer_create(GRect(x_coord, y_coord, 13, 25));
        text_layer_set_style(s_textgrid_elements[i], s_textgrid_font, settings.textgrid_color, GColorClear);
        layer_add_child(target, text_layer_get_layer(s_textgrid_elements[i])); 
    }

    update_textgrid_visibility();
    shuffle_textgrid();
}

static void draw_timebox_canvas(Layer *layer, GContext *ctx){
    graphics_context_set_stroke_color(ctx, settings.accent_color);
    graphics_context_set_fill_color(ctx, settings.accent_color);    

    GRect rect_bounds = layer_get_bounds(layer);
    graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone); 
}

static void select_time_box_coords(){
    switch(settings.time_box_position){
        case LEFT:
            s_time_box = GRect(0, 59, 112, 48);
            break;
        case MIDDLE:
            s_time_box = GRect(16, 59, 112, 48);
            break;
        case RIGHT:
            s_time_box = GRect(31, 59, 112, 48);
    }
}

static void create_time(Layer *target){
    //doubles up as first time creation and on settings change update
    select_time_box_coords();

    if(!s_time_font)
        s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FEATURE_MONO_BLACK_35));

    const int time_x = s_time_box.origin.x + 3;

    const int time_h = 48;
    const int time_y = s_time_box.origin.y;

    const int time_w = 44;
    const int time_w_separator = 24;

    if(!s_TimeLayer_bg)
        s_TimeLayer_bg = layer_create(GRectZero);

    layer_set_frame(s_TimeLayer_bg, s_time_box);
    layer_set_update_proc(s_TimeLayer_bg, draw_timebox_canvas);
    layer_add_child(target, s_TimeLayer_bg);
    layer_mark_dirty(s_TimeLayer_bg);

    if(!s_TimeLayers.hours){
        s_TimeLayers.hours = text_layer_create(GRect(time_x, time_y, time_w, time_h));
        s_TimeLayers.separator = text_layer_create(GRect(time_x + time_w, time_y, time_w_separator, time_h));
        s_TimeLayers.minutes = text_layer_create(GRect(time_x + time_w + time_w_separator - 5, time_y, time_w, time_h));

        text_layer_set_style(s_TimeLayers.hours, s_time_font, settings.time_color, GColorClear);
        text_layer_set_style(s_TimeLayers.minutes, s_time_font, settings.time_color, GColorClear);
        text_layer_set_style(s_TimeLayers.separator, s_time_font, settings.time_color, GColorClear);
        text_layer_set_text(s_TimeLayers.separator, ":");

        layer_insert_above_sibling(text_layer_get_layer(s_TimeLayers.hours), s_TimeLayer_bg);
        layer_insert_above_sibling(text_layer_get_layer(s_TimeLayers.minutes), s_TimeLayer_bg);
        layer_insert_above_sibling(text_layer_get_layer(s_TimeLayers.separator), s_TimeLayer_bg);
    }

    else{
        text_layer_set_text_color(s_TimeLayers.hours, settings.time_color);
        text_layer_set_text_color(s_TimeLayers.minutes, settings.time_color);
        text_layer_set_text_color(s_TimeLayers.separator, settings.time_color); 

        Layer *hours_layer = text_layer_get_layer(s_TimeLayers.hours);
        Layer *separator_layer = text_layer_get_layer(s_TimeLayers.separator);
        Layer *minutes_layer = text_layer_get_layer(s_TimeLayers.minutes);

        layer_set_frame(hours_layer, GRect(time_x, time_y, time_w, time_h));
        layer_set_frame(separator_layer, GRect(time_x + time_w, time_y, time_w_separator, time_h));
        layer_set_frame(minutes_layer, GRect(time_x + time_w + time_w_separator - 5, time_y, time_w, time_h));

        //FIXME: this horrible mess shouldn't be required.
        layer_insert_above_sibling(hours_layer, s_TimeLayer_bg);
        layer_insert_above_sibling(separator_layer, s_TimeLayer_bg);
        layer_insert_above_sibling(minutes_layer, s_TimeLayer_bg);
    }

}

//CONFIGDATA
//-----------------------------------------------------------------------------

static void default_settings(){
    //might as well use what i've already written
    settings.bg_color = _BG_COLOR_;
    settings.accent_color = _ACCENT_BG_COLOR_;
    settings.time_color = _TIME_COLOR_; 
    settings.textgrid_color = _TEXTGRID_COLOR_; 

    settings.textgrid_animation = true;
    settings.time_box_position = LEFT;
}

static void save_settings(){
    persist_write_data(_SETTINGS_KEY_, &settings, sizeof(settings));
}

static void load_settings(){
    default_settings();
    persist_read_data(_SETTINGS_KEY_, &settings, sizeof(settings));
}

static void update_settings(){
    for(uint i = 0; i < _NUM_ELEM_; i++){
        text_layer_set_text_color(s_textgrid_elements[i], settings.textgrid_color);
    }

    create_time(window_get_root_layer(s_window));
    update_textgrid_visibility();
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

    Tuple *textgrid_animation_t = dict_find(iter, MESSAGE_KEY_TextGridAnimation);
    if(textgrid_animation_t){
        settings.textgrid_animation = textgrid_animation_t->value->int32 == 1;
    }

    Tuple *time_box_position_t = dict_find(iter, MESSAGE_KEY_TimeBoxPosition);
    if(time_box_position_t){
        //idk why but this thing only sends strings
        //FIXME: this works but i kinda need to see what's going on.
        settings.time_box_position = time_box_position_t->value->int32 - 48;
    }

    save_settings();
    update_settings();
}

//MAIN LOADERS/UNLOADERS
//-----------------------------------------------------------------------------

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    window_set_background_color(s_window, settings.bg_color);

    create_textgrid(window_layer);
    create_time(window_layer);

}

static void main_window_unload(Window *window) {
    for(uint i=0; i<_NUM_ELEM_; i++) 
        text_layer_destroy(s_textgrid_elements[i]);

    text_layer_destroy(s_TimeLayers.hours);
    text_layer_destroy(s_TimeLayers.minutes);
    text_layer_destroy(s_TimeLayers.separator);

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
