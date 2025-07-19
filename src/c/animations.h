#ifndef ANIMATIONS_H_
#define ANIMATIONS_H_

extern int s_animation_counter;
extern int s_date_animation_counter;

//probably not the best idea
extern TextLayer *time_hours_pointer;

void anim_started_handler(Animation *animation, void *context);

void anim_stopped_handler(Animation *animation, bool finished, void *context);

void anim_1_started_handler(Animation *animation, void *context);

void anim_1_stopped_handler(Animation *animation, bool finished, void *context);

void animate_grid(TextLayer **s_textgrid_elements);

void time_anim_started_handler(Animation *animation, void *context);

void time_anim_stopped_handler(Animation *animation, bool finished, void *context);

void select_animate_time(TextLayer *time_minutes, TextLayer *time_hours, bool is_textgrid_animated);

void animate_time(TextLayer *target, int additional_delay, bool is_textgrid_animated);

void animate_date(TextLayer **s_date_elements_to_anim);

void date_anim_started_handler(Animation *animation, void *context);

void date_anim_stopped_handler(Animation *animation, bool finished, void *context);

void date_anim_1_started_handler(Animation *animation, void *context);

void date_anim_1_stopped_handler(Animation *animation, bool finished, void *context);

#endif