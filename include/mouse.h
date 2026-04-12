#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"

/* Mouse state */
typedef struct {
    int      x;
    int      y;
    bool     left_button;
    bool     right_button;
    bool     middle_button;
    int      dx;    /* Delta X since last read */
    int      dy;    /* Delta Y since last read */
} mouse_state_t;

/* Mouse event callback */
typedef void (*mouse_callback_t)(mouse_state_t* state);

/* Initialize PS/2 mouse */
void mouse_init(void);

/* Set screen bounds for cursor clamping */
void mouse_set_bounds(int width, int height);

/* Get current mouse state */
mouse_state_t mouse_get_state(void);

/* Set mouse event callback */
void mouse_set_callback(mouse_callback_t cb);

/* Called from IRQ12 handler */
void mouse_irq_handler(void);

#endif
