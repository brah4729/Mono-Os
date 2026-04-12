#ifndef OKI_H
#define OKI_H

#include "types.h"
#include "framebuffer.h"
#include "mouse.h"

/* ─── Oki Desktop Environment ──────────────────────────── */

/* Window flags */
#define OKI_WIN_VISIBLE     0x01
#define OKI_WIN_FOCUSED     0x02
#define OKI_WIN_DECORATED   0x04   /* Has titlebar */
#define OKI_WIN_RESIZABLE   0x08
#define OKI_WIN_MOVABLE     0x10
#define OKI_WIN_FULLSCREEN  0x20

/* Desktop configuration */
#define OKI_MAX_WINDOWS     32
#define OKI_PANEL_HEIGHT    32
#define OKI_TITLEBAR_HEIGHT 28
#define OKI_BORDER_WIDTH    2
#define OKI_GAP             6
#define OKI_CORNER_RADIUS   8

/* Workspace */
#define OKI_MAX_WORKSPACES  4

/* Window structure */
typedef struct oki_window {
    int         id;
    char        title[64];
    int         x, y, w, h;        /* Position and size */
    uint32_t    flags;
    int         workspace;          /* Which workspace (0-3) */

    /* Client framebuffer — the window's content */
    uint32_t*   surface;
    int         surface_w, surface_h;

    /* Decoration colors */
    color_t     border_color;
    color_t     titlebar_color;
    color_t     title_text_color;

    /* State */
    bool        dirty;               /* Needs redraw */
    bool        dragging;
    int         drag_offset_x;
    int         drag_offset_y;
} oki_window_t;

/* Desktop state */
typedef struct {
    oki_window_t*   windows[OKI_MAX_WINDOWS];
    int             window_count;
    int             focused_window;   /* Index of focused window, -1 if none */
    int             current_workspace;
    int             screen_w, screen_h;

    /* Panel */
    bool            panel_visible;

    /* Desktop wallpaper color (gradient) */
    color_t         wallpaper_top;
    color_t         wallpaper_bottom;

    /* State */
    bool            running;
    bool            needs_redraw;
} oki_desktop_t;

/* ─── API ──────────────────────────────────────────────── */

/* Initialize Oki Desktop Environment */
void oki_init(void);

/* Start the desktop event loop */
void oki_run(void);

/* Window management */
oki_window_t* oki_create_window(const char* title, int x, int y, int w, int h, uint32_t flags);
void oki_destroy_window(oki_window_t* win);
void oki_focus_window(oki_window_t* win);
void oki_move_window(oki_window_t* win, int x, int y);
void oki_resize_window(oki_window_t* win, int w, int h);

/* Drawing into a window's surface */
void oki_window_clear(oki_window_t* win, color_t color);
void oki_window_putchar(oki_window_t* win, int x, int y, char c, color_t fg);
void oki_window_puts(oki_window_t* win, int x, int y, const char* str, color_t fg);
void oki_window_fill_rect(oki_window_t* win, int x, int y, int w, int h, color_t color);

/* Compositor */
void oki_compose(void);

/* Workspace switching */
void oki_switch_workspace(int ws);

/* Desktop callback — process mouse/keyboard events */
void oki_handle_mouse(mouse_state_t* state);
void oki_handle_key(uint8_t scancode);

#endif
