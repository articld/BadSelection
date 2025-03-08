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

static TextLayer *s_time_layer;
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

static void update_time(bool first_update){
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

    text_layer_set_text(s_time_layer, s_buffer);

    //eventually this line will go to the grid animation, whenever I'm done implementing it
    //if(!first_update) shuffle_grid_text();
    if(!first_update) animate_grid();
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

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
    update_time(false);
}

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

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    window_set_background_color(s_window, _BG_COLOR_ );

    create_text_grid(window_layer);

    s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IBM_BOLD_35));

    s_time_layer = text_layer_create(GRect(0, 59, 112, 48));

    text_layer_set_font(s_time_layer, s_time_font);
    text_layer_set_text_color(s_time_layer, _TIME_COLOR_ );
    text_layer_set_background_color(s_time_layer, _ACCENT_BG_COLOR_ );
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    for(int i=0; i<_NUM_ELEM_; i++) 
        text_layer_destroy(s_textgrid_elements[i]);

    text_layer_destroy(s_time_layer);
    fonts_unload_custom_font(s_time_font);
    fonts_unload_custom_font(s_textgrid_font);
    //fonts_unload_custom_font(s_date_font);
}

static void init(void) {
    srand(time(NULL));
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    s_window = window_create();

    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
    });
    const bool animated = true;
    window_stack_push(s_window, animated);

    update_time(true);
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
