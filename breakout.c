// Header including all RIV APIs
#include <riv.h>

// Game state
bool started; // true when game has started
bool ended;   // true when game has ended

enum
{
    BALL_SIZE = 4,
    BRICK_HEIGHT = 8,
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
riv_recti paddle;
struct brick bricks[NUM_BRICKS];

// sound configuration
riv_waveform_desc break_sfx = {
    .type = RIV_WAVEFORM_NOISE,
    .attack = 0.025,
    .decay = 0.075,
    .sustain = 0.075,
    .release = 0.025,
    .start_frequency = 250,
    .end_frequency = 100,
    .amplitude = 0.25,
    .sustain_level = 0.1,
    .duty_cycle = 0.5,
    .pan = 0.0,
};

riv_waveform_desc bounce_sfx = {
    .type = RIV_WAVEFORM_TRIANGLE,
    .attack = 0.025,
    .decay = 0.075,
    .sustain = 0.125,
    .release = 0.120,
    .start_frequency = 250,
    .end_frequency = 250,
    .amplitude = 0.25,
    .sustain_level = 0.3,
    .duty_cycle = 0.5,
    .pan = 0.0,
};

riv_waveform_desc gameover_sfx = {
    .type = RIV_WAVEFORM_PULSE,
    .attack = 0.01,
    .decay = 0.150,
    .sustain = 0.1,
    .release = 0.1,
    .start_frequency = 110,
    .end_frequency = 22,
    .amplitude = 0.2,
    .sustain_level = 0.5,
    .duty_cycle = 0.5,
    .pan = 0.0,
};

riv_waveform_desc victory_sfx = {
    .type = RIV_WAVEFORM_PULSE,
    .attack = 0.01,
    .decay = 0.2,
    .sustain = 0.3,
    .release = 0.2,
    .start_frequency = 55,
    .end_frequency = 1760,
    .amplitude = 0.2,
    .sustain_level = 0.25,
    .duty_cycle = 0.125,
    .pan = 0.0,
};

// Called when game starts
void start_game()
{
    riv_printf("GAME START\n");
    started = true;

    // initialize score
    score = 0;

    // initialize paddle
    paddle = (riv_recti){riv->width / 2, riv->height - 20, 40, 4};

    // initialize ball
    ball_pos = (riv_vec2i){paddle.x, paddle.y - BALL_SIZE};
    ball_velocity = (riv_vec2f){1, -1};

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
        paddle.x -= MOVEMENT_SPEED;
        paddle.x = paddle.x < 0 ? 0 : paddle.x;
    }
    else if (riv->keys[RIV_GAMEPAD_RIGHT].down)
    {
        // move paddle to the right
        paddle.x += MOVEMENT_SPEED;
        paddle.x = (paddle.x + paddle.width) > riv->width ? riv->width - paddle.width : paddle.x;
    }

    // move ball
    ball_pos.x += ball_velocity.x;
    ball_pos.y += ball_velocity.y;

    // bounce ball off walls
    if (ball_pos.x - BALL_SIZE < 0 || ball_pos.x + BALL_SIZE > riv->width)
    {
        riv_waveform(&bounce_sfx);
        ball_velocity.x = -ball_velocity.x;
    }

    // bounce ball off ceiling
    if (ball_pos.y - BALL_SIZE < 0)
    {
        riv_waveform(&bounce_sfx);
        ball_velocity.y = -ball_velocity.y;
    }

    // bounce ball off paddle
    if (ball_pos.y + BALL_SIZE > paddle.y && ball_pos.y - BALL_SIZE < paddle.y + paddle.height)
    {
        if (ball_pos.x + BALL_SIZE > paddle.x && ball_pos.x - BALL_SIZE < paddle.x + paddle.width)
        {
            riv_waveform(&bounce_sfx);
            ball_velocity.y = -ball_velocity.y;
        }
    }

    // break bricks
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        if (bricks[i].active)
        {
            if (ball_pos.x + BALL_SIZE > bricks[i].pos.x && ball_pos.x - BALL_SIZE < bricks[i].pos.x + bricks[i].pos.width &&
                ball_pos.y + BALL_SIZE > bricks[i].pos.y && ball_pos.y - BALL_SIZE < bricks[i].pos.y + bricks[i].pos.height)
            {
                // break the brick
                bricks[i].active = false;

                // bounce the ball
                ball_velocity.y = -ball_velocity.y;

                // increment score
                score++;
                riv->outcard_len = riv_snprintf((char *)riv->outcard, RIV_SIZE_OUTCARD, "JSON{\"score\":%d}", score);

                // Play the break sound
                riv_waveform(&break_sfx);

                // break only one brick per frame
                break;
            }
        }
    }

    // game over if all bricks are broken
    if (score == NUM_BRICKS)
    {
        riv_waveform(&victory_sfx);
        end_game();
    }

    // game over if ball falls off the screen
    if (ball_pos.y > riv->height)
    {
        riv_waveform(&gameover_sfx);
        end_game();
    }
}

// Draw the game map
void draw_game()
{
    // draw paddle
    riv_draw_rect_fill(paddle.x, paddle.y, paddle.width, paddle.height, RIV_COLOR_LIGHTGREEN);

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

    // draw score
    char buf[128];
    riv_snprintf(buf, sizeof(buf), "SCORE %d", score);
    riv_draw_text(buf, RIV_SPRITESHEET_FONT_3X5, RIV_BOTTOMLEFT, 1, riv->height - 4, 1, RIV_COLOR_WHITE);
}

// Draw game start screen
void draw_start_screen()
{
    // Draw snake title
    riv_draw_text(
        "breakout",               // text to draw
        RIV_SPRITESHEET_FONT_5X7, // sprite sheet id of the font
        RIV_CENTER,               // anchor point on the text bounding box
        riv->width / 2,           // anchor x
        riv->height / 2,          // anchor y
        4,                        // text size multiplier
        RIV_COLOR_LIGHTGREEN      // text color
    );
    // Make "press to start blink" by changing the color depending on the frame number
    uint32_t col = (riv->frame % 8 == 0) ? RIV_COLOR_DARKRED : RIV_COLOR_LIGHTRED;
    // Draw press to start
    riv_draw_text("PRESS TO START", RIV_SPRITESHEET_FONT_5X7, RIV_CENTER, riv->width / 2, riv->height / 2 + 32, 1, col);
}

// Draw game over screen
void draw_end_screen()
{
    // Draw last game frame
    draw_game();
    // Draw GAME OVER
    riv_draw_text("GAME OVER", RIV_SPRITESHEET_FONT_5X7, RIV_CENTER, riv->width / 2, riv->height / 2, 2, RIV_COLOR_RED);
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
    // riv->width = 128;
    // riv->height = 128;
    riv->target_fps = 60;
    do
    {
        // Update game state
        update();
        // Draw game graphics
        draw();
    } while (riv_present());
    return 0;
}
