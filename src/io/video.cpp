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

#include "io/video.h"
#include "io/dksfile.h"
#include "util/wutil.h"
#include <SDL.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

struct video_state_t video_state = { NULL, NULL, NULL, NULL, 0 };

struct naytto ruutu;

int current_mode = VGA_MODE;
unsigned char *vircr;
int update_vircr_mode = 1;
int draw_with_vircr_mode = 1;
int pixel_multiplier_vga = 1, pixel_multiplier_svga = 1;
int wantfullscreen = 1;

SDL_Color curpal[256];

/**
 * Sets palette entries firstcolor to firstcolor+n-1
 * from pal[0] to pal[n-1].
 * @param pal the palette, specify NULL to set all colors to black=(0,0,0)
 * @param reverse = 1 to read colors in reverse order (pal[n-1] to pal[0])
 */
void setpal_range(const char pal[][3], int firstcolor, int n, int reverse) {
    SDL_Color *cc = (SDL_Color *) walloc(n * sizeof(SDL_Color));
    int i, from = (reverse ? n - 1 : 0);

    for (i = 0; i < n; i++) {
        if (pal == NULL) {
            cc[i].r = cc[i].g = cc[i].b = 0;
        } else {
            cc[i].r = 4 * pal[from][0];
            cc[i].g = 4 * pal[from][1];
            cc[i].b = 4 * pal[from][2];
        }
        cc[i].a = 255;
        if (reverse)
            from--;
        else
            from++;
    }

    memcpy(&curpal[firstcolor], cc, n * sizeof(SDL_Color));
    wfree(cc);

    if (n != 8)             // FIXME hack to ignore rotate_water_palet
        all_bitmaps_refresh();
}

void fillrect(int x, int y, int w, int h, int c) {
    SDL_Rect r;

    if (update_vircr_mode) {
        int screenw = (current_mode == VGA_MODE) ? 320 : 800;
        if (w == 1 && h == 1) {
            vircr[x + y * screenw] = c;
        } else {
            int i;
            for (i = 0; i < h; i++)
                memset(&vircr[x + (y + i) * screenw], c, w);
        }
    }

    if (draw_with_vircr_mode)
        return;

    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    SDL_SetRenderDrawColor(video_state.renderer,
                           curpal[c].r,
                           curpal[c].g,
                           curpal[c].b,
                           curpal[c].a);
    SDL_RenderFillRect(video_state.renderer, &r);
}

void do_all(int do_retrace) {
    if (draw_with_vircr_mode) {
        int w = (current_mode == VGA_MODE) ? 320 : 800;
        int wh = (current_mode == VGA_MODE) ? 320 * 200 : 800 * 600;
        int i;
        uint8_t *in = vircr;
        uint32_t *out = video_state.texture_buffer;

        for (i = 0; i < wh; i++, in++, out++)
            *out = ((255 << 24) |
                    (curpal[*in].r << 16) |
                    (curpal[*in].g << 8) |
                    curpal[*in].b);

        SDL_UpdateTexture(video_state.texture,
                          NULL,
                          video_state.texture_buffer,
                          w * sizeof(uint32_t));
        SDL_SetRenderDrawColor(video_state.renderer, 0, 0, 0, 255);
        SDL_RenderClear(video_state.renderer);
        SDL_RenderCopy(video_state.renderer, video_state.texture, NULL, NULL);
    }
    SDL_RenderPresent(video_state.renderer);
    if (!draw_with_vircr_mode) {
        // Prepare for drawing the next frame
        SDL_SetRenderDrawColor(video_state.renderer, 0, 0, 0, 255);
        SDL_RenderClear(video_state.renderer);
    }
}

static void sigint_handler(int dummy) {
    _exit(1);
}

void init_video(void) {
    int ret;

    if (!video_state.init_done) {
        ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE);
        if (ret) {
            fprintf(stderr, "SDL_Init failed with %d. Is your DISPLAY environment variable set?\n", ret);
            exit(1);
        }
        signal(SIGINT, sigint_handler);
        atexit(SDL_Quit);
        video_state.init_done = 1;
        video_state.window = SDL_CreateWindow("Triplane Classic",
                                              SDL_WINDOWPOS_UNDEFINED,
                                              SDL_WINDOWPOS_UNDEFINED,
                                              pixel_multiplier_vga * 320,
                                              pixel_multiplier_vga * 200,
                                              (wantfullscreen
                                               ? SDL_WINDOW_FULLSCREEN_DESKTOP
                                               : SDL_WINDOW_RESIZABLE));
        assert(video_state.window);
        video_state.renderer = SDL_CreateRenderer(video_state.window, -1, 0);
        assert(video_state.renderer);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        SDL_ShowCursor(SDL_DISABLE);
        vircr = (unsigned char *) walloc(800 * 600);
    }
}

static int init_mode(int new_mode, const char *paletname) {
    int las, las2;
    int w = (new_mode == SVGA_MODE) ? 800 : 320;
    int h = (new_mode == SVGA_MODE) ? 600 : 200;

    init_video();

    // FIXME resize the window in non-fullscreen mode? (what about user resizes?)

    if (video_state.texture != NULL) {
        SDL_DestroyTexture(video_state.texture);
        video_state.texture = NULL;
    }
    if (video_state.texture_buffer != NULL) {
        wfree(video_state.texture_buffer);
        video_state.texture_buffer = NULL;
    }

    SDL_RenderSetLogicalSize(video_state.renderer, w, h);

    if (draw_with_vircr_mode) {
        video_state.texture = SDL_CreateTexture(video_state.renderer,
                                                SDL_PIXELFORMAT_ARGB8888,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                w,
                                                h);
        assert(video_state.texture);
        video_state.texture_buffer = (uint32_t *) walloc(w * h * sizeof(uint32_t));
    }

    dksopen(paletname);

    dksread(ruutu.normaalipaletti, sizeof(ruutu.normaalipaletti));
    for (las = 0; las < 256; las++)
        for (las2 = 0; las2 < 3; las2++)
            ruutu.paletti[las][las2] = ruutu.normaalipaletti[las][las2];

    dksclose();

    setpal_range(ruutu.paletti, 0, 256);

    current_mode = new_mode;
    return 1;
}

int init_vesa(const char *paletname) {
    return init_mode(SVGA_MODE, paletname);
}

void init_vga(const char *paletname) {
    init_mode(VGA_MODE, paletname);
}
