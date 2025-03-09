#include <pebble.h>
#define _NUM_ELEM_ 60

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


static Window *s_window;

typedef struct{
    TextLayer *hours, *minutes, *separator;

} TL;
static TL s_TimeLayer;

static Layer *s_TimeLayer_bg;

static TextLayer *s_textgrid_elements[_NUM_ELEM_];

static int s_animation_counter = 0;

static GFont s_time_font, s_textgrid_font;
//static GFont s_date_font;

static void shuffle_grid_text(){
    static char s_str[_NUM_ELEM_][2] = {{'a', '\0'}};

    for(int i = 0; i<_NUM_ELEM_; i++){
        s_str[i][0] = rand() % 26 + 97;
        text_layer_set_text(s_textgrid_elements[i], s_str[i]);
    }
}

//TEXTGRID ANIMATION SHENENIGANS
//-----------------------------------------------------------------------------

static void anim_started_handler(Animation *animation, void *context) {
    return;
}

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
    static char s_str[_NUM_ELEM_][2] = {{'a', '\0'}};
    s_str[s_animation_counter][0] = rand() % 26 + 97;

    text_layer_set_text(context, s_str[s_animation_counter]);

    s_animation_counter = (s_animation_counter + 1) % 60;
}

static void anim_1_started_handler(Animation *animation, void *context) {
    return;
}

static void anim_1_stopped_handler(Animation *animation, bool finished, void *context) {
    return;
}

static void animate_grid(){
    if(!s_animation_counter){
        const int array_length = _NUM_ELEM_ * 2;
        Animation **arr = (Animation**)malloc(array_length * sizeof(Animation*));

        const int duration_ms = 300;
        int delay_ms = 0;

        for(int i=0; i< array_length; i++){
            if(i % 2 == 0){
                int j = i / 2;

                GRect start = layer_get_frame(text_layer_get_layer(s_textgrid_elements[j])); 
                GRect finish = layer_get_frame(text_layer_get_layer(s_textgrid_elements[j]));
                finish.origin.y = finish.origin.y + finish.size.h;
                finish.size.h = 0;

                PropertyAnimation *prop_anim = property_animation_create_layer_frame(text_layer_get_layer(s_textgrid_elements[j]), &start, &finish);
                Animation *anim = property_animation_get_animation(prop_anim);
                if(!layer_get_hidden(text_layer_get_layer(s_textgrid_elements[j])))
                    delay_ms += 75;

                animation_set_curve(anim, AnimationCurveEaseIn);
                animation_set_delay(anim, delay_ms);
                animation_set_duration(anim, duration_ms);

                animation_set_handlers(anim, (AnimationHandlers){
                    .started = anim_started_handler,
                    .stopped = anim_stopped_handler
                }, s_textgrid_elements[j]);

                arr[i] = anim;
            }

            else{
                int j = i - 1;
                Animation *anim_1 = animation_clone(arr[j]);
                animation_set_reverse(anim_1, true);
                animation_set_delay(anim_1, delay_ms + duration_ms + 120);
                animation_set_handlers(anim_1, (AnimationHandlers){
                    .started = anim_1_started_handler,
                    .stopped = anim_1_stopped_handler
                }, NULL);

                arr[i] = anim_1;
            }
        }

        Animation *spawn = animation_spawn_create_from_array(arr , array_length);
        animation_schedule(spawn);

        free(arr);
    }

}

//HOUR AND MINUTE ANIMATIONS
//-----------------------------------------------------------------------------

static void time_anim_started_handler(Animation *animation, void *context){
    return;
}

static void time_anim_stopped_handler(Animation *animation, bool finished, void *context){
    TextLayer *target = (TextLayer *) context;

    if(target == s_TimeLayer.minutes){
        time_t temp = time(NULL);
        struct tm *tick_time = localtime(&temp);

        static char s_m_buffer[4];
        strftime(s_m_buffer, sizeof(s_m_buffer), "%M", tick_time);
        text_layer_set_text(target, s_m_buffer); 
    }

    else{
        time_t temp = time(NULL);
        struct tm *tick_time = localtime(&temp);

        static char s_h_buffer[4];
        strftime(s_h_buffer, sizeof(s_h_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
        text_layer_set_text(target, s_h_buffer); 
    } 
}

static void animate_time(TextLayer *target, int additional_delay){
    Animation **arr = (Animation**)malloc(2 * sizeof(Animation*));

    const int duration_ms = 300;
    const int delay_ms = 1500 + additional_delay;

    Layer *time_layer = text_layer_get_layer(target);

    GRect start = layer_get_frame(time_layer);
    GRect finish = layer_get_frame(time_layer);
    finish.origin.y = finish.origin.y + finish.size.h;
    finish.size.h = 0;

    PropertyAnimation *prop_anim = property_animation_create_layer_frame(time_layer, &start, &finish);
    Animation *anim = property_animation_get_animation(prop_anim);

    animation_set_curve(anim, AnimationCurveEaseIn);
    animation_set_delay(anim, delay_ms);
    animation_set_duration(anim, duration_ms);

    Animation *anim_1 = animation_clone(anim);
    animation_set_reverse(anim_1, true);
    animation_set_delay(anim_1, delay_ms + duration_ms + 120);

    animation_set_handlers(anim, (AnimationHandlers){
        .started = time_anim_started_handler,
        .stopped = time_anim_stopped_handler
    }, target);

    arr[0] = anim;
    arr[1] = anim_1;

    Animation *spawn = animation_sequence_create_from_array(arr, 2);
    animation_schedule(spawn);
    free(arr);
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

    text_layer_set_text(s_TimeLayer.hours, s_h_buffer);
    text_layer_set_text(s_TimeLayer.minutes, s_m_buffer);
}

static void update_with_anim(bool first_update){
    if(!first_update) animate_grid();
    animate_time(s_TimeLayer.minutes, 100);

    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_h_buffer[4];
    strftime(s_h_buffer, sizeof(s_h_buffer), "%H", tick_time);
    
    if(strcmp(s_h_buffer, text_layer_get_text(s_TimeLayer.hours))){
        animate_time(s_TimeLayer.hours, 0);
    }
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
    update_with_anim(false);
}

//LAYER CREATION
//-----------------------------------------------------------------------------
static void create_text_grid(Layer *target){
    int x_coord = 0;
    int y_coord = 0;
    s_textgrid_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IBM_REGULAR_21));

    for(int i=0; i<_NUM_ELEM_; i++){
        x_coord = 2 + (i % 10) * 14;
        if(i) y_coord = (i % 10)? y_coord : y_coord + 27;

        s_textgrid_elements[i] = text_layer_create(GRect(x_coord, y_coord, 13, 25));
        if(y_coord>55 && y_coord <107 && x_coord< 111) layer_set_hidden(text_layer_get_layer(s_textgrid_elements[i]), true);

        text_layer_set_font(s_textgrid_elements[i], s_textgrid_font);
        text_layer_set_background_color(s_textgrid_elements[i], GColorClear);
        text_layer_set_text_color(s_textgrid_elements[i], _TEXTGRID_COLOR_ );
        text_layer_set_text_alignment(s_textgrid_elements[i], GTextAlignmentLeft);
        layer_add_child(target, text_layer_get_layer(s_textgrid_elements[i])); 
    }

    shuffle_grid_text();
}

static void draw_timebox_canvas(Layer *layer, GContext *ctx){
    graphics_context_set_stroke_color(ctx, _ACCENT_BG_COLOR_);
    graphics_context_set_fill_color(ctx, _ACCENT_BG_COLOR_);    

    GRect rect_bounds = layer_get_bounds(layer);
    graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone); 
}

static void create_time(Layer *target){
    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IBM_BOLD_35));

    //TODO, find a procedural way to put the right coordinates. 
    s_TimeLayer.hours = text_layer_create(GRect(0, 59, 44, 48));
    s_TimeLayer.separator =text_layer_create(GRect(44, 59, 24, 48));
    s_TimeLayer.minutes = text_layer_create(GRect(68, 59, 44, 48));

    GRect time_box = GRect(0, 59, 112, 48);

    s_TimeLayer_bg = layer_create(time_box);
    layer_set_update_proc(s_TimeLayer_bg, draw_timebox_canvas);
    layer_add_child(target, s_TimeLayer_bg);
    layer_mark_dirty(s_TimeLayer_bg);

    text_layer_set_text(s_TimeLayer.hours, "aa");
    text_layer_set_font(s_TimeLayer.hours, s_time_font);
    text_layer_set_text_color(s_TimeLayer.hours, _TIME_COLOR_ );
    text_layer_set_background_color(s_TimeLayer.hours, GColorClear);
    text_layer_set_text_alignment(s_TimeLayer.hours, GTextAlignmentLeft);

    text_layer_set_font(s_TimeLayer.minutes, s_time_font);
    text_layer_set_text_color(s_TimeLayer.minutes, _TIME_COLOR_);
    text_layer_set_background_color(s_TimeLayer.minutes, GColorClear);
    text_layer_set_text_alignment(s_TimeLayer.minutes, GTextAlignmentLeft);

    text_layer_set_font(s_TimeLayer.separator, s_time_font);
    text_layer_set_text_color(s_TimeLayer.separator, _TIME_COLOR_ );
    text_layer_set_background_color(s_TimeLayer.separator, GColorClear);
    text_layer_set_text_alignment(s_TimeLayer.separator, GTextAlignmentLeft);
    text_layer_set_text(s_TimeLayer.separator, ":");

    layer_add_child(target, text_layer_get_layer(s_TimeLayer.hours));
    layer_add_child(target, text_layer_get_layer(s_TimeLayer.minutes));
    layer_add_child(target, text_layer_get_layer(s_TimeLayer.separator));
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    window_set_background_color(s_window, _BG_COLOR_ );

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
    srand(time(NULL));
    tick_timer_service_subscribe(MINUTE_UNIT, tick_minute_handler);

    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
    });
    const bool animated = true;
    window_stack_push(s_window, animated);

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
