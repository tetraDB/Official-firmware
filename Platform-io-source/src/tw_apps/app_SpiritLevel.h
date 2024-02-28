#pragma once

#include "tw_apps/tw_app.h"

class AppSpiritLevel : public tw_app
{
	public:
		void setup(void);
		void pre_start(void);
		void draw(bool force);
		void draw_icon(uint8_t canvasid, int16_t _pos_x, int16_t _pos_y, uint8_t style_hint);
		bool process_touch(touch_event_t touch_event);

	private:		
		#define B_BLUE RGB(20, 20, 40)
		#define B_RED RGB(40, 20, 20)
		#define B_GREEN RGB(20, 40, 20)

		void drawUI();
};

extern AppSpiritLevel app_spirit_level;
