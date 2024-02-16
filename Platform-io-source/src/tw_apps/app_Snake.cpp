
#include "tw_apps/app_Snake.h"
#include "fonts/Clock_Digits.h"
#include "fonts/RobotoMono_Regular_All.h"
#include "peripherals/imu.h"

/**
 * @brief Draw the icon that gets shown on the app menu face
 *
 * Icons are 64x64 with rounded corners as per the code below, but the inner content can be anything that represents the app well
 * @param canvasid
 * @param _pos_x
 * @param _pos_y
 * @param style_hint
 */
void AppSnake::draw_icon(uint8_t canvasid, int16_t _pos_x, int16_t _pos_y, uint8_t style_hint)
{
	if (!is_icon_cached)
	{
		is_icon_cached = true;
		icon_sprite.fillSprite(0);
		icon_sprite.drawSmoothRoundRect(0, 0, 10, 6, icon_width, icon_height, RGB(0x00, 0xff, 0x00), 0);
		int borderGap = 10;
		int availableHeight = icon_height - borderGap * 2; 
		int count = 5;
		float size = availableHeight / count;
		float radius = (size / 2);		
		float starty = radius + borderGap;
		float stepy = size;

		for(int i = 0; i < count; i++)
		{
			icon_sprite.fillSmoothCircle(icon_width / 2, starty + (stepy * i), radius, RGB(0x00, 0xff, 0x00), RGB(0x00, 0xaa, 0x00));
		}
	}

	icon_x = _pos_x;
	icon_y = _pos_y;
	icon_sprite.pushToSprite(&canvas[canvasid], _pos_x, _pos_y);
}

/**
 * 
 */
void AppSnake::setup()
{
	if (!is_setup)
	{
		is_setup = true;
		// Add any one time setup code here
	}
}

/**
 * @brief Put anything in here that you want to have run every time the app is opened
 *
 * This is not the same as setup() above that only ever gets called the first time the app opens
 *
 */
void AppSnake::pre_start()
{	
	canvas[canvasid].setFreeFont(RobotoMono_Regular[15]);
	reset_game();
}

/**
 * Reset the game to initial state
 */
void AppSnake::reset_game()
{
	score = 0;
	snake_length = 3;
	game_state = GAME_STATE_WAITING_TO_START;
	snake_direction = SNAKE_DIRECTION_UP;
	snake_segments[0] = { BOARD_WIDTH / 2, BOARD_HEIGHT / 2 };
	snake_segments[1] = { snake_segments[0].x, (uint8_t)(snake_segments[0].y - 1) };
	snake_segments[2] = { snake_segments[0].x, (uint8_t)(snake_segments[0].y - 2) };
	randomise_food();
	snake_last_milli = millis();
}

/**
 * Main drawing call
 */
void AppSnake::draw(bool force)
{
	
	if (force || millis() - next_update > update_period)
	{
		setup();
		imu_read();
		next_update = millis();
		if (next_update - snake_last_milli > snake_speed)
		{
			snake_last_milli = next_update;
			draw_snake();
		}		
		canvas[canvasid].pushSprite(_x, _y);
	}
}

/**
 *
 */
void AppSnake::extend_snake(uint8_t x, uint8_t y)
{
	if (snake_length < BOARD_POSITIONS)
	{
		snake_length++;
		for(int i = snake_length-1; i > 0; i--)
		{
			snake_segments[i].x = snake_segments[i-1].x;
			snake_segments[i].y = snake_segments[i-1].y;
		}		
		snake_segments[0].x = x;
		snake_segments[0].y = y;
	}
}

/**
 * 
 */
void AppSnake::eat_food(uint8_t x, uint8_t y)
{
	score += 3;
	randomise_food();
	extend_snake(x, y);
	if (snake_speed > 50)
		snake_speed -= 10;
}
/**
 * 
 */
void AppSnake::randomise_food()
{
	food_x = random(0,BOARD_WIDTH - 1) & 0xFF;
	food_y = random(0,BOARD_HEIGHT - 1) & 0xFF;
}

/**
 * 
 */
bool AppSnake::is_food_at(uint8_t x, uint8_t y)
{
	bool eat = food_x == x && food_y == y;
	if (eat)
		eat_food(x,y);

	return eat;
}

/**
 * 
 */
void AppSnake::imu_read()
{	
	imu.update();
	float pitch = imu.get_pitch();
	float roll = imu.get_roll();

	if(abs(pitch) > abs(roll))
	{
		if (pitch > IMU_PITCH_THRESHOLD && snake_direction != SNAKE_DIRECTION_UP) snake_direction = SNAKE_DIRECTION_DOWN;
		if (pitch < -IMU_PITCH_THRESHOLD && snake_direction != SNAKE_DIRECTION_DOWN) snake_direction = SNAKE_DIRECTION_UP;
	}
	else
	{
		if (roll > IMU_PITCH_THRESHOLD && snake_direction != SNAKE_DIRECTION_RIGHT) snake_direction = SNAKE_DIRECTION_LEFT;
		if (roll < -IMU_PITCH_THRESHOLD && snake_direction != SNAKE_DIRECTION_LEFT) snake_direction = SNAKE_DIRECTION_RIGHT;
	}
}

/**
 * 
 */
void AppSnake::move_snake()
{	
	bool ate = false;
	int8_t move_x = 0;
	int8_t move_y = 0;

	switch(snake_direction)
	{
		case SNAKE_DIRECTION_UP:
		{
			if (snake_segments[0].y > 0)
			{
				if (!is_food_at(snake_segments[0].x, snake_segments[0].y-1))
					move_y = -1;
			}
			else
				game_state = GAME_STATE_GAMEOVER;
			break;
		}

		case SNAKE_DIRECTION_RIGHT:
		{
			if (snake_segments[0].x < BOARD_WIDTH - 1)
			{
				if (!is_food_at(snake_segments[0].x+1, snake_segments[0].y))				
					move_x = 1;
			}
			else
				game_state = GAME_STATE_GAMEOVER;
			break;
		}

		case SNAKE_DIRECTION_DOWN:
		{
			if (snake_segments[0].y < BOARD_HEIGHT - 1)
			{
				if (!is_food_at(snake_segments[0].x, snake_segments[0].y+1))
					move_y = 1;
			}
			else
				game_state = GAME_STATE_GAMEOVER;
			break;
		}

		case SNAKE_DIRECTION_LEFT:
		{
			if (snake_segments[0].x > 0)
			{
				if (!is_food_at(snake_segments[0].x-1, snake_segments[0].y))
					move_x = -1;
			}
			else
				game_state = GAME_STATE_GAMEOVER;
			break;
		}
	}	
	
	if (snake_direction != SNAKE_DIRECTION_NONE && (move_x != 0 || move_y != 0))
	{
		for(int i = snake_length-1; i > 0; i--)
		{
			snake_segments[i].x = snake_segments[i-1].x;
			snake_segments[i].y = snake_segments[i-1].y;
		}
		snake_segments[0].x += move_x;
		snake_segments[0].y += move_y;

		for(int i = 1; i < snake_length-1; i++)
		{
			if (
				snake_segments[0].x == snake_segments[i].x 
				&& snake_segments[0].y == snake_segments[i].y
			)
				game_state = GAME_STATE_GAMEOVER;
		}
	}
}

/**
 * 
 */
void AppSnake::draw_score()
{
	canvas[canvasid].setTextDatum(TL_DATUM);
	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].drawString("Score: ", 28, 6);

	canvas[canvasid].setTextDatum(TR_DATUM);
	canvas[canvasid].setTextColor(DARK_SCORE_16);
	canvas[canvasid].drawString("000000", display.width-28, 6);

	canvas[canvasid].setTextColor(LIGHT_SCORE_16);
	canvas[canvasid].drawString(String(score), display.width-28, 6);	
}

/**
 * 
 */
void AppSnake::draw_snake()
{
	canvas[canvasid].fillSprite(TFT_BLACK);
	canvas[canvasid].drawSmoothRoundRect(2, 35, 10, 10, display.width-6, display.height - 62, MID_YELLOW_32, 0);

	switch(game_state)
	{
		case GAME_STATE_WAITING_TO_START:
		{ 
			canvas[canvasid].setTextDatum(CC_DATUM);
			canvas[canvasid].setTextColor(TFT_WHITE);
			canvas[canvasid].drawString("Snake v0.1", display.width / 2, 15);

			canvas[canvasid].setTextDatum(CC_DATUM);
			canvas[canvasid].setTextColor(TFT_WHITE);
			canvas[canvasid].drawString("Tap to start", display.width / 2, display.height / 2);
			break;
		}

		case GAME_STATE_PLAYING:
		{			
			draw_score();
			move_snake();
			for (uint8_t i = 0; i < snake_length; i ++)
			{
				canvas[canvasid].fillSmoothCircle(
					BOARD_START_X + snake_segments[i].x * BOARD_SEGMENT_STEP,
					BOARD_START_Y + snake_segments[i].y * BOARD_SEGMENT_STEP,
					SNAKE_SEGMENT_RADIUS,
					SNAKE_COLOUR_SEGMENT_OUTER,
					SNAKE_COLOUR_SEGMENT_INNER
				);				
			}

			canvas[canvasid].fillSmoothCircle(
				BOARD_START_X + food_x * BOARD_SEGMENT_STEP,
				BOARD_START_Y + food_y * BOARD_SEGMENT_STEP,
				SNAKE_SEGMENT_RADIUS,
				SNAKE_COLOUR_FOOD_OUTER,
				SNAKE_COLOUR_FOOD_INNER
			);
			break;		
		}

		case GAME_STATE_GAMEOVER:
		{
			draw_score();
			canvas[canvasid].setTextDatum(CC_DATUM);
			canvas[canvasid].setTextColor(TFT_RED);
			canvas[canvasid].drawString("Game Over", display.width / 2, display.height / 2);
		}
	}

	canvas[canvasid].setTextDatum(CC_DATUM);
	canvas[canvasid].setTextColor(TFT_DARKGREY);
	canvas[canvasid].drawString("by tetraDB", display.width / 2, display.height - 15);
}

/**
 * 
 */
bool AppSnake::click(uint16_t pos_x, uint16_t pos_y) 
{
	switch(game_state)
	{
		case GAME_STATE_WAITING_TO_START: 
		{
			game_state = GAME_STATE_PLAYING;
			return true;
		}

		//case 2:
		//{			
		//	return false;
		//}
	}
	return false;
}

/**
 * 
 */
bool AppSnake::click_double(uint16_t click_pos_x, uint16_t click_pos_y)
{
	reset_game();
	return true; 
}

AppSnake app_snake;