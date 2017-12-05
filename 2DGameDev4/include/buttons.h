#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#include "window.h"

void button_return_to_main_menu(Button *self);

void button_exit(Button *self);

void button_toggle_pause(Button *self);

void button_restart_level(Button *self);

void button_start_level(Button *self);

void button_start_map_editor(Button *self);

#endif