
#include "tw_apps/app_3D.h"
#include "fonts/Clock_Digits.h"
#include "fonts/RobotoMono_Regular_All.h"
#include "peripherals/imu.h"
#include "settings/settings.h"

/**
 * @brief Put anything in here that you want to have run every time the app is opened
 *
 * This is not the same as setup() above that only ever gets called the first time the app opens
 *
 */
void App3D::pre_start() { }

/**
 * @brief Draw the icon that gets shown on the app menu face
 *
 * Icons are 64x64 with rounded corners as per the code below, but the inner content can be anything that represents the app well
 * @param canvasid
 * @param _pos_x
 * @param _pos_y
 * @param style_hint
 */
void App3D::draw_icon(uint8_t canvasid, int16_t _pos_x, int16_t _pos_y, uint8_t style_hint)
{
	if (!is_icon_cached)
	{
		is_icon_cached = true;
		icon_sprite.fillSprite(0);
		icon_sprite.drawSmoothRoundRect(0, 0, 10, 6, icon_width, icon_width, RGB(0xdd, 0x00, 0x00), 0);
		icon_sprite.setTextDatum(4); // Middle, Center
		icon_sprite.setFreeFont(RobotoMono_Regular[15]);
		icon_sprite.setTextColor(TFT_WHITE);
		icon_sprite.drawSmoothCircle(icon_width / 2, icon_height / 2, 20, RGB(0xff, 0x00, 0x00), 0);
		icon_sprite.drawString("3D", icon_width / 2, icon_width / 2);
	}

	icon_x = _pos_x;
	icon_y = _pos_y;
	icon_sprite.pushToSprite(&canvas[canvasid], _pos_x, _pos_y);
}

/**
 * 
 */
bool App3D::process_touch(touch_event_t touch_event)
{
	return false;
}

/**
 * 
 */
void App3D::draw(bool force)
{
	// Override the CPU settings for this app (if needed)
	// setCpuFrequencyMhz(required_cpu_speed);

	if (force || millis() - next_update > update_period)
	{
		next_update = millis();
		draw3D();
		canvas[canvasid].pushSprite(_x, _y);
	}
}
/**
 * 
 */
void App3D::draw3D()
{
	canvas[canvasid].fillSprite(TFT_BLACK);
	//canvas[canvasid].fillRoundRect(0, 0, display.width, display.height, 6, 0);
	//canvas[canvasid].drawRoundRect(0, 0, display.width, display.height, 6, TFT_WHITE);
	
	
	imu.update();
	
	float mag_x = -imu.get_pitch();
	float mag_y = -imu.get_roll();
	float mag_z = -imu.get_yaw();

	
	//imu.get_magnetic(&mag_x, &mag_y, &mag_z, true);	
	

	rotation[0] = mag_x;
	rotation[1] = mag_y;
	rotation[2] = mag_z;

	rotation_matrix.updateRotationMatrix( rotation[0], 180 + rotation[1], rotation[2] );

	for(uint16_t i = 0; i < verticies; i++)
		xyz_cube_verticies_rotated[i] = rotation_matrix * xyz_cube_verticies[i];
	
	for(uint16_t i = 0; i < polygons; i++)
	{
		Polygon polygon = xyz_cube_polygons[i];

		if (polygon.color == 3) // ignore the black parts for now
			continue;
		if (polygon.color == 4) // ignore the white parts for now
			continue;

		Vector3D A = xyz_cube_verticies_rotated[polygon.index[0]];
		Vector3D B = xyz_cube_verticies_rotated[polygon.index[1]];
		Vector3D C = xyz_cube_verticies_rotated[polygon.index[2]];

		float scale_A = palne_scale / (A.z + palne_scale) + 1.3;
		float scale_B = palne_scale / (B.z + palne_scale) + 1.3;
		float scale_C = palne_scale / (C.z + palne_scale) + 1.3;
		
		A *= scale_A;
		B *= scale_B;
		C *= scale_C;

		// back face culling
		if (((A.y - B.y)*(C.x - B.x)) - ((A.x - B.x)*(C.y - B.y)) >= 0 )		
			continue;

		canvas[canvasid].fillTriangle(
			display_center_x + A.x, 
			display_center_y + A.y, 
			display_center_x + B.x,
			display_center_y + B.y, 
			display_center_x + C.x,
			display_center_y + C.y,
			polygon_colors[polygon.color]
		);
	}
	
	icon_sprite.setFreeFont(RobotoMono_Regular[9]);
	canvas[canvasid].drawString(String(rotation[0], 0) + "," + String(rotation[1], 0) + "," + String(rotation[2], 0), display.width / 2, 260);
}

App3D app_3D;
