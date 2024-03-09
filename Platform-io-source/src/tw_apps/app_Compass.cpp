
#include "tw_apps/app_Compass.h"
#include "fonts/Clock_Digits.h"
#include "fonts/RobotoMono_Regular_All.h"
#include "peripherals/imu.h"
#include "settings/settings.h"

/**
 * 
 */
void AppCompass::setup()
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
void AppCompass::pre_start() 
{
	running_state = RUNNING_STATE_DRAW;
	last_poll = millis() - polling_rate_ms;
}

/**
 * @brief Draw the icon that gets shown on the app menu face
 *
 * Icons are 64x64 with rounded corners as per the code below, but the inner content can be anything that represents the app well
 * @param canvasid
 * @param _pos_x
 * @param _pos_y
 * @param style_hint
 */
void AppCompass::draw_icon(uint8_t canvasid, int16_t _pos_x, int16_t _pos_y, uint8_t style_hint)
{
	if (!is_icon_cached)
	{
		is_icon_cached = true;
		icon_sprite.fillSprite(0);
		icon_sprite.drawSmoothRoundRect(0, 0, 10, 6, icon_width, icon_width, RGB(0xdd, 0xa6, 0x00), 0);
		icon_sprite.setTextDatum(4); // Middle, Center
		icon_sprite.setFreeFont(RobotoMono_Regular[15]);
		icon_sprite.setTextColor(TFT_WHITE);
		icon_sprite.drawSmoothCircle(icon_width / 2, icon_height / 2, 20, RGB(0xff, 0xc9, 0x00), 0);
		icon_sprite.drawString("C", icon_width / 2, icon_width / 2);
	}

	icon_x = _pos_x;
	icon_y = _pos_y;
	icon_sprite.pushToSprite(&canvas[canvasid], _pos_x, _pos_y);
}

/**
 * 
 */
bool AppCompass::process_touch(touch_event_t touch_event)
{
	if (touch_event.type == TOUCH_SWIPE)
	{
		return false;
	}
	else if (touch_event.type == TOUCH_TAP)
	{
		if (running_state == RUNNING_STATE_DRAW)
		{
			if (touch_event.x < 32u && touch_event.y < 32u)
			{
				info_println("Entering calibrating magnetometer");
				resetCalibration();
				running_state = RUNNING_STATE_CALIBRATE;
				return true;
			}
			else
				return false;
		}
		else if (running_state == RUNNING_STATE_CALIBRATE)
		{
			if (touch_event.x < 32u && touch_event.y > 248u)
			{				
				// https://github.com/kriswiner/MPU6050/wiki/Simple-and-Effective-Magnetometer-Calibration
				info_println("Saving compass calibration values");

				float soft_iron_x = (mag_x_max - mag_x_min) / 2;
				float soft_iron_y = (mag_y_max - mag_y_min) / 2;
				float soft_iron_z = (mag_z_max - mag_z_min) / 2;
			
				float soft_scale = (soft_iron_x + soft_iron_y + soft_iron_z) / 3;

				settings.config.compass.hard_iron_x = (mag_x_max + mag_x_min) / 2;
				settings.config.compass.hard_iron_y = (mag_y_max + mag_y_min) / 2;
				settings.config.compass.hard_iron_z = (mag_z_max + mag_z_min) / 2;

				settings.config.compass.soft_iron_x = soft_scale / soft_iron_x;
				settings.config.compass.soft_iron_y = soft_scale / soft_iron_y;
				settings.config.compass.soft_iron_z = soft_scale / soft_iron_z;

				settings.save(true);
				
				last_poll = millis() - polling_rate_ms; // prevent the draw state waiting for some catchup?? needs investigating more because that shouldn't be the case
				running_state = RUNNING_STATE_DRAW;
				return true;
			}
			else if (touch_event.x > 208u && touch_event.y > 248u)
			{
				info_println("Canceled compass calibration");
				running_state = RUNNING_STATE_DRAW;
				return true;
			}
			else
			{
				info_println("Click " + String(touch_event.x) + "," + String(touch_event.y));
				canvas[canvasid].fillSmoothCircle(touch_event.x, touch_event.y, 3, TFT_ORANGE, TFT_ORANGE);
				return false;
			}
		}
	}
	return false;
}

/**
 * 
 */
void AppCompass::update_heading()
{
	long dif = millis() - last_poll;
	if (dif > polling_rate_ms)
	{
		last_poll = millis() + (dif - polling_rate_ms);

		imu.update();

		pitch_target = imu.get_pitch();
		roll_target = imu.get_roll();

		float mag_x;
		float mag_y;
		float mag_z;
		
		imu.get_magnetic(&mag_x, &mag_y, &mag_z, true);
		mag_x = -mag_x;	// invert x because the sensor is upside down

		float mag_pitch =  roll_target * DEG_TO_RAD; // mag pitch uses imu roll
		float mag_roll = -pitch_target * DEG_TO_RAD; // mag roll uses imu pitch

		float tilt_correct_x = mag_x * cos(mag_pitch) + mag_y * sin(mag_roll) * sin(mag_pitch) - mag_z * cos(mag_roll) * sin(mag_pitch);
		float tilt_correct_y = mag_y * cos(mag_roll) + mag_z * sin(mag_roll);

		heading_target = atan2f(tilt_correct_x, tilt_correct_y) * RAD_TO_DEG;
		
		float a = heading_target - heading_current;
		float b = heading_target < heading_current
			? heading_target + 360.0 - heading_current 
			: heading_target - 360.0 - heading_current
		;
		heading_target = abs(a) < abs(b)
			? heading_current + a
			: heading_current + b
		;
		
		heading_momentum = heading_target - heading_current;
	}

	heading_current += heading_momentum * 0.5;
	heading_momentum = heading_target - heading_current;
	heading_momentum *= 0.5;

	if (heading_current >= 360.0)
	{
		heading_current -= 360.0;
		heading_target -= 360.0;
	}

	if (heading_current < 0.0)
	{
		heading_current += 360.0;
		heading_target += 360.0;
	}
}

/**
 * 
 */
void AppCompass::draw(bool force)
{
	// Override the CPU settings for this app (if needed)
	setCpuFrequencyMhz(required_cpu_speed);

	if (force || millis() - next_update > update_period)
	{
		setup();
		next_update = millis();
		
		switch(running_state)
		{
			case RUNNING_STATE_DRAW:
			{
				update_heading();
				drawCompass(heading_current); // Draw centre of compass at 120,140
				break;
			}

			case RUNNING_STATE_CALIBRATE:
			{
				drawCalibrate();
				break;
			}
		}

		drawUI();
		canvas[canvasid].pushSprite(_x, _y);
	}
}

/**
 * 
 */
void AppCompass::resetCalibration()
{
	mag_x_min = 6000;
	mag_y_min = 6000;
	mag_z_min = 6000;

	mag_x_max = -6000;
	mag_y_max = -6000;
	mag_z_max = -6000;
	
	canvas[canvasid].fillSprite(TFT_BLACK);
}

/**
 *
 */
void AppCompass::drawCalibrate()
{
	float mag_x;
	float mag_y;
	float mag_z;

	imu.get_magnetic(&mag_x, &mag_y, &mag_z, false);

	mag_x_min = min(mag_x, mag_x_min);
	mag_y_min = min(mag_y, mag_y_min);
	mag_z_min = min(mag_z, mag_z_min);

	mag_x_max = max(mag_x, mag_x_max);
	mag_y_max = max(mag_y, mag_y_max);
	mag_z_max = max(mag_z, mag_z_max);

	canvas[canvasid].fillSprite(TFT_BLACK);

	canvas[canvasid].setFreeFont(RobotoMono_Regular[9]);
	
	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].setTextDatum(CC_DATUM);
	canvas[canvasid].drawString("Calibrating", display.width / 2, 20);		
	
	//canvas[canvasid].drawPixel(display.center_x + mag_x, display.center_y + mag_y, TFT_RED);
	//canvas[canvasid].drawPixel(display.center_x + mag_y, display.center_y + mag_z, TFT_GREEN);
	//canvas[canvasid].drawPixel(display.center_x + mag_z, display.center_y + mag_x, TFT_BLUE);
	
	//float xx = (mag_x_max + mag_x_min) / 2;
	//float yy = (mag_y_max + mag_y_min) / 2;
	//float zz = (mag_z_max + mag_z_min) / 2;

	//canvas[canvasid].drawSmoothCircle(display.center_x + xx, display.center_y + yy, abs(mag_x_max - mag_x_min) / 2, TFT_RED, TFT_TRANSPARENT);
	//canvas[canvasid].drawSmoothCircle(display.center_x + yy, display.center_y + zz, abs(mag_y_max - mag_y_min) / 2, TFT_GREEN, TFT_TRANSPARENT);
	//canvas[canvasid].drawSmoothCircle(display.center_x + zz, display.center_y + xx, abs(mag_z_max - mag_z_min) / 2, TFT_BLUE, TFT_TRANSPARENT);

	canvas[canvasid].drawSmoothCircle(display.center_x, display.center_y, abs(mag_x_max - mag_x_min) / 2, TFT_RED, TFT_TRANSPARENT);
	canvas[canvasid].drawSmoothCircle(display.center_x, display.center_y, abs(mag_y_max - mag_y_min) / 2, TFT_GREEN, TFT_TRANSPARENT);
	canvas[canvasid].drawSmoothCircle(display.center_x, display.center_y, abs(mag_z_max - mag_z_min) / 2, TFT_BLUE, TFT_TRANSPARENT);
}



/**
 * 
 */
void AppCompass::drawCircle(TFT_eSprite* sprite, int16_t x, int16_t y, int16_t diameter, int16_t thickness, uint32_t colour)
{
	thickness *= .5;
	int16_t half_thickness = thickness / 2;
	int16_t half_thickness2 = thickness - half_thickness;

	int16_t radius = diameter / 2;

	int16_t left = x - radius - half_thickness;
	int16_t top = y - radius - half_thickness;

	sprite->drawSmoothRoundRect(left, top, radius + half_thickness, radius - half_thickness2, 0, 0, colour, TFT_TRANSPARENT);
}


void AppCompass::drawCompass(float heading)
{	

	canvas[canvasid].fillSprite(TFT_BLACK);

	/*
	 Rotate a unit 1 vector pointing down (Cartesian coordinate) [x: 0, y: 1] around the origin at the given angle
	 We can reduce the calcs in the rotation matrix due to the x component being 0 and y being 1
	 
	 xp = x(0) * cos(a) - y(1) * sin(a)
	 yp = y(1) * cos(a) + x(0) * sin(a)
	 
	 This recuces down to
	 
	 xp = -y * sin(a)
	 yp =  y * cos(a)
	 
	 The result is a normalized rotated vector that can then be multiplied by a radius
	*/
	
	float heading_in_rad = heading * DEG_TO_RAD;
	
	float normal_x, normal_y, normal_x_90, normal_y_90;	
	
	// get the rotated normalised vector
	normal_x = -sin(heading_in_rad);
	normal_y =  cos(heading_in_rad);

	// trick that rotates the normal 90 deg for E and W
	normal_x_90 = -normal_y;
	normal_y_90 =  normal_x;

	uint8_t text_N_x = display.center_x - NESW_RADIUS * normal_x;
	uint8_t text_N_y = display.center_y - NESW_RADIUS * normal_y;
	uint8_t text_S_x = display.center_x + NESW_RADIUS * normal_x;
	uint8_t text_S_y = display.center_y + NESW_RADIUS * normal_y;

	uint8_t text_E_x = display.center_x - NESW_RADIUS * normal_x_90;
	uint8_t text_E_y = display.center_y - NESW_RADIUS * normal_y_90;	
	uint8_t text_W_x = display.center_x + NESW_RADIUS * normal_x_90;
	uint8_t text_W_y = display.center_y + NESW_RADIUS * normal_y_90;

	canvas[canvasid].drawCircle(120, 140, 30, TFT_DARKGREY);

	canvas[canvasid].setFreeFont(RobotoMono_Regular[12]);
	canvas[canvasid].setTextColor(TFT_RED);
	canvas[canvasid].setTextDatum(CC_DATUM);
	canvas[canvasid].drawString("N", text_N_x, text_N_y);
	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].drawString("E", text_E_x, text_E_y);
	canvas[canvasid].drawString("S", text_S_x, text_S_y);
	canvas[canvasid].drawString("W", text_W_x, text_W_y);
	canvas[canvasid].setTextColor(TFT_ORANGE);
	uint8_t l = canvas[canvasid].drawString(String(heading, 0), display.center_x, 240);
	canvas[canvasid].drawSmoothCircle(display.center_x + l * .5 + 7, 236, 3, TFT_ORANGE, TFT_BLACK);

	uint8_t needle_N_x = display.center_x - NEEDLE_L * normal_x;
	uint8_t needle_N_y = display.center_y - NEEDLE_L * normal_y;
	uint8_t needle_S_x = display.center_x + NEEDLE_L * normal_x;
	uint8_t needle_S_y = display.center_y + NEEDLE_L * normal_y;

	uint8_t needle_E_x = display.center_x - NEEDLE_W * normal_x_90;
	uint8_t needle_E_y = display.center_y - NEEDLE_W * normal_y_90;
	uint8_t needle_W_x = display.center_x + NEEDLE_W * normal_x_90;
	uint8_t needle_W_y = display.center_y + NEEDLE_W * normal_y_90;

	canvas[canvasid].fillTriangle(needle_N_x, needle_N_y, needle_E_x, needle_E_y, needle_W_x, needle_W_y, TFT_RED);
	canvas[canvasid].fillTriangle(needle_S_x, needle_S_y, needle_E_x, needle_E_y, needle_W_x, needle_W_y, TFT_LIGHTGREY);
	canvas[canvasid].fillSmoothCircle(120, 140, 3, TFT_DARKGREY, TFT_LIGHTGREY);
}

/**
 * 
 */
// void AppCompass::drawCompass(float heading)
// {	

// 	canvas[canvasid].fillSprite(TFT_BLACK);


// 	/*
// 	 Rotate a unit 1 vector pointing down (Cartesian coordinate) [x: 0, y: 1] around the origin at the given angle
// 	 We can reduce the calcs in the rotation matrix due to the x component being 0 and y being 1
	 
// 	 xp = x(0) * cos(a) - y(1) * sin(a)
// 	 yp = y(1) * cos(a) + x(0) * sin(a)
	 
// 	 This recuces down to
	 
// 	 xp = -y * sin(a)
// 	 yp =  y * cos(a)
	 
// 	 The result is a normalized rotated vector that can then be multiplied by a radius
// 	*/
	
// 	float heading_in_rad = heading * DEG_TO_RAD;
	
// 	float normal_x, normal_y, normal_x_90, normal_y_90;	
	
// 	// get the rotated normalised vector
// 	normal_x = -sin(heading_in_rad);
// 	normal_y =  cos(heading_in_rad);

// 	// trick that rotates the normal 90 deg for E and W
// 	normal_x_90 = -normal_y;
// 	normal_y_90 =  normal_x;

// 	uint8_t text_N_x = display.center_x - NESW_RADIUS * normal_x;
// 	uint8_t text_N_y = display.center_y - NESW_RADIUS * normal_y;
// 	uint8_t text_S_x = display.center_x + NESW_RADIUS * normal_x;
// 	uint8_t text_S_y = display.center_y + NESW_RADIUS * normal_y;

// 	uint8_t text_E_x = display.center_x - NESW_RADIUS * normal_x_90;
// 	uint8_t text_E_y = display.center_y - NESW_RADIUS * normal_y_90;	
// 	uint8_t text_W_x = display.center_x + NESW_RADIUS * normal_x_90;
// 	uint8_t text_W_y = display.center_y + NESW_RADIUS * normal_y_90;




// 	canvas[canvasid].fillSmoothCircle(display.center_x, display.center_y, 6, COLOUR_CIRCLE_2);

// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 30, 15, COLOUR_CIRCLE_1);
// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 30,  5, COLOUR_CIRCLE_2);

// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 56,  1, COLOUR_DASH);
// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 69,  1, COLOUR_DASH);

// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 142,  8, COLOUR_CIRCLE_2);
// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 150,  8, COLOUR_CIRCLE_1);

// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 183,  8, COLOUR_CIRCLE_1);
// 	drawCircle(&canvas[canvasid], display.center_x, display.center_y, 191,  8, COLOUR_CIRCLE_2);
	
// 	canvas[canvasid].setFreeFont(&Roboto_Mono_24);
// 	canvas[canvasid].setTextColor(RGB(247, 246, 215));
// 	canvas[canvasid].setTextDatum(CC_DATUM);
// 	canvas[canvasid].drawString("N", text_N_x, text_N_y);
// 	canvas[canvasid].drawString("E", text_E_x, text_E_y);
// 	canvas[canvasid].drawString("S", text_S_x, text_S_y);
// 	canvas[canvasid].drawString("W", text_W_x, text_W_y);

// 	float i_r = 80;
// 	float o_r = i_r + 6;
// 	float count = 32;

// 	for (float i = 0; i < 360; i += 360 / count)
// 	{
// 		float a = heading_in_rad + i * DEG_TO_RAD;
// 		float sa = sin(a);
// 		float ca = cos(a);
// 		canvas[canvasid].drawLine(
// 			display.center_x + -i_r * sa,
// 			display.center_y + i_r * ca,
// 			display.center_x + -o_r * sa,
// 			display.center_y + o_r * ca, COLOUR_DASH
// 		);	
// 	}

// 	int16_t x1 = display.center_x - 80 * normal_x;
// 	int16_t y1 = display.center_y - 80 * normal_y;

// 	int16_t x2 = display.center_x - 20 * normal_x;
// 	int16_t y2 = display.center_y - 20 * normal_y;

// 	int16_t x3 = display.center_x - 17 * normal_x;
// 	int16_t y3 = display.center_y - 17 * normal_y;

// 	int16_t x4 = display.center_x - 20 * normal_x;
// 	int16_t y4 = display.center_y - 20 * normal_y;

// 	canvas[canvasid].fillTriangle(
// 		x1, y1,
// 		x3 - 10 * -normal_x_90,	y3 - 10 * -normal_y_90,
// 		x4,	y4,
// 		COLOUR_BLADE_LIGHT
// 	);

// 	canvas[canvasid].fillTriangle(
// 		x1,	y1,
// 		x3 - 10 * normal_x_90, y3 - 10 * normal_y_90,
// 		x4,	y4,
// 		COLOUR_BLADE_DARK
// 	);

// 	x1 = display.center_x + 80 * normal_x;
// 	y1 = display.center_y + 80 * normal_y;

// 	x2 = display.center_x + 20 * normal_x;
// 	y2 = display.center_y + 20 * normal_y;

// 	x3 = display.center_x + 17 * normal_x;
// 	y3 = display.center_y + 17 * normal_y;

// 	x4 = display.center_x + 20 * normal_x;
// 	y4 = display.center_y + 20 * normal_y;

// 	canvas[canvasid].fillTriangle(
// 		x1, y1,
// 		x3 - 10 * -normal_x_90,	y3 - 10 * -normal_y_90,
// 		x4,	y4,
// 		COLOUR_BLADE_LIGHT
// 	);

// 	canvas[canvasid].fillTriangle(
// 		x1,	y1,
// 		x3 - 10 * normal_x_90, y3 - 10 * normal_y_90,
// 		x4,	y4,
// 		COLOUR_BLADE_DARK
// 	);


// 	x1 = display.center_x - 40 * normal_x_90;
// 	y1 = display.center_y - 40 * normal_y_90;

// 	x2 = display.center_x - 20 * normal_x_90;
// 	y2 = display.center_y - 20 * normal_y_90;

// 	x3 = display.center_x - 17 * normal_x_90;
// 	y3 = display.center_y - 17 * normal_y_90;

// 	x4 = display.center_x - 20 * normal_x_90;
// 	y4 = display.center_y - 20 * normal_y_90;

// 	canvas[canvasid].fillTriangle(
// 		x1, y1,
// 		x3 - 10 * -normal_x, y3 - 10 * -normal_y,
// 		x4,	y4,
// 		COLOUR_BLADE_DARK
// 	);

// 	canvas[canvasid].fillTriangle(
// 		x1,	y1,
// 		x3 - 10 * normal_x, y3 - 10 * normal_y,
// 		x4,	y4,
// 		COLOUR_BLADE_MID
// 	);


// 	x1 = display.center_x + 40 * normal_x_90;
// 	y1 = display.center_y + 40 * normal_y_90;

// 	x2 = display.center_x + 20 * normal_x_90;
// 	y2 = display.center_y + 20 * normal_y_90;

// 	x3 = display.center_x + 17 * normal_x_90;
// 	y3 = display.center_y + 17 * normal_y_90;

// 	x4 = display.center_x + 20 * normal_x_90;
// 	y4 = display.center_y + 20 * normal_y_90;

// 	canvas[canvasid].fillTriangle(
// 		x1, y1,
// 		x3 - 10 * -normal_x, y3 - 10 * -normal_y,
// 		x4,	y4,
// 		COLOUR_BLADE_DARK
// 	);

// 	canvas[canvasid].fillTriangle(
// 		x1,	y1,
// 		x3 - 10 * normal_x, y3 - 10 * normal_y,
// 		x4,	y4,
// 		COLOUR_BLADE_MID
// 	);

// /*
// 	canvas[canvasid].drawCircle(120, 140, 30, TFT_DARKGREY);

// 	canvas[canvasid].setFreeFont(RobotoMono_Regular[12]);
// 	canvas[canvasid].setTextColor(TFT_RED);
// 	canvas[canvasid].setTextDatum(CC_DATUM);
// 	canvas[canvasid].drawString("N", text_N_x, text_N_y);
// 	canvas[canvasid].setTextColor(TFT_WHITE);
// 	canvas[canvasid].drawString("E", text_E_x, text_E_y);
// 	canvas[canvasid].drawString("S", text_S_x, text_S_y);
// 	canvas[canvasid].drawString("W", text_W_x, text_W_y);
// 	canvas[canvasid].setTextColor(TFT_ORANGE);
// 	uint8_t l = canvas[canvasid].drawString(String(heading, 0), display.center_x, 240);
// 	canvas[canvasid].drawSmoothCircle(display.center_x + l * .5 + 7, 236, 3, TFT_ORANGE, TFT_BLACK);

// 	uint8_t needle_N_x = display.center_x - NEEDLE_L * normal_x;
// 	uint8_t needle_N_y = display.center_y - NEEDLE_L * normal_y;
// 	uint8_t needle_S_x = display.center_x + NEEDLE_L * normal_x;
// 	uint8_t needle_S_y = display.center_y + NEEDLE_L * normal_y;

// 	uint8_t needle_E_x = display.center_x - NEEDLE_W * normal_x_90;
// 	uint8_t needle_E_y = display.center_y - NEEDLE_W * normal_y_90;
// 	uint8_t needle_W_x = display.center_x + NEEDLE_W * normal_x_90;
// 	uint8_t needle_W_y = display.center_y + NEEDLE_W * normal_y_90;

// 	canvas[canvasid].fillTriangle(needle_N_x, needle_N_y, needle_E_x, needle_E_y, needle_W_x, needle_W_y, TFT_RED);
// 	canvas[canvasid].fillTriangle(needle_S_x, needle_S_y, needle_E_x, needle_E_y, needle_W_x, needle_W_y, TFT_LIGHTGREY);
// 	canvas[canvasid].fillSmoothCircle(120, 140, 3, TFT_DARKGREY, TFT_LIGHTGREY);*/
// }

/**
 * 
 */
void AppCompass::drawUI()
{
	int16_t x = 0;
	int16_t y = 0;

	switch(running_state)
	{
		case RUNNING_STATE_DRAW:
		{
			x = 0;
			y = 0;

			uint16_t BUTTON_BG_COLOUR = B_BLUE;

			canvas[canvasid].fillRect(x, y, 32, 15, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x, y, 15, 32, BUTTON_BG_COLOUR);
			canvas[canvasid].fillSmoothRoundRect(x, y, 32, 32, 16, BUTTON_BG_COLOUR, BUTTON_BG_COLOUR);

			x += 10;
			y += 10;

			canvas[canvasid].fillRect(x +  3, y + 3, 15, 15, TFT_DARKGREY);

			canvas[canvasid].fillRect(x +  3, y +  3, 2, 2, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x + 16, y +  3, 2, 2, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x +  3, y + 16, 2, 2, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x + 16, y + 16, 2, 2, BUTTON_BG_COLOUR);

			canvas[canvasid].fillRect(x +  6, y +  3, 3, 2, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x + 12, y +  3, 3, 2, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x +  6, y + 16, 3, 2, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x + 12, y + 16, 3, 2, BUTTON_BG_COLOUR);

			canvas[canvasid].fillRect(x +  3, y +  6, 2, 3, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x + 16, y +  6, 2, 3, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x +  3, y + 12, 2, 3, BUTTON_BG_COLOUR);
			canvas[canvasid].fillRect(x + 16, y + 12, 2, 3, BUTTON_BG_COLOUR);

			canvas[canvasid].fillRect(x +  8, y + 8, 5, 5, BUTTON_BG_COLOUR);

			canvas[canvasid].drawPixel(x +  5, y +  3, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 15, y +  3, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  3, y +  5, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  7, y +  5, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 13, y +  5, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 17, y +  5, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  5, y +  7, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 10, y +  7, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 15, y +  7, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  7, y + 10, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 13, y + 10, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  5, y + 13, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 10, y + 13, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 15, y + 13, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  3, y + 15, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  7, y + 15, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 13, y + 15, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 17, y + 15, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x +  5, y + 17, BUTTON_BG_COLOUR);
			canvas[canvasid].drawPixel(x + 15, y + 17, BUTTON_BG_COLOUR);
			break;
		}

		case RUNNING_STATE_CALIBRATE:
		{
			x = 0;
			y = display.height - 32;
			canvas[canvasid].fillSmoothRoundRect(x, y, 32, 32, 16, B_GREEN, B_GREEN);
			canvas[canvasid].fillRect(x, y + 16, 32, 16, B_GREEN);
			canvas[canvasid].fillRect(x, y, 15, 32, B_GREEN);
			canvas[canvasid].setFreeFont(RobotoMono_Regular[10]);
			canvas[canvasid].setTextColor(TFT_GREEN);
			canvas[canvasid].setTextDatum(CC_DATUM);
			canvas[canvasid].drawString("S", x + 18, y + 14);

			x = display.width - 32;
			y = display.height - 32;
			canvas[canvasid].fillSmoothRoundRect(x, y, 32, 32, 16, B_RED, B_RED);
			canvas[canvasid].fillRect(x, y + 16, 32, 16, B_RED);
			canvas[canvasid].fillRect(x + 16, y, 16, 32, B_RED);
			canvas[canvasid].setFreeFont(RobotoMono_Regular[10]);
			canvas[canvasid].setTextColor(TFT_RED);
			canvas[canvasid].setTextDatum(CC_DATUM);
			canvas[canvasid].drawString("X", x + 14, y + 14);
			break;
		}
	}
}


/**
 * 
 */
AppCompass app_compass;