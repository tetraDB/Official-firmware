#pragma once

#include "tw_apps/tw_app.h"

class AppCompass : public tw_app
{
	#define NEEDLE_L 84 / 2 // Needle length is 84, we want radius which is 42
	#define NEEDLE_W 12 / 2 // Needle width is 12, radius is then 6
	#define NESW_RADIUS 60 // radius that N E S W rotate around

	// running states
	#define RUNNING_STATE_COMPAS 1
	#define RUNNING_STATE_CALIBRATE 2

	public:
		void setup(void);
		void pre_start(void);
		void draw(bool force);
		void draw_icon(uint8_t canvasid, int16_t _pos_x, int16_t _pos_y, uint8_t style_hint);
		bool click(uint16_t touch_pos_x, uint16_t touch_pos_y);
		bool click_double(uint16_t touch_pos_x, uint16_t touch_pos_y);

		void drawCompass(int x, int y, float angle);
		void drawCalibrate();
		void getCoord(int x, int y, int *xp, int *yp, int r, float a);

	private:
		String version = "1.0";
		bool showingGyro = false;

		int number = 0;
		int angle = 0;

		int lx1 = 0;
		int ly1 = 0;
		int lx2 = 0;
		int ly2 = 0;
		int lx3 = 0;
		int ly3 = 0;
		int lx4 = 0;
		int ly4 = 0;

		// Test only
		uint16_t n = 0;
		uint32_t dt = 0;
		
		uint8_t running_state;
		bool has_calibrated = false;
};

extern AppCompass app_compass;
