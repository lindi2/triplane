/* 
 * Triplane Classic - a side-scrolling dogfighting game.
 * Copyright (C) 1996,1997,2009  Dodekaedron Software Creations Oy
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * tjt@users.sourceforge.net
 */

/* Support functions for menus */

#include "menus/menusupport.h"
#include "io/sdl_compat.h"
#include "io/mouse.h"
#include <SDL.h>

static int menu_mousetab = 0, menu_n1 = 0, menu_n2 = 0;

void menu_keys(int *exit_flag, int *help_flag) {
    int ch;

    while (kbhit()) {
        ch = getch();
        if (ch == SDLK_F1) {
            if (help_flag != NULL)
                *help_flag = !*help_flag;
        } else if (ch == SDLK_ESCAPE) {
            if (exit_flag != NULL)
                *exit_flag = 1;
        } else if (ch == SDLK_SPACE) {
            menu_n1 = 1;
        } else if (ch == SDLK_RETURN) {
            menu_n2 = 1;
        } else if (ch == SDLK_TAB) {
            menu_mousetab++;
        }
    }
}

/*
 * positions is an array, ordered first by y then by x, ending with an
 * entry with active=-1; or NULL if none
 */
void menu_mouse(int *x, int *y, int *n1, int *n2,
               const menu_position *positions) {
    const menu_position *p;

    koords(x, y, n1, n2);

    if (positions == NULL)
        menu_mousetab = 0;
    if (menu_mousetab > 0) {
        for (p = positions; p->active >= 0; p++) {
            if (p->active == 0 ||
                p->y < *y ||
                (p->y == *y && p->x <= *x)) {
                continue;
            }
            if (menu_mousetab == 1)
                break;
            menu_mousetab--;
        }

        if (p->active < 0) { // no next found, so find first active
            for (p = positions; p->active == 0; p++)
                ;
        }

        if (p->active == 1) {
            *x = p->x;
            *y = p->y;
            hiiri_to(*x, *y);
        }

        menu_mousetab = 0;
    }

    if (menu_n1 == 1) {
        *n1 = 1;
        menu_n1 = 0;
    }
    if (menu_n2 == 1) {
        *n2 = 1;
        menu_n2 = 0;
    }
}

void wait_mouse_relase(int nokb) {
    int n1 = 1, n2 = 1, x, y;

    while (n1 || n2) {
        koords(&x, &y, &n1, &n2);

        if (!nokb)
            while (kbhit())
                getch();
    }
}

void wait_press_and_release(void) {
    int x, y, n1, n2;

    n1 = 0;
    while (!n1) {
        if (kbhit())
            break;
        koords(&x, &y, &n1, &n2);
    }

    wait_mouse_relase(0);
}
