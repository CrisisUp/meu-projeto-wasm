CC = emcc
FUNCS = "['_init_game','_apply_player_thrust','_shoot','_update_game','_get_player_x','_get_player_vx','_get_player_lives','_get_score','_is_game_over','_check_enemy_hit','_get_last_hit_x','_get_last_hit_y','_activate_triple_shot','_heal_player','_get_triple_shot_timer','_get_bullets_ptr','_get_enemies_ptr']"

CFLAGS = -O3 -s STANDALONE_WASM -s EXPORTED_FUNCTIONS=$(FUNCS) -s EXPORTED_RUNTIME_METHODS="['WebAssembly']" --no-entry

SRC = src/game.c
TARGET = dist/game.wasm

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(TARGET)

serve:
	cd dist && python -m http.server 8080

clean:
	cmd /c "del /f /q dist\game.wasm 2>nul"

.PHONY: all serve clean