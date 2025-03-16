#ifndef ANIMATIONS_H_
#define ANIMATIONS_H_

extern int s_animation_counter;

void anim_started_handler(Animation *animation, void *context);

void anim_stopped_handler(Animation *animation, bool finished, void *context);

void anim_1_started_handler(Animation *animation, void *context);

void anim_1_stopped_handler(Animation *animation, bool finished, void *context);

void animate_grid(TextLayer **s_textgrid_elements);

void time_anim_started_handler(Animation *animation, void *context);

void time_anim_stopped_handler(Animation *animation, bool finished, void *context);

void animate_time(TextLayer *target, int additional_delay);
#endif