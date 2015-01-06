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

/*******************************************************************************

   Purpose: 
   	Mouse handling part of Wsystem 2.0 for DJGPP v.2.0á5

*******************************************************************************/

#include "io/mouse.h"
#include "io/video.h"
#include <SDL.h>

/*
 * SDL_RenderSetLogicalSize does not expose functions to calculate its
 * mapping, and both SDL_WarpMouseInWindow and SDL_GetMouseState work
 * with physical coordinates. We thus need to convert them manually.
 * Perhaps one day SDL will have equivalent functions that we can use
 * instead of these...
 */
static void logical_to_physical(int logx, int logy, int *physx, int *physy) {
    float scaleX, scaleY;
    SDL_Rect vp;

    SDL_RenderGetScale(video_state.renderer, &scaleX, &scaleY);
    SDL_RenderGetViewport(video_state.renderer, &vp);

    *physx = floor(vp.x * scaleX) + floor(logx * scaleX);
    *physy = floor(vp.y * scaleY) + floor(logy * scaleY);
}

static void physical_to_logical(int physx, int physy, int *logx, int *logy) {
    float scaleX, scaleY;
    SDL_Rect vp;

    SDL_RenderGetScale(video_state.renderer, &scaleX, &scaleY);
    SDL_RenderGetViewport(video_state.renderer, &vp);

    if (physx / scaleX <= vp.x)
      *logx = 0;
    else if (physx / scaleX >= vp.x + vp.w)
      *logx = vp.w - 1;
    else
      *logx = ceil((physx - floor(vp.x * scaleX)) / scaleX);

    if (physy / scaleY <= vp.y)
      *logy = 0;
    else if (physy / scaleY >= vp.y + vp.h)
      *logy = vp.h - 1;
    else
      *logy = ceil((physy - floor(vp.y * scaleY)) / scaleY);
}

void hiiri_to(int x, int y) {
    int px, py;
    logical_to_physical(x, y, &px, &py);
    SDL_WarpMouseInWindow(video_state.window, px, py);
}

void koords(int *x, int *y, int *n1, int *n2) {
    Uint8 ret;
    int px, py;

    SDL_PumpEvents();
    ret = SDL_GetMouseState(&px, &py);
    *n1 = !!(ret & SDL_BUTTON(1));
    *n2 = !!(ret & SDL_BUTTON(3));

    physical_to_logical(px, py, x, y);
}
