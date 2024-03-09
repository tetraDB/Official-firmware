#pragma once

#include "tw_apps/tw_app.h"

class AppCompass : public tw_app
{
	public:
		void setup(void);
		void pre_start(void);
		void draw(bool force);
		void draw_icon(uint8_t canvasid, int16_t _pos_x, int16_t _pos_y, uint8_t style_hint);
		bool process_touch(touch_event_t touch_event);

	private:

		#define BACKGROUND TFT_BLACK
		
		// needle values
		#define NEEDLE_L 84 / 2 // Needle length is 84, we want radius which is 42
		#define NEEDLE_W 12 / 2 // Needle width is 12, radius is then 6
		#define NESW_RADIUS 60 // radius that N E S W rotate around	
			
		// running states
		#define RUNNING_STATE_DRAW 1
		#define RUNNING_STATE_CALIBRATE 2
		
		#define B_BLUE RGB(10, 40, 60)
		#define B_RED RGB(40, 20, 20)
		#define B_GREEN RGB(20, 40, 20)
		#define COLOUR_CIRCLE_1 RGB(251, 202, 100)
		#define COLOUR_CIRCLE_2 RGB(126, 46, 11)
		#define COLOUR_DASH RGB(223, 127, 27)
		#define COLOUR_BLADE_LIGHT RGB(255, 246, 201)
		#define COLOUR_BLADE_MID  RGB(255, 236, 142)
		#define COLOUR_BLADE_DARK RGB(181, 82, 14)

		#define polling_rate_ms 100
		unsigned long last_poll;

		float heading_target = 0;
		float heading_current = 0;
		float heading_momentum = 0;
		
		float pitch_target = 0;
		float pitch_current = 0;
		float pitch_momentum = 0;
		
		float roll_target = 0;
		float roll_current = 0;
		float roll_momentum = 0;

		uint8_t running_state;

		void drawUI();
		void drawCompass(float heading);
		void drawCalibrate();
		void resetCalibration();
		void update_heading();
		void drawCircle(TFT_eSprite* sprite, int16_t x, int16_t y, int16_t diameter, int16_t thickness, uint32_t colour);

		//these are needed for calibration

		float mag_x_min;
		float mag_y_min;
		float mag_z_min;

		float mag_x_max;
		float mag_y_max;
		float mag_z_max;
};

extern AppCompass app_compass;
