#include <pebble.h>
#include "animations.h"

//this is not good, but I'll fix it later
#define _NUM_ELEM_ 60

int s_animation_counter = 0;

TextLayer *time_hours_pointer = NULL;

//TEXTGRID ANIMATION
//-----------------------------------------------------------------------------

void anim_started_handler(Animation *animation, void *context) {
    return;
}

void anim_stopped_handler(Animation *animation, bool finished, void *context) {
    static char s_str[_NUM_ELEM_][2] = {{'a', '\0'}};
    s_str[s_animation_counter][0] = rand() % 26 + 97;

    text_layer_set_text(context, s_str[s_animation_counter]);

    s_animation_counter = (s_animation_counter + 1) % 60;

    if(!finished){
        animation_destroy(animation);
    }
}

void anim_1_started_handler(Animation *animation, void *context) {
    return;
}

void anim_1_stopped_handler(Animation *animation, bool finished, void *context) {
    return;
}

void animate_grid(TextLayer **s_textgrid_elements){
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

void time_anim_started_handler(Animation *animation, void *context){
    return;
}

void time_anim_stopped_handler(Animation *animation, bool finished, void *context){
    TextLayer *target = (TextLayer *) context;

    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    if(time_hours_pointer != context){
        static char s_m_buffer[4];
        strftime(s_m_buffer, sizeof(s_m_buffer), "%M", tick_time);
        text_layer_set_text(target, s_m_buffer); 
    }

    else{
        static char s_h_buffer[4];
        strftime(s_h_buffer, sizeof(s_h_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);
        text_layer_set_text(target, s_h_buffer); 
    }
    
    if(!finished){
        animation_destroy(animation);
    }
}

void select_animate_time(TextLayer *time_minutes, TextLayer *time_hours){
    time_hours_pointer = time_hours;

    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    animate_time(time_minutes, 100);

    static char s_h_buffer[4];
    strftime(s_h_buffer, sizeof(s_h_buffer), "%H", tick_time);
    
    if(strcmp(s_h_buffer, text_layer_get_text(time_hours))){
        animate_time(time_hours, 0);
    }
}

void animate_time(TextLayer *target, int additional_delay){
    Animation **arr = (Animation**)malloc(2 * sizeof(Animation*));

    const int duration_ms = 300;
    const int delay_ms = (75 * _NUM_ELEM_ / 2) + additional_delay;

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

    Animation *spawn = animation_spawn_create_from_array(arr, 2);
    animation_schedule(spawn);
    free(arr);
}
