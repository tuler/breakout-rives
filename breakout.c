// Header including all RIV APIs
#include <riv.h>

// Game state
bool started; // true when game has started
bool ended;   // true when game has ended

enum
{
    BALL_SIZE = 2,
    PADDLE_HEIGHT = 2,
    PADDLE_Y = 120,
    BRICK_HEIGHT = 4,
    BRICKS_PER_ROW = 8,
    BRICKS_ROWS = 8,
    MOVEMENT_SPEED = 5,
    NUM_BRICKS = BRICKS_PER_ROW * BRICKS_ROWS
};

struct brick
{
    riv_recti pos;
    bool active;
    riv_color_id color;
};

int score;
riv_vec2i ball_pos;
riv_vec2f ball_velocity;
int64_t paddle_pos;
int64_t paddle_width;
struct brick bricks[NUM_BRICKS];

// Called when game starts
void start_game()
{
    riv_printf("GAME START\n");
    started = true;

    // initialize score
    score = 0;

    // initialize paddle
    paddle_pos = riv->width / 2;
    paddle_width = 20;

    // initialize ball
    ball_pos = (riv_vec2i){paddle_pos, PADDLE_Y - BALL_SIZE};
    ball_velocity = (riv_vec2f){5, -5};

    // initialize bricks
    int64_t brick_width = riv->width / BRICKS_PER_ROW;
    for (int y = 0; y < BRICKS_ROWS; y++)
    {
        for (int x = 0; x < BRICKS_PER_ROW; x++)
        {
            bricks[y * BRICKS_PER_ROW + x] = (struct brick){
                (riv_recti){x * brick_width, y * BRICK_HEIGHT, brick_width, BRICK_HEIGHT},
                true,
                RIV_COLOR_RED + y};
        }
    }
}

// Called when game ends
void end_game()
{
    riv_printf("GAME OVER\n");
    ended = true;
    // Quit in 3 seconds
    riv->quit_frame = riv->frame + 3 * riv->target_fps;
}

// Update game logic
void update_game()
{
    if (riv->keys[RIV_GAMEPAD_LEFT].down)
    {
        // move paddle to the left
        paddle_pos -= MOVEMENT_SPEED;
        paddle_pos = paddle_pos < 0 ? 0 : paddle_pos;
    }
    else if (riv->keys[RIV_GAMEPAD_RIGHT].down)
    {
        // move paddle to the right
        paddle_pos += MOVEMENT_SPEED;
        paddle_pos = (paddle_pos + paddle_width) > riv->width ? riv->width - paddle_width : paddle_pos;
    }

    // move ball
    ball_pos.x += ball_velocity.x;
    ball_pos.y += ball_velocity.y;

    // bounce ball off walls
    if (ball_pos.x - BALL_SIZE < 0 || ball_pos.x + BALL_SIZE > riv->width)
    {
        ball_velocity.x = -ball_velocity.x;
    }

    // end_game();
}

// Draw the game map
void draw_game()
{
    // draw paddle
    riv_draw_rect_fill(paddle_pos, PADDLE_Y, paddle_width, PADDLE_HEIGHT, RIV_COLOR_LIGHTGREEN);

    // draw ball
    riv_draw_circle_fill(ball_pos.x, ball_pos.y, BALL_SIZE, RIV_COLOR_ORANGE);

    // draw bricks
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        if (bricks[i].active)
        {
            riv_draw_rect_fill(bricks[i].pos.x + 1, bricks[i].pos.y + 1, bricks[i].pos.width - 1, bricks[i].pos.height - 1, bricks[i].color);
        }
    }
}

// Draw game start screen
void draw_start_screen()
{
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
    riv_draw_text("PRESS TO START", RIV_SPRITESHEET_FONT_5X7, RIV_CENTER, 64, 64 + 16, 1, col);
}

// Draw game over screen
void draw_end_screen()
{
    // Draw last game frame
    draw_game();
    // Draw GAME OVER
    riv_draw_text("GAME OVER", RIV_SPRITESHEET_FONT_5X7, RIV_CENTER, 64, 64, 2, RIV_COLOR_RED);
}

// Called every frame to update game state
void update()
{
    if (!started)
    { // Game not started yet
        // Let game start whenever a key has been pressed
        if (riv->key_toggle_count > 0)
        {
            start_game();
        }
    }
    else if (!ended)
    { // Game is progressing
        update_game();
    }
}

// Called every frame to draw the game
void draw()
{
    // Clear screen
    riv_clear(RIV_COLOR_DARKSLATE);
    // Draw different screens depending on the game state
    if (!started)
    { // Game not started yet
        draw_start_screen();
    }
    else if (!ended)
    { // Game is progressing
        draw_game();
    }
    else
    { // Game ended
        draw_end_screen();
    }
}

// Entry point
int main()
{
    // Main loop, keep presenting frames until user quit or game ends
    riv->width = 128;
    riv->height = 128;
    riv->target_fps = 8;
    do
    {
        // Update game state
        update();
        // Draw game graphics
        draw();
    } while (riv_present());
    return 0;
}
