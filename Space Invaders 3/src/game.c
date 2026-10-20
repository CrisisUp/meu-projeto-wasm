#include <emscripten.h>
#include <stdlib.h>

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 700

#define MAX_BULLETS 50
#define MAX_ENEMY_BULLETS 20
#define MAX_ENEMIES 20

typedef struct {
    float x, y;
    float vx;
    int active;
} Bullet; // 16 bytes

typedef struct {
    float x, y;
    float vx, vy;
    int active;
} EnemyBullet; // 20 bytes

typedef struct {
    float x, y;
    float speed_y;
    int active;
} Enemy; // 16 bytes

typedef struct {
    float x, y;
    float vx;
    int hp;
    int max_hp;
    int active;
    int shoot_timer;
} Boss; // 28 bytes

typedef struct {
    float player_x;
    float player_vx;
    int player_lives;
    int score;
    int game_over;
    int enemy_hit_flag;
    float last_hit_x;
    float last_hit_y;
    int triple_shot_timer;
    int boss_defeated_flag;
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    EnemyBullet enemy_bullets[MAX_ENEMY_BULLETS];
    Boss boss;
} GameState;

static GameState state;

EMSCRIPTEN_KEEPALIVE
void init_game(void) {
    state.player_x = (float)SCREEN_WIDTH / 2.0f;
    state.player_vx = 0.0f;
    state.player_lives = 5;
    state.score = 0;
    state.game_over = 0;
    state.enemy_hit_flag = 0;
    state.boss_defeated_flag = 0;
    state.last_hit_x = 0;
    state.last_hit_y = 0;
    state.triple_shot_timer = 0;

    for (int i = 0; i < MAX_BULLETS; i++) state.bullets[i].active = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) state.enemies[i].active = 0;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) state.enemy_bullets[i].active = 0;

    state.boss.active = 0;
    state.boss.hp = 0;
    state.boss.max_hp = 50;
    state.boss.x = (float)SCREEN_WIDTH / 2.0f;
    state.boss.y = -100.0f;
    state.boss.vx = 2.2f;
    state.boss.shoot_timer = 0;
}

EMSCRIPTEN_KEEPALIVE
void apply_player_thrust(float dir) {
    if (state.game_over) return;
    const float ACCEL = 1.15f;
    const float MAX_SPEED = 9.5f;

    state.player_vx += dir * ACCEL;
    if (state.player_vx > MAX_SPEED) state.player_vx = MAX_SPEED;
    if (state.player_vx < -MAX_SPEED) state.player_vx = -MAX_SPEED;
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

static void spawn_enemy_bullet(float x, float y, float vx, float vy) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!state.enemy_bullets[i].active) {
            state.enemy_bullets[i].x = x;
            state.enemy_bullets[i].y = y;
            state.enemy_bullets[i].vx = vx;
            state.enemy_bullets[i].vy = vy;
            state.enemy_bullets[i].active = 1;
            break;
        }
    }
}

EMSCRIPTEN_KEEPALIVE
void shoot(void) {
    if (state.game_over) return;

    if (state.triple_shot_timer > 0) {
        spawn_bullet(state.player_x, (float)SCREEN_HEIGHT - 65.0f, 0.0f);
        spawn_bullet(state.player_x - 14.0f, (float)SCREEN_HEIGHT - 60.0f, -2.5f);
        spawn_bullet(state.player_x + 14.0f, (float)SCREEN_HEIGHT - 60.0f, 2.5f);
    } else {
        spawn_bullet(state.player_x, (float)SCREEN_HEIGHT - 65.0f, 0.0f);
    }
}

EMSCRIPTEN_KEEPALIVE
void activate_triple_shot(int frames) {
    state.triple_shot_timer = frames;
}

EMSCRIPTEN_KEEPALIVE
void heal_player(void) {
    if (state.player_lives < 5) state.player_lives++;
}

static void spawn_enemy(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state.enemies[i].active) {
            state.enemies[i].x = 45.0f + (float)(rand() % (SCREEN_WIDTH - 90));
            state.enemies[i].y = -40.0f;
            state.enemies[i].speed_y = 1.0f + ((float)(rand() % 8) / 10.0f);
            state.enemies[i].active = 1;
            break;
        }
    }
}

static void trigger_boss(void) {
    state.boss.active = 1;
    state.boss.max_hp = 50;
    state.boss.hp = 50;
    state.boss.x = (float)SCREEN_WIDTH / 2.0f;
    state.boss.y = -80.0f;
    state.boss.vx = 2.4f;
    state.boss.shoot_timer = 50;
}

EMSCRIPTEN_KEEPALIVE
void update_game(void) {
    if (state.game_over) return;

    state.enemy_hit_flag = 0;
    state.boss_defeated_flag = 0;

    // Movimentação do Jogador
    state.player_x += state.player_vx;
    state.player_vx *= 0.88f;

    if (state.player_x < 36.0f) { state.player_x = 36.0f; state.player_vx = 0.0f; }
    if (state.player_x > SCREEN_WIDTH - 36.0f) { state.player_x = SCREEN_WIDTH - 36.0f; state.player_vx = 0.0f; }

    if (state.triple_shot_timer > 0) state.triple_shot_timer--;

    // Condição para o Boss
    if (!state.boss.active && state.score >= 1500 && (state.score % 2500 < 500)) {
        trigger_boss();
    }

    // Inimigos normais surgem se o Boss não estiver ativo
    if (!state.boss.active) {
        if (rand() % 65 == 0) {
            spawn_enemy();
        }
    }

    // Atualiza tiros do jogador
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (state.bullets[i].active) {
            state.bullets[i].y -= 10.5f;
            state.bullets[i].x += state.bullets[i].vx;

            if (state.bullets[i].y < -20.0f || state.bullets[i].x < -10.0f || state.bullets[i].x > SCREEN_WIDTH + 10.0f) {
                state.bullets[i].active = 0;
            }
        }
    }

    // Atualiza o Boss
    if (state.boss.active) {
        if (state.boss.y < 90.0f) {
            state.boss.y += 1.2f;
        } else {
            state.boss.x += state.boss.vx;
            if (state.boss.x < 70.0f || state.boss.x > SCREEN_WIDTH - 70.0f) {
                state.boss.vx = -state.boss.vx;
            }

            state.boss.shoot_timer--;
            if (state.boss.shoot_timer <= 0) {
                state.boss.shoot_timer = 60;
                spawn_enemy_bullet(state.boss.x - 28.0f, state.boss.y + 35.0f, -1.2f, 4.5f);
                spawn_enemy_bullet(state.boss.x + 28.0f, state.boss.y + 35.0f, 1.2f, 4.5f);
                spawn_enemy_bullet(state.boss.x, state.boss.y + 40.0f, 0.0f, 5.0f);
            }
        }

        // Colisão com tiros do jogador
        for (int b = 0; b < MAX_BULLETS; b++) {
            if (!state.bullets[b].active) continue;

            float dx = state.bullets[b].x - state.boss.x;
            float dy = state.bullets[b].y - state.boss.y;
            if (dx > -48.0f && dx < 48.0f && dy > -40.0f && dy < 40.0f) {
                state.bullets[b].active = 0;
                state.boss.hp--;
                state.last_hit_x = state.bullets[b].x;
                state.last_hit_y = state.bullets[b].y;
                state.enemy_hit_flag = 1;

                if (state.boss.hp <= 0) {
                    state.boss.active = 0;
                    state.score += 1000;
                    state.boss_defeated_flag = 1;
                    state.last_hit_x = state.boss.x;
                    state.last_hit_y = state.boss.y;
                }
                break;
            }
        }
    }

    // Atualiza tiros inimigos
    for (int eb = 0; eb < MAX_ENEMY_BULLETS; eb++) {
        if (!state.enemy_bullets[eb].active) continue;

        state.enemy_bullets[eb].x += state.enemy_bullets[eb].vx;
        state.enemy_bullets[eb].y += state.enemy_bullets[eb].vy;

        float dx = state.enemy_bullets[eb].x - state.player_x;
        float dy = state.enemy_bullets[eb].y - (SCREEN_HEIGHT - 55.0f);
        if (dx > -24.0f && dx < 24.0f && dy > -24.0f && dy < 24.0f) {
            state.enemy_bullets[eb].active = 0;
            state.player_lives--;
            if (state.player_lives <= 0) state.game_over = 1;
            continue;
        }

        if (state.enemy_bullets[eb].y > SCREEN_HEIGHT + 20.0f || state.enemy_bullets[eb].x < -20.0f || state.enemy_bullets[eb].x > SCREEN_WIDTH + 20.0f) {
            state.enemy_bullets[eb].active = 0;
        }
    }

    // Atualiza inimigos normais
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!state.enemies[i].active) continue;

        state.enemies[i].y += state.enemies[i].speed_y;

        if (state.enemies[i].y > SCREEN_HEIGHT + 20.0f) {
            state.enemies[i].active = 0;
            state.player_lives--;
            if (state.player_lives <= 0) state.game_over = 1;
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

EMSCRIPTEN_KEEPALIVE float get_player_x(void) { return state.player_x; }
EMSCRIPTEN_KEEPALIVE float get_player_vx(void) { return state.player_vx; }
EMSCRIPTEN_KEEPALIVE int get_player_lives(void) { return state.player_lives; }
EMSCRIPTEN_KEEPALIVE int get_score(void) { return state.score; }
EMSCRIPTEN_KEEPALIVE int is_game_over(void) { return state.game_over; }
EMSCRIPTEN_KEEPALIVE int check_enemy_hit(void) { return state.enemy_hit_flag; }
EMSCRIPTEN_KEEPALIVE int check_boss_defeated(void) { return state.boss_defeated_flag; }
EMSCRIPTEN_KEEPALIVE float get_last_hit_x(void) { return state.last_hit_x; }
EMSCRIPTEN_KEEPALIVE float get_last_hit_y(void) { return state.last_hit_y; }
EMSCRIPTEN_KEEPALIVE int get_triple_shot_timer(void) { return state.triple_shot_timer; }
EMSCRIPTEN_KEEPALIVE int is_boss_active(void) { return state.boss.active; }
EMSCRIPTEN_KEEPALIVE float get_boss_x(void) { return state.boss.x; }
EMSCRIPTEN_KEEPALIVE float get_boss_y(void) { return state.boss.y; }
EMSCRIPTEN_KEEPALIVE int get_boss_hp(void) { return state.boss.hp; }
EMSCRIPTEN_KEEPALIVE int get_boss_max_hp(void) { return state.boss.max_hp; }
EMSCRIPTEN_KEEPALIVE Bullet* get_bullets_ptr(void) { return state.bullets; }
EMSCRIPTEN_KEEPALIVE EnemyBullet* get_enemy_bullets_ptr(void) { return state.enemy_bullets; }
EMSCRIPTEN_KEEPALIVE Enemy* get_enemies_ptr(void) { return state.enemies; }