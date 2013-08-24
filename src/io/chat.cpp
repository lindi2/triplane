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
   	Writing and sending chat messages in networked game

*******************************************************************************/

#include "triplane.h"
#include "gfx/gfx.h"
#include "gfx/font.h"
#include "io/network.h"
#include "util/wutil.h"

// Maximum length of a chat message in the line editor, not including
// the terminating NUL. Must be smaller than length of
// NET_PKTTYPE_C_CHATMSG msg.
#define EDITLINELEN 70

static const char chat_f6_message[] = "Help!";
static const char chat_f7_message[] = "Yes, ok";
static const char chat_f8_message[] = "No";

static Font *chat_font = NULL;
static void (*chat_sender)(const char *) = NULL;
static int chat_overlay_on = 0;
static char chat_text[EDITLINELEN + 1] = "";
static int chat_textpos = 0;
static uint32_t chat_last_message_time = 0;

static void chat_send(const char *msg) {
    if (SDL_GetTicks() < chat_last_message_time + 1000)
        return;                 // we are chatting too fast, ignore

    chat_last_message_time = SDL_GetTicks();

    if (chat_sender != NULL)
        chat_sender(msg);
}

static void chat_erase_text(void) {
    chat_text[0] = '\0';
    chat_textpos = 0;
}

static void chat_overlay_start(void) {
    chat_overlay_on = 1;
}

static void chat_overlay_stop(void) {
    chat_overlay_on = 0;
}

void chat_overlay_init(Font *font) {
    chat_font = font;
}

void chat_set_sender(void (*new_sender)(const char *)) {
    chat_sender = new_sender;
}

void chat_draw_overlay(void) {
    int display_was_enabled;

    if (!chat_overlay_on)
        return;

    if (chat_sender == NULL) {
        chat_overlay_stop();
        return;
    }

    // don't display this to the clients, if we are the server
    display_was_enabled = network_display_enable(0);

    // FIXME draw a bitmap for the background (load in chat_overlay_init())
    fill_vircr(20, 50, 300, 80, 14);
    chat_font->printf(30, 55, "Message for chat (Enter=done, Esc=exit):");
    chat_font->printf(30, 65, "%s_", chat_text);

    network_display_enable(display_was_enabled);
}

static int chat_edit_key(int keysym) {
    switch (keysym) {
    case SDLK_RETURN:
        chat_overlay_stop();
        if (chat_text[0] != '\0')
            chat_send(chat_text);
        chat_erase_text();
        return 1;
    case SDLK_ESCAPE:
        chat_overlay_stop();
        chat_erase_text();
        return 1;
    case SDLK_F5:
        chat_overlay_stop();
        return 1;
    case SDLK_BACKSPACE:
        if (chat_textpos > 0) {
            chat_text[--chat_textpos] = '\0';
        }
        return 1;
    default:
        if (printable_char(keysym)) {
            if (chat_textpos < EDITLINELEN) {
                chat_text[chat_textpos++] = keysym;
                chat_text[chat_textpos] = '\0';
            }
            return 1;
        }
    }

    return 0;
}

// returns 1 if keysym was consumed by the chat code
int chat_input_key(int keysym) {
    if (chat_sender == NULL)
        return 0;

    if (chat_overlay_on) {
        return chat_edit_key(keysym);
    } else {
        if (keysym == SDLK_F5) {
            chat_overlay_start();
            return 1;
        } else if (keysym == SDLK_F6) {
            chat_send(chat_f6_message);
            return 1;
        } else if (keysym == SDLK_F7) {
            chat_send(chat_f7_message);
            return 1;
        } else if (keysym == SDLK_F8) {
            chat_send(chat_f8_message);
            return 1;
        }
    }
    return 0;
}

int chat_overlay_is_on(void) {
    return chat_overlay_on;
}
