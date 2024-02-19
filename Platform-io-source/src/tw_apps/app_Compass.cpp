
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
		if (running_state == RUNNING_STATE_DRAW && touch_event.dir == TOUCH_SWIPE_DOWN)
		{
			resetCalibration();
			running_state = RUNNING_STATE_CALIBRATE;
			return true;
		}
		else if (running_state == RUNNING_STATE_CALIBRATE && touch_event.dir == TOUCH_SWIPE_UP)
		{
			settings.config.compass.hard_iron_x = hard_iron_avg[0];
			settings.config.compass.hard_iron_y = hard_iron_avg[1];
			settings.config.compass.hard_iron_z = hard_iron_avg[2];
			settings.save(true);
			info_print(F("Compass hard iron values saved"));			
			running_state = RUNNING_STATE_DRAW;
			return true;
		}
	}
	return false;
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

		imu.update();
		heading = moveTowardsHeading(heading, imu.get_yaw());

		switch(running_state)
		{
			case RUNNING_STATE_DRAW:
			{
				drawCompass(120, 140, heading); // Draw centre of compass at 120,140
				break;
			}

			case RUNNING_STATE_CALIBRATE:
			{
				drawCalibrate();
				break;
			}
		}

		canvas[canvasid].pushSprite(_x, _y);
	}
}

/**
 * 	@brief Move one heading towards another (0.0 > 360.0)
 */
float AppCompass::moveTowardsHeading(float currentHeading, float newHeading)
{
	newHeading = newHeading;

	float result;
	float a = newHeading - currentHeading;
	float b = newHeading < currentHeading
		? newHeading + 360.0 - currentHeading 
		: newHeading - 360.0 - currentHeading
	;
	result = abs(a) < abs(b)
		? currentHeading + a * align_rate
		: currentHeading + b * align_rate
	;

	if (result >= 360.0)
		result -= 360.0;
	if (result < 0.0)
		result += 360.0;

	return result;
}

/**
 * 
 */
void AppCompass::resetCalibration()
{
	for(uint8_t i=0; i < 3; i++)
	{
		hard_iron_avg[i] = 0;
		hard_iron_min[i] = std::numeric_limits<float>::max();
		hard_iron_max[i] = std::numeric_limits<float>::min();
	}
}

/**
 * 
 */
void AppCompass::drawCalibrate()
{
	float mag[3];
	imu.get_magnetic(&mag[0], &mag[1], &mag[2]);

	if (mag[0] < hard_iron_min[0]) hard_iron_min[0] = mag[0];
	if (mag[0] > hard_iron_max[0]) hard_iron_max[0] = mag[0];

	if (mag[1] < hard_iron_min[1]) hard_iron_min[1] = mag[1];
	if (mag[1] > hard_iron_max[1]) hard_iron_max[1] = mag[1];

	if (mag[2] < hard_iron_min[2]) hard_iron_min[2] = mag[2];
	if (mag[2] > hard_iron_max[2]) hard_iron_max[2] = mag[2];

	hard_iron_avg[0] = hard_iron_min[0] + ((hard_iron_max[0] - hard_iron_min[0]) / 2);
	hard_iron_avg[1] = hard_iron_min[1] + ((hard_iron_max[1] - hard_iron_min[1]) / 2);
	hard_iron_avg[2] = hard_iron_min[2] + ((hard_iron_max[2] - hard_iron_min[2]) / 2);

	canvas[canvasid].fillSprite(TFT_TRANSPARENT);

	canvas[canvasid].setFreeFont(RobotoMono_Regular[12]);
	
	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].setTextDatum(CC_DATUM);
	canvas[canvasid].drawString("Calibrating", display.width / 2, 40);
	
	canvas[canvasid].setTextColor(TFT_ORANGE);
	canvas[canvasid].drawString("Hard Iron", display.width / 2, 80);
	
	canvas[canvasid].setTextColor(TFT_RED);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("X:", display.width *.25, 110);
	canvas[canvasid].setTextDatum(CR_DATUM);
	canvas[canvasid].drawString(String(hard_iron_avg[0], 0), display.width *.75, 110);

	canvas[canvasid].setTextColor(TFT_GREEN);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("Y:", display.width *.25, 130);
	canvas[canvasid].setTextDatum(CR_DATUM);
	canvas[canvasid].drawString(String(hard_iron_avg[1], 0), display.width *.75, 130);

	canvas[canvasid].setTextColor(TFT_BLUE);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("Z:", display.width *.25, 150);
	canvas[canvasid].setTextDatum(CR_DATUM);
	canvas[canvasid].drawString(String(hard_iron_avg[2], 0), display.width *.75, 150);

	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].setTextDatum(CC_DATUM);
	canvas[canvasid].drawString("Current", display.width / 2, 190);	

	canvas[canvasid].setTextColor(TFT_ORANGE);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("X:", display.width *.25, 210);
	canvas[canvasid].setTextDatum(CR_DATUM);
	canvas[canvasid].drawString(String(settings.config.compass.hard_iron_x, 0), display.width *.75, 210);

	canvas[canvasid].setTextColor(TFT_ORANGE);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("X:", display.width *.25, 230);
	canvas[canvasid].setTextDatum(CR_DATUM);
	canvas[canvasid].drawString(String(settings.config.compass.hard_iron_y, 0), display.width *.75, 230);

	canvas[canvasid].setTextColor(TFT_ORANGE);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("X:", display.width *.25, 250);
	canvas[canvasid].setTextDatum(CR_DATUM);
	canvas[canvasid].drawString(String(settings.config.compass.hard_iron_z, 0), display.width *.75, 250);
		
	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("/\\", 0, 260);
}

/**
 * 
 */
void AppCompass::drawCompass(int x, int y, float angle)
{
	// TFT_TRANSPARENT is a special colour with reversible 8/16 bit coding
	// this allows it to be used in both 8 and 16 bit colour sprites.
	canvas[canvasid].fillSprite(TFT_TRANSPARENT);

	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].setTextDatum(CL_DATUM);
	canvas[canvasid].drawString("\\/", 0, 20);

	canvas[canvasid].drawCircle(120, 140, 30, TFT_DARKGREY);

	/*
	 Rotate a unit 1 vector pointing down (Cartesian coordinate) [x: 0, y: 1] around the origin at given angle
	 we can reduce the calcs in the rotation matrix due to the x component being 0 and y being 1
	 
	 xp = x * cos(a) - y * sin(a)
	 yp = y * cos(a) + x * sin(a)
	 
	 this recuces down to
	 
	 xp = -y * sin(a)
	 yp =  y * cos(a)
	 
	 the result is a normalized vector that can then be multiplied by a radius, using left hand rule
	*/
	
	// we can multiply these normals with a radius to get all the coords we need
	float normal_x, normal_y, normal_x_90, normal_y_90;
	float angle_rad = angle * DEG_TO_RAD;
	
	// get the rotated up normalised vector
	normal_x = -sin(angle_rad);
	normal_y =  cos(angle_rad);

	// trick that rotates the normal 90 deg for E and W
	normal_x_90 = -normal_y;
	normal_y_90 =  normal_x;

	text_N_x = x - NESW_RADIUS * normal_x;
	text_N_y = y - NESW_RADIUS * normal_y;
	text_S_x = x + NESW_RADIUS * normal_x;
	text_S_y = y + NESW_RADIUS * normal_y;

	text_E_x = x - NESW_RADIUS * normal_x_90 * 0.6;
	text_E_y = y - NESW_RADIUS * normal_y_90 * 0.6;	
	text_W_x = x + NESW_RADIUS * normal_x_90 * 0.6;
	text_W_y = y + NESW_RADIUS * normal_y_90 * 0.6;

	canvas[canvasid].setFreeFont(RobotoMono_Regular[12]);
	canvas[canvasid].setTextColor(TFT_RED);
	canvas[canvasid].setTextDatum(CC_DATUM);
	canvas[canvasid].drawString("N", text_N_x, text_N_y);
	canvas[canvasid].setTextColor(TFT_WHITE);
	canvas[canvasid].drawString("E", text_E_x, text_E_y);
	canvas[canvasid].drawString("S", text_S_x, text_S_y);
	canvas[canvasid].drawString("W", text_W_x, text_W_y);
	canvas[canvasid].setTextColor(TFT_ORANGE);
	canvas[canvasid].drawString(String(angle, 2), display.width / 2, 240);

	needle_N_x = x - NEEDLE_L * normal_x;
	needle_N_y = y - NEEDLE_L * normal_y;
	needle_S_x = x + NEEDLE_L * normal_x;
	needle_S_y = y + NEEDLE_L * normal_y;

	needle_E_x = x - NEEDLE_W * normal_x_90;
	needle_E_y = y - NEEDLE_W * normal_y_90;
	needle_W_x = x + NEEDLE_W * normal_x_90;
	needle_W_y = y + NEEDLE_W * normal_y_90;

	canvas[canvasid].fillTriangle(needle_N_x, needle_N_y, needle_E_x, needle_E_y, needle_W_x, needle_W_y, TFT_RED);
	canvas[canvasid].fillTriangle(needle_S_x, needle_S_y, needle_E_x, needle_E_y, needle_W_x, needle_W_y, TFT_LIGHTGREY);

	canvas[canvasid].fillSmoothCircle(120, 140, 3, TFT_DARKGREY, TFT_LIGHTGREY);
	
	//rotation[0] += 0.2;
	//rotation[1] += 0.6;
	//rotation[2] += 1.2;
	
	rotation[0] = 180 + imu.get_pitch();
	rotation[1] = 180 + imu.get_roll();
	//rotation[2] = imu.get_yaw();

	//if (rotation[0] < 360.0) rotation[0] -= 360;
	//if (rotation[1] < 360.0) rotation[1] -= 360;
	//if (rotation[2] < 360.0) rotation[2] -= 360;

	rotation_matrix.updateRotationMatrix( rotation[0], rotation[1], rotation[2] );
	rotated_square[0] = rotation_matrix * square[0];
	rotated_square[1] = rotation_matrix * square[1];
	rotated_square[2] = rotation_matrix * square[2];
	rotated_square[3] = rotation_matrix * square[3];
	rotated_square[4] = rotation_matrix * square[4];
	rotated_square[5] = rotation_matrix * square[5];
	rotated_square[6] = rotation_matrix * square[6];
	rotated_square[7] = rotation_matrix * square[7];
	

	float s = 120.0;
	float scale_0 = s / (rotated_square[0].z + s);
	float scale_1 = s / (rotated_square[1].z + s);
	float scale_2 = s / (rotated_square[2].z + s);
	float scale_3 = s / (rotated_square[3].z + s);

	float scale_4 = s / (rotated_square[4].z + s);
	float scale_5 = s / (rotated_square[5].z + s);
	float scale_6 = s / (rotated_square[6].z + s);
	float scale_7 = s / (rotated_square[7].z + s);

	canvas[canvasid].fillTriangle(
		x + rotated_square[0].x * scale_0, 
		y + rotated_square[0].y * scale_0, 
		x + rotated_square[1].x * scale_1,
		y + rotated_square[1].y * scale_1, 
		x + rotated_square[3].x * scale_3,
		y + rotated_square[3].y * scale_3,
		TFT_RED
	);
	canvas[canvasid].fillTriangle(
		x + rotated_square[0].x * scale_0,
		y + rotated_square[0].y * scale_0,
		x + rotated_square[3].x * scale_3,
		y + rotated_square[3].y * scale_3,
		x + rotated_square[2].x * scale_2,
		y + rotated_square[2].y * scale_2,
		TFT_PURPLE
	);
	
	
	canvas[canvasid].fillTriangle(
		x + rotated_square[2].x * scale_2,
		y + rotated_square[2].y * scale_2,
		x + rotated_square[3].x * scale_3,
		y + rotated_square[3].y * scale_3,
		x + rotated_square[6].x * scale_6,
		y + rotated_square[6].y * scale_6,
		TFT_GREEN
	);

	canvas[canvasid].fillTriangle(
		x + rotated_square[3].x * scale_3,
		y + rotated_square[3].y * scale_3,
		x + rotated_square[6].x * scale_6,
		y + rotated_square[6].y * scale_6,
		x + rotated_square[7].x * scale_7,
		y + rotated_square[7].y * scale_7,
		TFT_MAGENTA
	);

	canvas[canvasid].fillTriangle(
		x + rotated_square[4].x * scale_4,
		y + rotated_square[4].y * scale_4, 
		x + rotated_square[5].x * scale_5, 
		y + rotated_square[5].y * scale_5, 
		x + rotated_square[6].x * scale_6, 
		y + rotated_square[6].y * scale_6,
		TFT_BLUE
	);
	canvas[canvasid].fillTriangle(
		x + rotated_square[5].x * scale_5,
		y + rotated_square[5].y * scale_5,
		x + rotated_square[6].x * scale_6,
		y + rotated_square[6].y * scale_6,
		x + rotated_square[7].x * scale_7,
		y + rotated_square[7].y * scale_7,
		TFT_ORANGE
	);

	
	canvas[canvasid].drawString(String(rotation[0], 2) + "," + String(rotation[1], 2) + "," + String(rotation[2], 2), display.width / 2, 260);
}

AppCompass app_compass;
