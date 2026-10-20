#include <emscripten.h>
#include <stdlib.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 700

#define MAX_BULLETS 45
#define MAX_ENEMIES 20

typedef struct {
    float x, y;
    float vx; // Velocidade horizontal para disparos em leque
    int active;
} Bullet;

typedef struct {
    float x, y;
    float speed_y;
    int active;
} Enemy;

typedef struct {
    float player_x;
    int player_lives;
    int score;
    int game_over;
    int enemy_hit_flag;
    float last_hit_x;
    float last_hit_y;
    int triple_shot_timer; // Controlo de tempo do power-up em quadros
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
} GameState;

static GameState state;

EMSCRIPTEN_KEEPALIVE
void init_game(void) {
    state.player_x = (float)SCREEN_WIDTH / 2.0f;
    state.player_lives = 5;
    state.score = 0;
    state.game_over = 0;
    state.enemy_hit_flag = 0;
    state.last_hit_x = 0;
    state.last_hit_y = 0;
    state.triple_shot_timer = 0;

    for (int i = 0; i < MAX_BULLETS; i++) {
        state.bullets[i].active = 0;
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        state.enemies[i].active = 0;
    }
}

EMSCRIPTEN_KEEPALIVE
void move_player(float dir) {
    if (state.game_over) return;
    state.player_x += dir * 9.0f;
    if (state.player_x < 36.0f) state.player_x = 36.0f;
    if (state.player_x > SCREEN_WIDTH - 36.0f) state.player_x = SCREEN_WIDTH - 36.0f;
}

static void spawn_bullet(float x, float y, float vx) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!state.bullets[i].active) {
            state.bullets[i].x = x;
            state.bullets[i].y = y;
            state.bullets[i].vx = vx;
            state.bullets[i].active = 1;
            break;
        }
    }
}

EMSCRIPTEN_KEEPALIVE
void shoot(void) {
    if (state.game_over) return;

    if (state.triple_shot_timer > 0) {
        // Disparo Triplo em leque
        spawn_bullet(state.player_x, (float)SCREEN_HEIGHT - 65.0f, 0.0f);
        spawn_bullet(state.player_x - 14.0f, (float)SCREEN_HEIGHT - 60.0f, -2.5f);
        spawn_bullet(state.player_x + 14.0f, (float)SCREEN_HEIGHT - 60.0f, 2.5f);
    } else {
        // Disparo normal
        spawn_bullet(state.player_x, (float)SCREEN_HEIGHT - 65.0f, 0.0f);
    }
}

EMSCRIPTEN_KEEPALIVE
void activate_triple_shot(int frames) {
    state.triple_shot_timer = frames;
}

EMSCRIPTEN_KEEPALIVE
void heal_player(void) {
    if (state.player_lives < 5) {
        state.player_lives++;
    }
}

static void spawn_enemy(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state.enemies[i].active) {
            state.enemies[i].x = 45.0f + (float)(rand() % (SCREEN_WIDTH - 90));
            state.enemies[i].y = -40.0f;
            state.enemies[i].speed_y = 0.9f + ((float)(rand() % 9) / 10.0f);
            state.enemies[i].active = 1;
            break;
        }
    }
}

EMSCRIPTEN_KEEPALIVE
void update_game(void) {
    if (state.game_over) return;

    state.enemy_hit_flag = 0;

    if (state.triple_shot_timer > 0) {
        state.triple_shot_timer--;
    }

    if (rand() % 80 == 0) {
        spawn_enemy();
    }

    // Atualiza projéteis
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (state.bullets[i].active) {
            state.bullets[i].y -= 10.5f;
            state.bullets[i].x += state.bullets[i].vx;

            if (state.bullets[i].y < -20.0f || state.bullets[i].x < -10.0f || state.bullets[i].x > SCREEN_WIDTH + 10.0f) {
                state.bullets[i].active = 0;
            }
        }
    }

    // Atualiza invasores e colisões
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state.enemies[i].active) continue;

        state.enemies[i].y += state.enemies[i].speed_y;

        if (state.enemies[i].y > SCREEN_HEIGHT + 20.0f) {
            state.enemies[i].active = 0;
            state.player_lives--;
            if (state.player_lives <= 0) {
                state.game_over = 1;
            }
            continue;
        }

        for (int b = 0; b < MAX_BULLETS; b++) {
            if (!state.bullets[b].active) continue;

            float dx = state.bullets[b].x - state.enemies[i].x;
            float dy = state.bullets[b].y - state.enemies[i].y;
            if (dx > -32.0f && dx < 32.0f && dy > -32.0f && dy < 32.0f) {
                state.last_hit_x = state.enemies[i].x;
                state.last_hit_y = state.enemies[i].y;
                state.enemies[i].active = 0;
                state.bullets[b].active = 0;
                state.score += 100;
                state.enemy_hit_flag = 1;
                break;
            }
        }
    }
}

EMSCRIPTEN_KEEPALIVE
float get_player_x(void) { return state.player_x; }

EMSCRIPTEN_KEEPALIVE
int get_player_lives(void) { return state.player_lives; }

EMSCRIPTEN_KEEPALIVE
int get_score(void) { return state.score; }

EMSCRIPTEN_KEEPALIVE
int is_game_over(void) { return state.game_over; }

EMSCRIPTEN_KEEPALIVE
int check_enemy_hit(void) { return state.enemy_hit_flag; }

EMSCRIPTEN_KEEPALIVE
float get_last_hit_x(void) { return state.last_hit_x; }

EMSCRIPTEN_KEEPALIVE
float get_last_hit_y(void) { return state.last_hit_y; }

EMSCRIPTEN_KEEPALIVE
int get_triple_shot_timer(void) { return state.triple_shot_timer; }

EMSCRIPTEN_KEEPALIVE
Bullet* get_bullets_ptr(void) { return state.bullets; }

EMSCRIPTEN_KEEPALIVE
Enemy* get_enemies_ptr(void) { return state.enemies; }