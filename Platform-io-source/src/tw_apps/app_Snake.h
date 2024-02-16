#pragma once
#include "tw_apps/tw_app.h"

class AppSnake : public tw_app
{
	struct SnakeSegment
	{
		uint8_t x;
		uint8_t y;
	};

	// snake defs
	#define SNAKE_SEGMENT_RADIUS 5
	#define SNAKE_SEGMENT_STEP 12

	// play area defs
	#define BOARD_WIDTH 18
	#define BOARD_HEIGHT 17
	#define BOARD_SEGMENT_STEP 12
	#define BOARD_POSITIONS BOARD_WIDTH * BOARD_HEIGHT

	#define BOARD_START_X 17
	#define BOARD_END_X BOARD_START_X + SNAKE_SEGMENT_STEP * BOARD_WIDTH

	#define BOARD_START_Y 48
	#define BOARD_END_Y BOARD_START_Y + SNAKE_SEGMENT_STEP * BOARD_HEIGHT

	// colour defs	
	#define DARK_SCORE_16  0x0840
	#define LIGHT_SCORE_16 0xd200
	#define MID_YELLOW_32  RGB(0x80, 0x80, 0x00)
	#define SNAKE_COLOUR_SEGMENT_OUTER RGB(0x00, 0xff, 0x00)
	#define SNAKE_COLOUR_SEGMENT_INNER RGB(0x00, 0x88, 0x00)
	#define SNAKE_COLOUR_HEAD_OUTER RGB(0xff, 0xff, 0x00)
	#define SNAKE_COLOUR_HEAD_INNER RGB(0x88, 0x88, 0x00)

	#define SNAKE_COLOUR_FOOD_INNER RGB(0xff, 0x00, 0x00)
	#define SNAKE_COLOUR_FOOD_OUTER RGB(0x88, 0x00, 0x00)
	
	// game states
	#define GAME_STATE_WAITING_TO_START 1
	#define GAME_STATE_PLAYING 2
	#define GAME_STATE_GAMEOVER 3

	// snake direction
	#define SNAKE_DIRECTION_NONE 0
	#define SNAKE_DIRECTION_UP 1
	#define SNAKE_DIRECTION_RIGHT 2
	#define SNAKE_DIRECTION_DOWN 3
	#define SNAKE_DIRECTION_LEFT 4

	#define IMU_PITCH_THRESHOLD 8.0

	public:
		void setup(void);
		void pre_start(void);
		void draw(bool force);
		void draw_icon(uint8_t canvasid, int16_t _pos_x, int16_t _pos_y, uint8_t style_hint);
		bool click(uint16_t pos_x, uint16_t pos_y);
		bool click_double(uint16_t click_pos_x, uint16_t click_pos_y);

	private:
		void draw_snake();
		void draw_score();
		void reset_game();
		void extend_snake(uint8_t x, uint8_t y);
		void eat_food(uint8_t x, uint8_t y);
		
		void move_snake();
		void imu_read();
		
		uint16_t score = 0;
		uint8_t snake_direction = 0;
		u_long snake_speed = 300;
		u_long snake_last_milli = 0;
		uint8_t game_state = 0;
		SnakeSegment snake_segments[BOARD_POSITIONS];
		uint16_t snake_length = 0;

		uint8_t food_x;
		uint8_t food_y;		
		void randomise_food();
		bool is_food_at(uint8_t x, uint8_t y);
};

extern AppSnake app_snake;