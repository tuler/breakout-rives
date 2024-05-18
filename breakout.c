// Header including all RIV APIs
#include <riv.h>

// Game state
bool started; // true when game has started
bool ended; // true when game has ended

riv_vec2i ball_position;
riv_vec2f ball_velocity;
int64_t paddle_pos;
int64_t paddle_width;

enum {
    PADDLE_HEIGHT = 2,
    BRICK_SIZE = 8,
};

// Called when game starts
void start_game() {
    riv_printf("GAME START\n");
    started = true;
    paddle_pos = riv->width / 2;
    paddle_width = 20;
}

// Called when game ends
void end_game() {
    riv_printf("GAME OVER\n");
    ended = true;
    // Quit in 3 seconds
    riv->quit_frame = riv->frame + 3*riv->target_fps;
}

// Update game logic
void update_game() {
    // TODO: update game
    end_game();
}

// Draw the game map
void draw_game() {
    // TODO: draw game
    riv_draw_rect_fill(paddle_pos - paddle_width/2, 120, paddle_width, PADDLE_HEIGHT, RIV_COLOR_LIGHTGREEN);
}

// Draw game start screen
void draw_start_screen() {
    // Draw snake title
    riv_draw_text(
        "breakout",               // text to draw
        RIV_SPRITESHEET_FONT_5X7, // sprite sheet id of the font
        RIV_CENTER,               // anchor point on the text bounding box
        64,                       // anchor x
        64,                       // anchor y
        2,                        // text size multiplier
        RIV_COLOR_LIGHTGREEN      // text color
    );
    // Make "press to start blink" by changing the color depending on the frame number
    uint32_t col = (riv->frame % 2 == 0) ? RIV_COLOR_LIGHTRED : RIV_COLOR_DARKRED;
    // Draw press to start
    riv_draw_text("PRESS TO START", RIV_SPRITESHEET_FONT_5X7, RIV_CENTER, 64, 64+16, 1, col);
}

// Draw game over screen
void draw_end_screen() {
    // Draw last game frame
    draw_game();
    // Draw GAME OVER
    riv_draw_text("GAME OVER", RIV_SPRITESHEET_FONT_5X7, RIV_CENTER, 64, 64, 2, RIV_COLOR_RED);
}

// Called every frame to update game state
void update() {
    if (!started) { // Game not started yet
        // Let game start whenever a key has been pressed
        if (riv->key_toggle_count > 0) {
            start_game();
        }
    } else if (!ended) { // Game is progressing
        update_game();
    }
}

/*
void draw_ball() {
    riv_draw_rect(
        ball_position.x, ball_position.y, 4, 4, RIV_COLOR_LIGHTGREEN
    );
}

void draw_brick(riv_recti *brick) {
    if (brick->active) {
        riv_draw_rect(
            brick->x, brick->y, brick->width, brick->height, RIV_COLOR_LIGHTGREEN
        );
    }
}

void draw_bricks(struct Brick bricks[], int count) {
    for (int i = 0; i < count; i++) {
        draw_brick(&bricks[i]);
    }
}
*/

// Called every frame to draw the game
void draw() {
    // Clear screen
    riv_clear(RIV_COLOR_DARKSLATE);
    // Draw different screens depending on the game state
    if (!started) { // Game not started yet
        draw_start_screen();
    } else if (!ended) { // Game is progressing
        draw_game();
    } else { // Game ended
        draw_end_screen();
    }
}

// Entry point
int main() {
    // Main loop, keep presenting frames until user quit or game ends
    riv->width = 128;
    riv->height = 128;
    riv->target_fps = 8;
    do {
        // Update game state
        update();
        // Draw game graphics
        draw();
    } while(riv_present());
    return 0;
}
