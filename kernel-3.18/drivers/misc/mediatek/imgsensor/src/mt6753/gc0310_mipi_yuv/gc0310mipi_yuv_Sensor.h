
 
#ifndef __GC0310_SENSOR_H
#define __GC0310_SENSOR_H


#define VGA_PERIOD_PIXEL_NUMS						694
#define VGA_PERIOD_LINE_NUMS						488

#define IMAGE_SENSOR_VGA_GRAB_PIXELS			0
#define IMAGE_SENSOR_VGA_GRAB_LINES			0

#define IMAGE_SENSOR_VGA_WIDTH					(640)
#define IMAGE_SENSOR_VGA_HEIGHT					(480)

#define IMAGE_SENSOR_PV_WIDTH					(IMAGE_SENSOR_VGA_WIDTH)
#define IMAGE_SENSOR_PV_HEIGHT					(IMAGE_SENSOR_VGA_HEIGHT)

#define IMAGE_SENSOR_FULL_WIDTH					(IMAGE_SENSOR_VGA_WIDTH)
#define IMAGE_SENSOR_FULL_HEIGHT					(IMAGE_SENSOR_VGA_HEIGHT)

#define GC0310_WRITE_ID							        0x42
#define GC0310_READ_ID								0x43

// GC0310 SENSOR Chip ID: 0xd0

typedef enum
{
	GC0310_RGB_Gamma_m1 = 0,
	GC0310_RGB_Gamma_m2,
	GC0310_RGB_Gamma_m3,
	GC0310_RGB_Gamma_m4,
	GC0310_RGB_Gamma_m5,
	GC0310_RGB_Gamma_m6,
	GC0310_RGB_Gamma_night
}GC0310_GAMMA_TAG;



UINT32 GC0310Open(void);
UINT32 GC0310Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC0310FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 GC0310GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC0310GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 GC0310Close(void);

extern void kdSetI2CSpeed(u16 i2cSpeed);

#endif /* __SENSOR_H */
