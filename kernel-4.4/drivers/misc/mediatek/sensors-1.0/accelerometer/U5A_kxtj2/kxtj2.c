/* Version: KXTJ3-MTK-Android8.0-rc5 2017-12-05 */
/* KXTJ2_1009 motion sensor driver
 *
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
//Added by zhi.chen.hz for significant on 2018.03.05 begin
#include "step_counter.h"
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/pinctrl-state.h>
//Added by zhi.chen.hz for significant on 2018.03.05 end
#include <cust_acc.h>
#include <accel.h>
#include "kxtj2.h"
#include <linux/irq.h>
/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_KXTJ2_1009 150
/*----------------------------------------------------------------------------*/
#define DEBUG 1
/*----------------------------------------------------------------------------*/
//#define CONFIG_KXTJ2_1009_LOWPASS   /*apply low pass filter on output*/       
#define SW_CALIBRATION
//#define USE_EARLY_SUSPEND
/*----------------------------------------------------------------------------*/
#define KXTJ2_1009_AXIS_X          0
#define KXTJ2_1009_AXIS_Y          1
#define KXTJ2_1009_AXIS_Z          2
#define KXTJ2_1009_DATA_LEN        6
#define KXTJ2_1009_DEV_NAME        "KXTJ2_1009"
/*----------------------------------------------------------------------------*/
/***************************SIGNIFICANT***************************/
#define SIGNIFICANT
#ifdef SIGNIFICANT
int kxtj2_set_acc_interrupter(bool flag);
int kxtj2_acc_init_flag = -1;
static void eintWork(struct work_struct *work);
//static bool enable_status = false;
extern bool significant_enable_status;
#endif
/***************************SIGNIFICANT***************************/
/* Maintain  cust info here */
struct acc_hw accel_cust;
static struct acc_hw *hw = &accel_cust;

/* For  driver get cust info */
struct acc_hw *get_cust_acc(void)
{
	return &accel_cust;
}
/*----------------------------------------------------------------------------*/
static const struct i2c_device_id kxtj2_1009_i2c_id[] = {{KXTJ2_1009_DEV_NAME,0},{}};
//static struct i2c_board_info __initdata i2c_kxtj2_1009={ I2C_BOARD_INFO(KXTJ2_1009_DEV_NAME, (KXTJ2_1009_I2C_SLAVE_ADDR>>1))};
/*the adapter id will be available in customization*/
//static unsigned short kxtj2_1009_force[] = {0x00, KXTJ2_1009_I2C_SLAVE_ADDR, I2C_CLIENT_END, I2C_CLIENT_END};
//static const unsigned short *const kxtj2_1009_forces[] = { kxtj2_1009_force, NULL };
//static struct i2c_client_address_data kxtj2_1009_addr_data = { .forces = kxtj2_1009_forces,};

/*----------------------------------------------------------------------------*/
static int kxtj2_1009_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id); 
static int kxtj2_1009_i2c_remove(struct i2c_client *client);
//static int kxtj2_1009_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int kxtj2_1009_suspend(struct device *dev);
static int kxtj2_1009_resume(struct device *dev);

static int kxtj2_1009_local_init(void);
static int  kxtj2_1009_remove(void);
static bool pre_suspend; //adde by yan.gong 
#ifdef CUSTOM_KERNEL_SENSORHUB
static int kxtj2_1009_setup_irq(void);
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB

/*----------------------------------------------------------------------------*/
typedef enum {
    ADX_TRC_FILTER  = 0x01,
    ADX_TRC_RAWDATA = 0x02,
    ADX_TRC_IOCTL   = 0x04,
    ADX_TRC_CALI	= 0X08,
    ADX_TRC_INFO	= 0X10,
} ADX_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};
/*----------------------------------------------------------------------------*/
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*------------------------------------------------------------lock----------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][KXTJ2_1009_AXES_NUM];
    int sum[KXTJ2_1009_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct kxtj2_1009_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;
     //Added by zhi.chen.hz for significant on 2018.03.05 begin
	struct      device_node *irq_node;
	int         irq;
	struct work_struct	eint_work;
       //Added by zhi.chen.hz for significant on 2018.03.05 end
#ifdef CUSTOM_KERNEL_SENSORHUB
    struct work_struct	irq_work;
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB
    
    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
    atomic_t                filter;
    s16                     cali_sw[KXTJ2_1009_AXES_NUM+1];

    /*data*/
    s8                      offset[KXTJ2_1009_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[KXTJ2_1009_AXES_NUM+1];

#ifdef CUSTOM_KERNEL_SENSORHUB
	int 					SCP_init_done;
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB

#if defined(CONFIG_KXTJ2_1009_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif 
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(USE_EARLY_SUSPEND)
    struct early_suspend    early_drv;
#endif
    /* Sensor Who am I ID */
    u8 wai;
};
/*----------------------------------------------------------------------------*/
#ifdef CONFIG_OF
static const struct of_device_id accel_of_match[] = {
	{.compatible = "mediatek,gsensor"},
	{},
};
#endif

#ifdef CONFIG_PM_SLEEP
static const struct dev_pm_ops kxjt2_1009_accel_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(kxtj2_1009_suspend, kxtj2_1009_resume)
};
#endif

static struct i2c_driver kxtj2_1009_i2c_driver = {
	.driver = {
//        .owner          = THIS_MODULE,
		.name           = KXTJ2_1009_DEV_NAME,
#ifdef CONFIG_PM_SLEEP
		.pm = &kxjt2_1009_accel_pm_ops,
#endif
#ifdef CONFIG_OF
		.of_match_table = accel_of_match,
#endif
	},
	.probe		= kxtj2_1009_i2c_probe,
	.remove		= kxtj2_1009_i2c_remove,
//	.detect		= kxtj2_1009_i2c_detect,
	.id_table	= kxtj2_1009_i2c_id,
//	.address_data = &kxtj2_1009_addr_data,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *kxtj2_1009_i2c_client = NULL;
static struct kxtj2_1009_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = true;
static struct GSENSOR_VECTOR3D gsensor_gain;
static char selftestRes[8]= {0}; 
static DEFINE_MUTEX(kxtj2_1009_mutex);
static bool enable_status = false;

static int kxtj2_1009_init_flag =-1; // 0<==>OK -1 <==> fail

static struct acc_init_info kxtj2_1009_init_info = {
		.name = "kxtj2_1009",
		.init = kxtj2_1009_local_init,
		.uninit = kxtj2_1009_remove,
	
};

/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk( GSE_TAG"%s\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk( GSE_TAG fmt, ##args)
/*----------------------------------------------------------------------------*/
static struct data_resolution kxtj2_1009_data_resolution[1] = {
 /* combination by {FULL_RES,RANGE}*/
    {{ 0, 9}, 1024}, // dataformat +/-2g  in 12-bit resolution;  { 3, 9} = 3.9 = (2*2*1000)/(2^12);  256 = (2^12)/(2*2)          
};
/*----------------------------------------------------------------------------*/
static struct data_resolution kxtj2_1009_offset_resolution = {{15, 6}, 64};
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetPowerMode(struct i2c_client *client, bool enable);
/*--------------------KXTJ2_1009 power control function----------------------------------*/
static void KXTJ2_1009_power(struct acc_hw *hw, unsigned int on) 
{
	//static unsigned int power_on = 0;
/*
	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{        
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "KXTJ2_1009"))
			{
				GSE_ERR("kxtj2_1009 power on fails!!\n");
			}
		}
		else	// power off
		{
			if (!hwPowerDown(hw->power_id, "KXTJ2_1009"))
			{
				GSE_ERR("kxtj2_1009 power off fail!!\n");
			}			  
		}
	}
*/
	//power_on = on;    
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetSelfTest_On(struct i2c_client *client, int val)
{
  int err;
  u8  databuf[2];
  struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
  databuf[0] = 0x3A;
  if(val){
    databuf[1] = 0xCA;
  }else{
    databuf[1] = 0x00;
  }
  err = i2c_master_send(obj->client, databuf, 0x2);
  if(err <= 0)
  {
    return KXTJ2_1009_ERR_I2C;
  }
   return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetStPol(struct i2c_client *client, int val)
{
        int err;
        u8  databuf[2];
        bool cur_sensor_power = sensor_power;
        struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
        KXTJ2_1009_SetPowerMode(obj->client, false);
        if(hwmsen_read_block(obj->client, 0x1E, databuf, 0x01))
        {
                GSE_ERR("kxtj2_1009 read Dataformat failt \n");
                return KXTJ2_1009_ERR_I2C;
        }

        if(val)
        {
          databuf[0] |= 0x2;   //ST POL = 1
        }else{
          databuf[0] &= ~0x2;   //ST POL = 0
        }
		databuf[0] = 0x1E;
        databuf[1] = databuf[0];
        
        err = i2c_master_send(obj->client, databuf, 0x2);

        if(err <= 0)
        {
                return KXTJ2_1009_ERR_I2C;
        }
        KXTJ2_1009_SetPowerMode(obj->client, cur_sensor_power/*true*/);

        return 0;
}

/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetDataResolution(struct kxtj2_1009_i2c_data *obj)
{
	int err;
	u8  databuf[2];
	bool cur_sensor_power = sensor_power;

	KXTJ2_1009_SetPowerMode(obj->client, false);

	if(hwmsen_read_block(obj->client, KXTJ2_1009_REG_DATA_RESOLUTION, databuf, 0x01))
	{
		GSE_ERR("kxtj2_1009 read Dataformat failt \n");
		return KXTJ2_1009_ERR_I2C;
	}

	databuf[0] &= ~KXTJ2_1009_RANGE_DATA_RESOLUTION_MASK;
	databuf[0] |= KXTJ2_1009_RANGE_DATA_RESOLUTION_MASK;//12bit
	databuf[1] = databuf[0];
	databuf[0] = KXTJ2_1009_REG_DATA_RESOLUTION;


	err = i2c_master_send(obj->client, databuf, 0x2);

	if(err <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}

	KXTJ2_1009_SetPowerMode(obj->client, cur_sensor_power/*true*/);

	//kxtj2_1009_data_resolution[0] has been set when initialize: +/-2g  in 8-bit resolution:  15.6 mg/LSB*/   
	obj->reso = &kxtj2_1009_data_resolution[0];

	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ReadData(struct i2c_client *client, s16 data[KXTJ2_1009_AXES_NUM])
{
	struct kxtj2_1009_i2c_data *priv = i2c_get_clientdata(client);        
    int err = 0;
#if 0//ifdef CUSTOM_KERNEL_SENSORHUB
    SCP_SENSOR_HUB_DATA req;
    int len;
#else//#ifdef CUSTOM_KERNEL_SENSORHUB
	u8 addr = KXTJ2_1009_REG_DATAX0;
	u8 buf[KXTJ2_1009_DATA_LEN] = {0};
	int i;
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB

#if 0//ifdef CUSTOM_KERNEL_SENSORHUB
    req.get_data_req.sensorType = ID_ACCELEROMETER;
    req.get_data_req.action = SENSOR_HUB_GET_DATA;
    len = sizeof(req.get_data_req);
    err = SCP_sensorHub_req_send(&req, &len, 1);
    if (err)
    {
        GSE_ERR("kxtj2_1009 SCP_sensorHub_req_send!\n");
        return err;
    }

    if (ID_ACCELEROMETER != req.get_data_rsp.sensorType ||
        SENSOR_HUB_GET_DATA != req.get_data_rsp.action ||
        0 != req.get_data_rsp.errCode)
    {
        GSE_ERR("kxtj2_1009 error : %d\n", req.get_data_rsp.errCode);
        return req.get_data_rsp.errCode;
    }

    len -= offsetof(SCP_SENSOR_HUB_GET_DATA_RSP, int8_Data);

    if (6 == len)
    {
        data[KXTJ2_1009_AXIS_X] = req.get_data_rsp.int16_Data[0];
        data[KXTJ2_1009_AXIS_Y] = req.get_data_rsp.int16_Data[1];
        data[KXTJ2_1009_AXIS_Z] = req.get_data_rsp.int16_Data[2];
    }
    else if (3 == len)
    {
        data[KXTJ2_1009_AXIS_X] = req.get_data_rsp.int8_Data[0];
        data[KXTJ2_1009_AXIS_Y] = req.get_data_rsp.int8_Data[1];
        data[KXTJ2_1009_AXIS_Z] = req.get_data_rsp.int8_Data[2];
    }
    else
    {
        GSE_ERR("kxtj2_1009 data length fail : %d\n", len);
    }

    if(atomic_read(&priv->trace) & ADX_TRC_RAWDATA)
	{
        //show data
	}
#else//#ifdef CUSTOM_KERNEL_SENSORHUB
	if(NULL == client)
	{
		err = -EINVAL;
	}
	else if((err = hwmsen_read_block(client, addr, buf, 0x06)) != 0)
	{
		GSE_ERR("kxtj2_1009 error: %d\n", err);
	}
	else
	{
		data[KXTJ2_1009_AXIS_X] = (s16)((buf[KXTJ2_1009_AXIS_X*2] >> 4) |
		         (buf[KXTJ2_1009_AXIS_X*2+1] << 4));
		data[KXTJ2_1009_AXIS_Y] = (s16)((buf[KXTJ2_1009_AXIS_Y*2] >> 4) |
		         (buf[KXTJ2_1009_AXIS_Y*2+1] << 4));
		data[KXTJ2_1009_AXIS_Z] = (s16)((buf[KXTJ2_1009_AXIS_Z*2] >> 4) |
		         (buf[KXTJ2_1009_AXIS_Z*2+1] << 4));

		for(i=0;i<3;i++)				
		{								//because the data is store in binary complement number formation in computer system
			if ( data[i] == 0x0800 )	//so we want to calculate actual number here
				data[i]= -2048;			//10bit resolution, 512= 2^(12-1)
			else if ( data[i] & 0x0800 )//transfor format
			{							
				data[i] -= 0x1; 		
				data[i] = ~data[i]; 	
				data[i] &= 0x07ff;		
				data[i] = -data[i]; 	
			}
		}	


		if(atomic_read(&priv->trace) & ADX_TRC_RAWDATA)
		{
			GSE_LOG("[%08X %08X %08X] => [%5d %5d %5d]\n", data[KXTJ2_1009_AXIS_X], data[KXTJ2_1009_AXIS_Y], data[KXTJ2_1009_AXIS_Z],
		                               data[KXTJ2_1009_AXIS_X], data[KXTJ2_1009_AXIS_Y], data[KXTJ2_1009_AXIS_Z]);
		}
#ifdef CONFIG_KXTJ2_1009_LOWPASS
		if(atomic_read(&priv->filter))
		{
			if(atomic_read(&priv->fir_en) && !atomic_read(&priv->suspend))
			{
				int idx, firlen = atomic_read(&priv->firlen);   
				if(priv->fir.num < firlen)
				{                
					priv->fir.raw[priv->fir.num][KXTJ2_1009_AXIS_X] = data[KXTJ2_1009_AXIS_X];
					priv->fir.raw[priv->fir.num][KXTJ2_1009_AXIS_Y] = data[KXTJ2_1009_AXIS_Y];
					priv->fir.raw[priv->fir.num][KXTJ2_1009_AXIS_Z] = data[KXTJ2_1009_AXIS_Z];
					priv->fir.sum[KXTJ2_1009_AXIS_X] += data[KXTJ2_1009_AXIS_X];
					priv->fir.sum[KXTJ2_1009_AXIS_Y] += data[KXTJ2_1009IK_AXIS_Y];
					priv->fir.sum[KXTJ2_1009_AXIS_Z] += data[KXTJ2_1009_AXIS_Z];
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d]\n", priv->fir.num,
							priv->fir.raw[priv->fir.num][KXTJ2_1009_AXIS_X], priv->fir.raw[priv->fir.num][KXTJ2_1009_AXIS_Y], priv->fir.raw[priv->fir.num][KXTJ2_1009_AXIS_Z],
							priv->fir.sum[KXTJ2_1009_AXIS_X], priv->fir.sum[KXTJ2_1009_AXIS_Y], priv->fir.sum[KXTJ2_1009_AXIS_Z]);
					}
					priv->fir.num++;
					priv->fir.idx++;
				}
				else
				{
					idx = priv->fir.idx % firlen;
					priv->fir.sum[KXTJ2_1009_AXIS_X] -= priv->fir.raw[idx][KXTJ2_1009_AXIS_X];
					priv->fir.sum[KXTJ2_1009_AXIS_Y] -= priv->fir.raw[idx][KXTJ2_1009_AXIS_Y];
					priv->fir.sum[KXTJ2_1009_AXIS_Z] -= priv->fir.raw[idx][KXTJ2_1009_AXIS_Z];
					priv->fir.raw[idx][KXTJ2_1009_AXIS_X] = data[KXTJ2_1009_AXIS_X];
					priv->fir.raw[idx][KXTJ2_1009_AXIS_Y] = data[KXTJ2_1009_AXIS_Y];
					priv->fir.raw[idx][KXTJ2_1009_AXIS_Z] = data[KXTJ2_1009_AXIS_Z];
					priv->fir.sum[KXTJ2_1009_AXIS_X] += data[KXTJ2_1009_AXIS_X];
					priv->fir.sum[KXTJ2_1009_AXIS_Y] += data[KXTJ2_1009_AXIS_Y];
					priv->fir.sum[KXTJ2_1009_AXIS_Z] += data[KXTJ2_1009_AXIS_Z];
					priv->fir.idx++;
					data[KXTJ2_1009_AXIS_X] = priv->fir.sum[KXTJ2_1009_AXIS_X]/firlen;
					data[KXTJ2_1009_AXIS_Y] = priv->fir.sum[KXTJ2_1009_AXIS_Y]/firlen;
					data[KXTJ2_1009_AXIS_Z] = priv->fir.sum[KXTJ2_1009_AXIS_Z]/firlen;
					if(atomic_read(&priv->trace) & ADX_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d] : [%5d %5d %5d]\n", idx,
						priv->fir.raw[idx][KXTJ2_1009_AXIS_X], priv->fir.raw[idx][KXTJ2_1009_AXIS_Y], priv->fir.raw[idx][KXTJ2_1009_AXIS_Z],
						priv->fir.sum[KXTJ2_1009_AXIS_X], priv->fir.sum[KXTJ2_1009_AXIS_Y], priv->fir.sum[KXTJ2_1009_AXIS_Z],
						data[KXTJ2_1009_AXIS_X], data[KXTJ2_1009_AXIS_Y], data[KXTJ2_1009_AXIS_Z]);
					}
				}
			}
		}	
#endif         
	}
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB
	return err;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ReadOffset(struct i2c_client *client, s8 ofs[KXTJ2_1009_AXES_NUM])
{    
	int err = 0;

	ofs[1]=ofs[2]=ofs[0]=0x00;

	GSE_ERR("kxtj2_1009 offesx=%x, y=%x, z=%x",ofs[0],ofs[1],ofs[2]);
	
	return err;    
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ResetCalibration(struct i2c_client *client)
{
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
	int err = 0;
#ifdef CUSTOM_KERNEL_SENSORHUB
    SCP_SENSOR_HUB_DATA data;
    KXTJ2_1009_CUST_DATA *pCustData;
    unsigned int len;

	if(0 != obj->SCP_init_done)
	{
	    pCustData = (KXTJ2_1009_CUST_DATA *)&data.set_cust_req.custData;

	    data.set_cust_req.sensorType = ID_ACCELEROMETER;
	    data.set_cust_req.action = SENSOR_HUB_SET_CUST;
	    pCustData->resetCali.action = KXTJ2_1009_CUST_ACTION_RESET_CALI;
	    len = offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + sizeof(pCustData->resetCali);
	    SCP_sensorHub_req_send(&data, &len, 1);
	}
#endif

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	memset(obj->offset, 0x00, sizeof(obj->offset));
	return err;    
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ReadCalibration(struct i2c_client *client, int dat[KXTJ2_1009_AXES_NUM])
{
    struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
    int mul;

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
	    if ((err = KXTJ2_1009_ReadOffset(client, obj->offset))) {
        GSE_ERR("kxtj2_1009 read offset fail, %d\n", err);
        return err;
    	}    
    	mul = obj->reso->sensitivity/kxtj2_1009_offset_resolution.sensitivity;
	#endif

    dat[obj->cvt.map[KXTJ2_1009_AXIS_X]] = obj->cvt.sign[KXTJ2_1009_AXIS_X]*(obj->offset[KXTJ2_1009_AXIS_X]*mul + obj->cali_sw[KXTJ2_1009_AXIS_X]);
    dat[obj->cvt.map[KXTJ2_1009_AXIS_Y]] = obj->cvt.sign[KXTJ2_1009_AXIS_Y]*(obj->offset[KXTJ2_1009_AXIS_Y]*mul + obj->cali_sw[KXTJ2_1009_AXIS_Y]);
    dat[obj->cvt.map[KXTJ2_1009_AXIS_Z]] = obj->cvt.sign[KXTJ2_1009_AXIS_Z]*(obj->offset[KXTJ2_1009_AXIS_Z]*mul + obj->cali_sw[KXTJ2_1009_AXIS_Z]);                        
                                       
    return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ReadCalibrationEx(struct i2c_client *client, int act[KXTJ2_1009_AXES_NUM], int raw[KXTJ2_1009_AXES_NUM])
{  
	/*raw: the raw calibration data; act: the actual calibration data*/
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
    #ifdef SW_CALIBRATION
    #else
	int err;
    #endif
	int mul;

 

	#ifdef SW_CALIBRATION
		mul = 0;//only SW Calibration, disable HW Calibration
	#else
		if(err = KXTJ2_1009_ReadOffset(client, obj->offset))
		{
			GSE_ERR("kxtj2_1009 read offset fail, %d\n", err);
			return err;
		}   
		mul = obj->reso->sensitivity/kxtj2_1009_offset_resolution.sensitivity;
	#endif
	
	raw[KXTJ2_1009_AXIS_X] = obj->offset[KXTJ2_1009_AXIS_X]*mul + obj->cali_sw[KXTJ2_1009_AXIS_X];
	raw[KXTJ2_1009_AXIS_Y] = obj->offset[KXTJ2_1009_AXIS_Y]*mul + obj->cali_sw[KXTJ2_1009_AXIS_Y];
	raw[KXTJ2_1009_AXIS_Z] = obj->offset[KXTJ2_1009_AXIS_Z]*mul + obj->cali_sw[KXTJ2_1009_AXIS_Z];

	act[obj->cvt.map[KXTJ2_1009_AXIS_X]] = obj->cvt.sign[KXTJ2_1009_AXIS_X]*raw[KXTJ2_1009_AXIS_X];
	act[obj->cvt.map[KXTJ2_1009_AXIS_Y]] = obj->cvt.sign[KXTJ2_1009_AXIS_Y]*raw[KXTJ2_1009_AXIS_Y];
	act[obj->cvt.map[KXTJ2_1009_AXIS_Z]] = obj->cvt.sign[KXTJ2_1009_AXIS_Z]*raw[KXTJ2_1009_AXIS_Z];                        
	                       
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_WriteCalibration(struct i2c_client *client, int dat[KXTJ2_1009_AXES_NUM])
{
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
	int err;
	int cali[KXTJ2_1009_AXES_NUM], raw[KXTJ2_1009_AXES_NUM];
#ifdef CUSTOM_KERNEL_SENSORHUB
    SCP_SENSOR_HUB_DATA data;
    KXTJ2_1009_CUST_DATA *pCustData;
    unsigned int len;
#endif
#ifdef SW_CALIBRATION
#else
	int lsb = kxtj2_1009_offset_resolution.sensitivity;
	int divisor = obj->reso->sensitivity/lsb;
#endif

	if(0 != (err = KXTJ2_1009_ReadCalibrationEx(client, cali, raw)))	/*offset will be updated in obj->offset*/
	{ 
		GSE_ERR("kxtj2_1009 read offset fail, %d\n", err);
		return err;
	}

	GSE_LOG("OLDOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		raw[KXTJ2_1009_AXIS_X], raw[KXTJ2_1009_AXIS_Y], raw[KXTJ2_1009_AXIS_Z],
		obj->offset[KXTJ2_1009_AXIS_X], obj->offset[KXTJ2_1009_AXIS_Y], obj->offset[KXTJ2_1009_AXIS_Z],
		obj->cali_sw[KXTJ2_1009_AXIS_X], obj->cali_sw[KXTJ2_1009_AXIS_Y], obj->cali_sw[KXTJ2_1009_AXIS_Z]);

#ifdef CUSTOM_KERNEL_SENSORHUB
    pCustData = (KXTJ2_1009_CUST_DATA *)data.set_cust_req.custData;
    data.set_cust_req.sensorType = ID_ACCELEROMETER;
    data.set_cust_req.action = SENSOR_HUB_SET_CUST;
    pCustData->setCali.action = KXTJ2_1009_CUST_ACTION_SET_CALI;
    pCustData->setCali.data[KXTJ2_1009_AXIS_X] = dat[KXTJ2_1009_AXIS_X];
    pCustData->setCali.data[KXTJ2_1009_AXIS_Y] = dat[KXTJ2_1009_AXIS_Y];
    pCustData->setCali.data[KXTJ2_1009_AXIS_Z] = dat[KXTJ2_1009_AXIS_Z];
    len = offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + sizeof(pCustData->setCali);
    SCP_sensorHub_req_send(&data, &len, 1);
#endif

	/*calculate the real offset expected by caller*/
	cali[KXTJ2_1009_AXIS_X] += dat[KXTJ2_1009_AXIS_X];
	cali[KXTJ2_1009_AXIS_Y] += dat[KXTJ2_1009_AXIS_Y];
	cali[KXTJ2_1009_AXIS_Z] += dat[KXTJ2_1009_AXIS_Z];

	GSE_LOG("UPDATE: (%+3d %+3d %+3d)\n", 
		dat[KXTJ2_1009_AXIS_X], dat[KXTJ2_1009_AXIS_Y], dat[KXTJ2_1009_AXIS_Z]);

#ifdef SW_CALIBRATION
	obj->cali_sw[KXTJ2_1009_AXIS_X] = obj->cvt.sign[KXTJ2_1009_AXIS_X]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_X]]);
	obj->cali_sw[KXTJ2_1009_AXIS_Y] = obj->cvt.sign[KXTJ2_1009_AXIS_Y]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_Y]]);
	obj->cali_sw[KXTJ2_1009_AXIS_Z] = obj->cvt.sign[KXTJ2_1009_AXIS_Z]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_Z]]);	
#else
	obj->offset[KXTJ2_1009_AXIS_X] = (s8)(obj->cvt.sign[KXTJ2_1009_AXIS_X]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_X]])/(divisor));
	obj->offset[KXTJ2_1009_AXIS_Y] = (s8)(obj->cvt.sign[KXTJ2_1009_AXIS_Y]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_Y]])/(divisor));
	obj->offset[KXTJ2_1009_AXIS_Z] = (s8)(obj->cvt.sign[KXTJ2_1009_AXIS_Z]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_Z]])/(divisor));

	/*convert software calibration using standard calibration*/
	obj->cali_sw[KXTJ2_1009_AXIS_X] = obj->cvt.sign[KXTJ2_1009_AXIS_X]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_X]])%(divisor);
	obj->cali_sw[KXTJ2_1009_AXIS_Y] = obj->cvt.sign[KXTJ2_1009_AXIS_Y]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_Y]])%(divisor);
	obj->cali_sw[KXTJ2_1009_AXIS_Z] = obj->cvt.sign[KXTJ2_1009_AXIS_Z]*(cali[obj->cvt.map[KXTJ2_1009_AXIS_Z]])%(divisor);

	GSE_LOG("NEWOFF: (%+3d %+3d %+3d): (%+3d %+3d %+3d) / (%+3d %+3d %+3d)\n", 
		obj->offset[KXTJ2_1009_AXIS_X]*divisor + obj->cali_sw[KXTJ2_1009_AXIS_X], 
		obj->offset[KXTJ2_1009_AXIS_Y]*divisor + obj->cali_sw[KXTJ2_1009_AXIS_Y], 
		obj->offset[KXTJ2_1009_AXIS_Z]*divisor + obj->cali_sw[KXTJ2_1009_AXIS_Z], 
		obj->offset[KXTJ2_1009_AXIS_X], obj->offset[KXTJ2_1009_AXIS_Y], obj->offset[KXTJ2_1009_AXIS_Z],
		obj->cali_sw[KXTJ2_1009_AXIS_X], obj->cali_sw[KXTJ2_1009_AXIS_Y], obj->cali_sw[KXTJ2_1009_AXIS_Z]);

	if(err = hwmsen_write_block(obj->client, KXTJ2_1009_REG_OFSX, obj->offset, KXTJ2_1009_AXES_NUM))
	{
		GSE_ERR("kxtj2_1009 write offset fail: %d\n", err);
		return err;
	}
#endif

	return err;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_CheckDeviceID(struct i2c_client *client)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = KXTJ2_1009_REG_DEVID;   

	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		goto exit_KXTJ2_1009_CheckDeviceID;
	}
	
	udelay(500);

	databuf[0] = 0x0;        
	res = i2c_master_recv(client, databuf, 0x01);
	if(res <= 0)
	{
		goto exit_KXTJ2_1009_CheckDeviceID;
	}
	
	obj_i2c_data->wai = databuf[0];

	if(false)
	{
		GSE_ERR("KXTJ2_1009_CheckDeviceID 0x%x failt!\n ", databuf[0]);
		return KXTJ2_1009_ERR_IDENTIFICATION;
	}
	else
	{
		GSE_ERR("KXTJ2_1009_CheckDeviceID 0x%x pass!\n ", databuf[0]);
	}
	
	exit_KXTJ2_1009_CheckDeviceID:
	if (res <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}
	
	return KXTJ2_1009_SUCCESS;
}
/*----------------------------------------------------------------------------*/
#ifdef CUSTOM_KERNEL_SENSORHUB
static int KXTJ2_1009_SCP_SetPowerMode(bool enable)
{
	int res = 0;
    SCP_SENSOR_HUB_DATA req;
    int len;
	
	if(enable == sensor_power)
	{
		GSE_LOG("Sensor power status is newest!\n");
		return KXTJ2_1009_SUCCESS;
	}

    req.activate_req.sensorType = ID_ACCELEROMETER;
    req.activate_req.action = SENSOR_HUB_ACTIVATE;
    req.activate_req.enable = enable;
    len = sizeof(req.activate_req);
    res = SCP_sensorHub_req_send(&req, &len, 1);
    if (res)
    {
        GSE_ERR("kxtj2_1009 SCP_sensorHub_req_send!\n");
        return res;
    }

	//GSE_LOG("KXTJ2_1009_SetPowerMode %d!\n ",enable);


	sensor_power = enable;

	mdelay(5);
	
	return KXTJ2_1009_SUCCESS;    
}
#endif
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetPowerMode(struct i2c_client *client, bool enable)
{
	int res = 0;
    u8 databuf[2];
	u8 addr = KXTJ2_1009_REG_POWER_CTL;
	
	if(enable == sensor_power)
	{
		GSE_LOG("Sensor power status is newest!\n");
		return KXTJ2_1009_SUCCESS;
	}

	if(hwmsen_read_block(client, addr, databuf, 0x01))
	{
		GSE_ERR("kxtj2_1009 read power ctl register err!\n");
		return KXTJ2_1009_ERR_I2C;
	}

	
	if(enable == true)
	{
		databuf[0] |= KXTJ2_1009_MEASURE_MODE;
	}
	else
	{
		databuf[0] &= ~KXTJ2_1009_MEASURE_MODE;
	}
	databuf[1] = databuf[0];
	databuf[0] = KXTJ2_1009_REG_POWER_CTL;
	

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}

	//GSE_LOG("KXTJ2_1009_SetPowerMode %d!\n ",enable);


	sensor_power = enable;

	mdelay(5);
	
	return KXTJ2_1009_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
	u8 databuf[10];    
	int res = 0;
	bool cur_sensor_power = sensor_power;

	memset(databuf, 0, sizeof(u8)*10);  

	KXTJ2_1009_SetPowerMode(client, false);

	if(hwmsen_read_block(client, KXTJ2_1009_REG_DATA_FORMAT, databuf, 0x01))
	{
		GSE_ERR("kxtj2_1009 read Dataformat failt \n");
		return KXTJ2_1009_ERR_I2C;
	}

	databuf[0] &= ~KXTJ2_1009_RANGE_MASK;
	databuf[0] |= dataformat;
	databuf[1] = databuf[0];
	databuf[0] = KXTJ2_1009_REG_DATA_FORMAT;


	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}

	KXTJ2_1009_SetPowerMode(client, cur_sensor_power/*true*/);
	
	//GSE_ERR("KXTJ2_1009_SetDataFormat OK! \n");
	

	return KXTJ2_1009_SetDataResolution(obj);    
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetBWRate(struct i2c_client *client, u8 bwrate)
{
	u8 databuf[10];    
	int res = 0;
	bool cur_sensor_power = sensor_power;

	memset(databuf, 0, sizeof(u8)*10);    

	
	KXTJ2_1009_SetPowerMode(client, false);

	if(hwmsen_read_block(client, KXTJ2_1009_REG_BW_RATE, databuf, 0x01))
	{
		GSE_ERR("kxtj2_1009 read rate failt \n");
		return KXTJ2_1009_ERR_I2C;
	}

	databuf[0] &= 0xf0;
	databuf[0] |= bwrate;
	databuf[1] = databuf[0];
	databuf[0] = KXTJ2_1009_REG_BW_RATE;


	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}

	
	KXTJ2_1009_SetPowerMode(client, cur_sensor_power/*true*/);
	//GSE_ERR("KXTJ2_1009_SetBWRate OK! \n");
	
	return KXTJ2_1009_SUCCESS;    
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_SetIntEnable(struct i2c_client *client, bool enable)
{
	u8 databuf[10];    
	int res = 0;

	memset(databuf, 0, sizeof(u8)*10);    
	databuf[0] = KXTJ2_1009_REG_INT_ENABLE;  
        
        if(enable)
        {
             databuf[1] = 0x30;
        }
        else 
        {
            databuf[1] = 0x10;
        }

	res = i2c_master_send(client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}
	
	return KXTJ2_1009_SUCCESS;    
}

#if 1
//Added by yuduan.xie for significant on 2017.2.27 begin
static int KXTJ2_1009_setMotionDetectFunc(struct i2c_client *client,bool flag)
{
         u8 databuf[10];

	int res;
         if (flag == false)
         {
            return 0;
         }

        memset(databuf, 0, sizeof(u8)*10); 
	databuf[0] = 0x1B;
        databuf[1] = 0xc2;
        res = i2c_master_send(client, databuf, 0x2); 
	//GSE_ERR("gongyan1 :%d\n",res);

	memset(databuf, 0, sizeof(u8)*10); 
	databuf[0] = 0x1D;    
	databuf[1] = 0x06;
	res = i2c_master_send(client, databuf, 0x2); 
	//GSE_ERR("gongyan2 :%d\n",res);

	memset(databuf, 0, sizeof(u8)*10); 
	databuf[0] = 0x1E;    
	databuf[1] = 0x30;
	res = i2c_master_send(client, databuf, 0x2); 
	// GSE_ERR("gongyan3 :%d\n",res);

	memset(databuf, 0, sizeof(u8)*10); 
	databuf[0] = 0x21;    
	databuf[1] = 0x02;
	res = i2c_master_send(client, databuf, 0x2); 
	//GSE_ERR("gongyan4 :%d\n",res);

	memset(databuf, 0, sizeof(u8)*10); 
	databuf[0] = 0x29;    
	databuf[1] = 0x02;
	res = i2c_master_send(client, databuf, 0x2); 
	// GSE_ERR("gongyan5 :%d\n",res);

	memset(databuf, 0, sizeof(u8)*10); 
	databuf[0] = 0x6A;    
	databuf[1] = 0x15;
	res = i2c_master_send(client, databuf, 0x2); 	
    //    GSE_ERR("gongyan6 :%d\n",res);
        //KXTJ2_1009_SetPowerMode(client, true);

	if(res <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}

	return KXTJ2_1009_SUCCESS;    
}
//Modified by yuduan.xie for significant on 2017.2.27 end
#endif
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_init_client(struct i2c_client *client, int reset_cali)
{
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;

	res = KXTJ2_1009_CheckDeviceID(client); 
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}	
  //  GSE_ERR("KXTJ2_1009_CheckDeviceID OK!\n");
  //	GSE_LOG("KXTJ2_1009_CheckDeviceID OK! %d!\n ",res);
	res = KXTJ2_1009_SetPowerMode(client, enable_status/*false*/);
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}
	
  //  GSE_ERR("KXTJ2_1009_SetPowerMode OK!\n");
  //	GSE_LOG("KXTJ2_1009_SetPowerMode OK! %d!\n ",res);
	res = KXTJ2_1009_SetBWRate(client, KXTJ2_1009_BW_100HZ);
	if(res != KXTJ2_1009_SUCCESS ) //0x2C->BW=100Hz
	{
		return res;
	}
  //  GSE_ERR("KXTJ2_1009_SetBWRate OK!\n");
  //	GSE_LOG("KXTJ2_1009_SetBWRate OK! %d!\n ",res);
	res = KXTJ2_1009_SetDataFormat(client, KXTJ2_1009_RANGE_2G);
	if(res != KXTJ2_1009_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
  //  GSE_ERR("KXTJ2_1009_SetDataFormat OK!\n");
  //	GSE_LOG("KXTJ2_1009_SetDataFormat OK! %d!\n ",res);
	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = obj->reso->sensitivity;

#ifdef CUSTOM_KERNEL_SENSORHUB
    res = kxtj2_1009_setup_irq();
    if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB
#if 1

	//Added by yuduan.xie for significant on 2017.2.27 begin
       res = KXTJ2_1009_SetIntEnable(client,false);
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}
	//Added by yuduan.xie for significant on 2017.2.27 end

#endif
	//GSE_LOG("KXTJ2_1009_SetBWRate OK! %d!\n ",res);
	if(0 != reset_cali)
	{ 
		/*reset calibration only in power on*/
		res = KXTJ2_1009_ResetCalibration(client);
		if(res != KXTJ2_1009_SUCCESS)
		{
			return res;
		}
		GSE_LOG("kxtj2_1009_init_client OK! %d!\n ",KXTJ2_1009_SUCCESS);
	}
	//GSE_ERR("kxtj2_1009_init_client OK!\n");
	//GSE_LOG("kxtj2_1009_init_client OK! %d!\n ",reset_cali);
#ifdef CONFIG_KXTJ2_1009_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));  
#endif
    //GSE_LOG("kxtj2_1009_init_client OK! %d!\n ",KXTJ2_1009_SUCCESS);
	return KXTJ2_1009_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];    

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}
	
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	if (obj_i2c_data->wai == KXTJ3_WHO_AM_I_ID) {
		sprintf(buf, "KXTJ3 Chip");
	}
	else {
		sprintf(buf, "KXTJ2_1009 Chip");
	}

	return 0;
}

/*Kionix Auto-Cali Start*/
#define KIONIX_AUTO_CAL     //Setup AUTO-Cali parameter
#ifdef KIONIX_AUTO_CAL
//#define DEBUG_MSG_CAL
#define Sensitivity_def      1024	//	
#define Detection_range   200 	// Follow KXTJ2 SPEC Offset Range define
#define Stable_range        50     	// Stable iteration
#define BUF_RANGE_Limit 10 	
static int BUF_RANGE = BUF_RANGE_Limit;			
static int temp_zbuf[50]={0};
static int temp_zsum = 0; // 1024 * BUF_RANGE ;
static int Z_AVG[2] = {Sensitivity_def,Sensitivity_def} ;
static int Wave_Max,Wave_Min;
#endif
/*Kionix Auto-Cali End*/

/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
	struct kxtj2_1009_i2c_data *obj = (struct kxtj2_1009_i2c_data*)i2c_get_clientdata(client);
	u8 databuf[20];
	int acc[KXTJ2_1009_AXES_NUM];
	int res = 0;
/*Kionix Auto-Cali Start*/
#ifdef KIONIX_AUTO_CAL
    s16 raw[3];
    int k;
#endif    
/*Kionix Auto-Cali End*/

	memset(databuf, 0, sizeof(u8)*10);

	if(NULL == buf)
	{
		return -1;
	}
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	if (atomic_read(&obj->suspend))
	{
		return 0;
	}
	/*if(sensor_power == FALSE)
	{
		res = KXTJ2_1009_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on kxtj2_1009 error %d!\n", res);
		}
	}*/

	if(0 != (res = KXTJ2_1009_ReadData(client, obj->data)))
	{        
		GSE_ERR("kxtj2_1009 I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
#if 0//ifdef CUSTOM_KERNEL_SENSORHUB
        acc[KXTJ2_1009_AXIS_X] = obj->data[KXTJ2_1009_AXIS_X];
		acc[KXTJ2_1009_AXIS_Y] = obj->data[KXTJ2_1009_AXIS_Y];
		acc[KXTJ2_1009_AXIS_Z] = obj->data[KXTJ2_1009_AXIS_Z];		
#else

/*Kionix Auto-Cali Start*/
#ifdef KIONIX_AUTO_CAL
        raw[0]=obj->data[KXTJ2_1009_AXIS_X];
        raw[1]=obj->data[KXTJ2_1009_AXIS_Y];
        raw[2]=obj->data[KXTJ2_1009_AXIS_Z];

        if(     (abs(raw[0]) < Detection_range)  
            &&  (abs(raw[1]) < Detection_range) 
            &&  (abs((abs(raw[2])- Sensitivity_def))  < ((Detection_range)+ 308)))
        {

            #ifdef DEBUG_MSG_CAL
				GSE_ERR("+++KXTJ2 Calibration Raw Data,%d,%d,%d\n",raw[0],raw[1],raw[2]);
            #endif
            temp_zsum = 0;
            Wave_Max =-4095;
            Wave_Min = 4095;
            
            
            if ( BUF_RANGE > BUF_RANGE_Limit ) BUF_RANGE = BUF_RANGE_Limit; 
                           
            for (k=0; k < BUF_RANGE-1; k++) {
                temp_zbuf[k] = temp_zbuf[k+1];
                if (temp_zbuf[k] == 0) temp_zbuf[k] = Sensitivity_def ;
                temp_zsum += temp_zbuf[k];
                if (temp_zbuf[k] > Wave_Max) Wave_Max = temp_zbuf[k];
                if (temp_zbuf[k] < Wave_Min) Wave_Min = temp_zbuf[k];
            }

            temp_zbuf[k] = raw[2]; // k=BUF_RANGE-1, update Z raw to bubber
            temp_zsum += temp_zbuf[k];
            if (temp_zbuf[k] > Wave_Max) Wave_Max = temp_zbuf[k];
            if (temp_zbuf[k] < Wave_Min) Wave_Min = temp_zbuf[k];      
            if (Wave_Max-Wave_Min < Stable_range )
            {
                
                if ( temp_zsum > 0)
                {
                    Z_AVG[0] = temp_zsum / BUF_RANGE;
    		        #ifdef DEBUG_MSG_CAL
						GSE_LOG("+++ Z_AVG=%d\n ", Z_AVG[0]);
                    #endif
                }
                else 
                {
                    Z_AVG[1] = temp_zsum / BUF_RANGE;
		            #ifdef DEBUG_MSG_CAL
						GSE_LOG("--- Z_AVG=%d\n ", Z_AVG[1]);
                    #endif
                }
            }
        }
        else if(abs((abs(raw[2])- Sensitivity_def))  > ((Detection_range)+ 154))
        {
            #ifdef DEBUG_MSG_CAL
				GSE_ERR("KXTJ2 out of SPEC Raw Data,%d,%d,%d\n",raw[0],raw[1],raw[2]);
            #endif
        }

        if ( raw[2] >=0) 
            raw[2] = raw[2] * 1024 / abs(Z_AVG[0]); // Gain Compensation
        else 
            raw[2] = raw[2] * 1024 / abs(Z_AVG[1]); // Gain Compensation
                
        #ifdef DEBUG_MSG_CAL
			GSE_ERR("kxtj2_1009 ---After Cali,X=%d,Y=%d,Z=%d \n",raw[0],raw[1],raw[2]);
        #endif
        obj->data[KXTJ2_1009_AXIS_X]=raw[0];
        obj->data[KXTJ2_1009_AXIS_Y]=raw[1];
        obj->data[KXTJ2_1009_AXIS_Z]=raw[2];
#endif
/*Kionix Auto-Cali End*/

		//printk("raw data x=%d, y=%d, z=%d \n",obj->data[KXTJ2_1009_AXIS_X],obj->data[KXTJ2_1009_AXIS_Y],obj->data[KXTJ2_1009_AXIS_Z]);
		obj->data[KXTJ2_1009_AXIS_X] += obj->cali_sw[KXTJ2_1009_AXIS_X];
		obj->data[KXTJ2_1009_AXIS_Y] += obj->cali_sw[KXTJ2_1009_AXIS_Y];
		obj->data[KXTJ2_1009_AXIS_Z] += obj->cali_sw[KXTJ2_1009_AXIS_Z];
		
		//printk("cali_sw x=%d, y=%d, z=%d \n",obj->cali_sw[KXTJ2_1009_AXIS_X],obj->cali_sw[KXTJ2_1009_AXIS_Y],obj->cali_sw[KXTJ2_1009_AXIS_Z]);
		
		/*remap coordinate*/
		acc[obj->cvt.map[KXTJ2_1009_AXIS_X]] = obj->cvt.sign[KXTJ2_1009_AXIS_X]*obj->data[KXTJ2_1009_AXIS_X];
		acc[obj->cvt.map[KXTJ2_1009_AXIS_Y]] = obj->cvt.sign[KXTJ2_1009_AXIS_Y]*obj->data[KXTJ2_1009_AXIS_Y];
		acc[obj->cvt.map[KXTJ2_1009_AXIS_Z]] = obj->cvt.sign[KXTJ2_1009_AXIS_Z]*obj->data[KXTJ2_1009_AXIS_Z];
		//printk("cvt x=%d, y=%d, z=%d \n",obj->cvt.sign[KXTJ2_1009_AXIS_X],obj->cvt.sign[KXTJ2_1009_AXIS_Y],obj->cvt.sign[KXTJ2_1009_AXIS_Z]);


		//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[KXTJ2_1009_AXIS_X], acc[KXTJ2_1009_AXIS_Y], acc[KXTJ2_1009_AXIS_Z]);

		//Out put the mg
		//printk("mg acc=%d, GRAVITY=%d, sensityvity=%d \n",acc[KXTJ2_1009_AXIS_X],GRAVITY_EARTH_1000,obj->reso->sensitivity);
		acc[KXTJ2_1009_AXIS_X] = acc[KXTJ2_1009_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[KXTJ2_1009_AXIS_Y] = acc[KXTJ2_1009_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[KXTJ2_1009_AXIS_Z] = acc[KXTJ2_1009_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;		
#endif	

		sprintf(buf, "%04x %04x %04x", acc[KXTJ2_1009_AXIS_X], acc[KXTJ2_1009_AXIS_Y], acc[KXTJ2_1009_AXIS_Z]);
		if(atomic_read(&obj->trace) & ADX_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
		}
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_ReadRawData(struct i2c_client *client, char *buf)
{
	struct kxtj2_1009_i2c_data *obj = (struct kxtj2_1009_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return EINVAL;
	}
	
	if(0 != (res = KXTJ2_1009_ReadData(client, obj->data)))
	{        
		GSE_ERR("kxtj2_1009 I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{
		sprintf(buf, "KXTJ2_1009_ReadRawData %04x %04x %04x", obj->data[KXTJ2_1009_AXIS_X], 
			obj->data[KXTJ2_1009_AXIS_Y], obj->data[KXTJ2_1009_AXIS_Z]);
	
	}
	
	return 0;
}
/*----------------------------------------------------------------------------*/
static int KXTJ2_1009_InitSelfTest(struct i2c_client *client)
{
	int res = 0;
	u8  data,result;
	
	res = hwmsen_read_byte(client, KXTJ2_1009_REG_CTL_REG3, &data);
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}
//enable selftest bit
	res = hwmsen_write_byte(client, KXTJ2_1009_REG_CTL_REG3,  KXTJ2_1009_SELF_TEST|data);
	if(res != KXTJ2_1009_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
//step 1
	res = hwmsen_read_byte(client, KXTJ2_1009_DCST_RESP, &result);
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}
	//GSE_LOG("step1: result = %x\n",result);
	if(result != 0xaa)
		return -EINVAL;

//step 2
	res = hwmsen_write_byte(client, KXTJ2_1009_REG_CTL_REG3,  KXTJ2_1009_SELF_TEST|data);
	if(res != KXTJ2_1009_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
//step 3
	res = hwmsen_read_byte(client, KXTJ2_1009_DCST_RESP, &result);
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}
	//GSE_LOG("step3: result = %x\n",result);
	if(result != 0xAA)
		return -EINVAL;
		
//step 4
	res = hwmsen_read_byte(client, KXTJ2_1009_DCST_RESP, &result);
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}
	//GSE_LOG("step4: result = %x\n",result);
	if(result != 0x55)
		return -EINVAL;
	else
		return KXTJ2_1009_SUCCESS;
}
/*----------------------------------------------------------------------------*/
#if 0
static int KXTJ2_1009_JudgeTestResult(struct i2c_client *client, s32 prv[KXTJ2_1009_AXES_NUM], s32 nxt[KXTJ2_1009_AXES_NUM])
{

    int res=0;
	u8 test_result=0;
    if(0 != (res = hwmsen_read_byte(client, 0x0c, &test_result)))
        return res;

	GSE_ERR("KXTJ2_1009 test_result = %x \n",test_result);
    if ( test_result != 0xaa ) 
	{
        GSE_ERR("KXTJ2_1009_JudgeTestResult failt\n");
        res = -EINVAL;
    }
    return res;
}
#endif
/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj2_1009_i2c_client;
	char strbuf[KXTJ2_1009_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("kxtj2_1009 i2c client is null!!\n");
		return 0;
	}
	
	KXTJ2_1009_ReadChipInfo(client, strbuf, KXTJ2_1009_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);        
}
#if 0
static ssize_t gsensor_init(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = kxtj2_1009_i2c_client;
		char strbuf[KXTJ2_1009_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("kxtj2_1009 i2c client is null!!\n");
			return 0;
		}
		kxtj2_1009_init_client(client, 1);
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}
#endif
/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj2_1009_i2c_client;
	char strbuf[KXTJ2_1009_BUFSIZE];
	
	if(NULL == client)
	{
		GSE_ERR("kxtj2_1009 i2c client is null!!\n");
		return 0;
	}
	KXTJ2_1009_ReadSensorData(client, strbuf, KXTJ2_1009_BUFSIZE);
	//KXTJ2_1009_ReadRawData(client, strbuf);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);            
}
#if 0
static ssize_t show_sensorrawdata_value(struct device_driver *ddri, char *buf, size_t count)
	{
		struct i2c_client *client = kxtj2_1009_i2c_client;
		char strbuf[KXTJ2_1009_BUFSIZE];
		
		if(NULL == client)
		{
			GSE_ERR("kxtj2_1009 i2c client is null!!\n");
			return 0;
		}
		//KXTJ2_1009_ReadSensorData(client, strbuf, KXTJ2_1009_BUFSIZE);
		KXTJ2_1009_ReadRawData(client, strbuf);
		return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);			
	}
#endif
/*----------------------------------------------------------------------------*/
static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj2_1009_i2c_client;
	struct kxtj2_1009_i2c_data *obj;
	int err, len = 0, mul;
	int tmp[KXTJ2_1009_AXES_NUM];

	if(NULL == client)
	{
		GSE_ERR("kxtj2_1009 i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);



	if(0 != (err = KXTJ2_1009_ReadOffset(client, obj->offset)))
	{
		return -EINVAL;
	}
	else if(0 != (err = KXTJ2_1009_ReadCalibration(client, tmp)))
	{
		return -EINVAL;
	}
	else
	{    
		mul = obj->reso->sensitivity/kxtj2_1009_offset_resolution.sensitivity;
		len += snprintf(buf+len, PAGE_SIZE-len, "[HW ][%d] (%+3d, %+3d, %+3d) : (0x%02X, 0x%02X, 0x%02X)\n", mul,                        
			obj->offset[KXTJ2_1009_AXIS_X], obj->offset[KXTJ2_1009_AXIS_Y], obj->offset[KXTJ2_1009_AXIS_Z],
			obj->offset[KXTJ2_1009_AXIS_X], obj->offset[KXTJ2_1009_AXIS_Y], obj->offset[KXTJ2_1009_AXIS_Z]);
		len += snprintf(buf+len, PAGE_SIZE-len, "[SW ][%d] (%+3d, %+3d, %+3d)\n", 1, 
			obj->cali_sw[KXTJ2_1009_AXIS_X], obj->cali_sw[KXTJ2_1009_AXIS_Y], obj->cali_sw[KXTJ2_1009_AXIS_Z]);

		len += snprintf(buf+len, PAGE_SIZE-len, "[ALL]    (%+3d, %+3d, %+3d) : (%+3d, %+3d, %+3d)\n", 
			obj->offset[KXTJ2_1009_AXIS_X]*mul + obj->cali_sw[KXTJ2_1009_AXIS_X],
			obj->offset[KXTJ2_1009_AXIS_Y]*mul + obj->cali_sw[KXTJ2_1009_AXIS_Y],
			obj->offset[KXTJ2_1009_AXIS_Z]*mul + obj->cali_sw[KXTJ2_1009_AXIS_Z],
			tmp[KXTJ2_1009_AXIS_X], tmp[KXTJ2_1009_AXIS_Y], tmp[KXTJ2_1009_AXIS_Z]);
		
		return len;
    }
}
/*----------------------------------------------------------------------------*/
static ssize_t store_cali_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct i2c_client *client = kxtj2_1009_i2c_client;  
	int err, x, y, z;
	int dat[KXTJ2_1009_AXES_NUM];

	if(!strncmp(buf, "rst", 3))
	{
		if(0 != (err = KXTJ2_1009_ResetCalibration(client)))
		{
			GSE_ERR("kxtj2_1009 reset offset err = %d\n", err);
		}	
	}
	else if(3 == sscanf(buf, "0x%02X 0x%02X 0x%02X", &x, &y, &z))
	{
		dat[KXTJ2_1009_AXIS_X] = x;
		dat[KXTJ2_1009_AXIS_Y] = y;
		dat[KXTJ2_1009_AXIS_Z] = z;
		if(0 != (err = KXTJ2_1009_WriteCalibration(client, dat)))
		{
			GSE_ERR("kxtj2_1009 write calibration err = %d\n", err);
		}		
	}
	else
	{
		GSE_ERR("kxtj2_1009 invalid format\n");
	}
	
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_self_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj2_1009_i2c_client;

	if(NULL == client)
	{
		GSE_ERR("kxtj2_1009 i2c client is null!!\n");
		return 0;
	}

	return snprintf(buf, 8, "%s\n", selftestRes);
}
/*----------------------------------------------------------------------------*/
static ssize_t store_self_value(struct device_driver *ddri, const char *buf, size_t count)
{   /*write anything to this register will trigger the process*/
	struct item{
	s16 raw[KXTJ2_1009_AXES_NUM];
	};
	
	struct i2c_client *client = kxtj2_1009_i2c_client;  
	int res, num;
	struct item *prv = NULL, *nxt = NULL;
	u8 data;

	if(1 != sscanf(buf, "%d", &num))
	{
		GSE_ERR("kxtj2_1009 parse number fail\n");
		return count;
	}
	else if(num == 0)
	{
		GSE_ERR("kxtj2_1009 invalid data count\n");
		return count;
	}

	prv = kzalloc(sizeof(*prv) * num, GFP_KERNEL);
	nxt = kzalloc(sizeof(*nxt) * num, GFP_KERNEL);
	if (!prv || !nxt)
	{
		goto exit;
	}


	GSE_LOG("NORMAL:\n");
	KXTJ2_1009_SetPowerMode(client,true); 

	/*initial setting for self test*/
	if(!KXTJ2_1009_InitSelfTest(client))
	{
		GSE_LOG("SELFTEST : PASS\n");
		strcpy(selftestRes,"y");
	}	
	else
	{
		GSE_LOG("SELFTEST : FAIL\n");		
		strcpy(selftestRes,"n");
	}

	res = hwmsen_read_byte(client, KXTJ2_1009_REG_CTL_REG3, &data);
	if(res != KXTJ2_1009_SUCCESS)
	{
		return res;
	}

	res = hwmsen_write_byte(client, KXTJ2_1009_REG_CTL_REG3,  ~KXTJ2_1009_SELF_TEST&data);
	if(res != KXTJ2_1009_SUCCESS) //0x2C->BW=100Hz
	{
		return res;
	}
	
	exit:
	/*restore the setting*/    
	kxtj2_1009_init_client(client, 0);
	kfree(prv);
	kfree(nxt);
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_selftest_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = kxtj2_1009_i2c_client;
	struct kxtj2_1009_i2c_data *obj;

	if(NULL == client)
	{
		GSE_ERR("kxtj2_1009 i2c client is null!!\n");
		return 0;
	}

	obj = i2c_get_clientdata(client);
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->selftest));
}
/*----------------------------------------------------------------------------*/
static ssize_t store_selftest_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
	int tmp;

	if(NULL == obj)
	{
		GSE_ERR("kxtj2_1009 i2c data obj is null!!\n");
		return 0;
	}
	
	
	if(1 == sscanf(buf, "%d", &tmp))
	{        
		if(atomic_read(&obj->selftest) && !tmp)
		{
			/*enable -> disable*/
			kxtj2_1009_init_client(obj->client, 0);
		}
		else if(!atomic_read(&obj->selftest) && tmp)
		{
			/*disable -> enable*/
			KXTJ2_1009_InitSelfTest(obj->client);            
		}
		
		GSE_LOG("selftest: %d => %d\n", atomic_read(&obj->selftest), tmp);
		atomic_set(&obj->selftest, tmp); 
	}
	else
	{ 
		GSE_ERR("kxtj2_1009 invalid content: '%s', length = %d\n", buf, (int)count);   
	}
	return count;
}

/* KXTJ3 sensor selftest is not same as in KXTJ2 */
static int KXTJ3_SelfTest(void)
{  //set PC1 = 0,2G,12bit ,ST POL = 1,
        int res;
        int i;
        u8 databuf[2];
        s16 st_pol_positive[3] = {0};
        s16 st_normal[3] = {0};
        s16 st_pol_positive_sum[3] = {0};
        s16 st_normal_sum[3] = {0};
        s16 st_diff[3] = {0};
	int test_status = 1; /* default is test pass */

        struct i2c_client *client = kxtj2_1009_i2c_client;

	//GSE_LOG("KXTJ3_SelfTest : in");

        res = KXTJ2_1009_SetBWRate(client, KXTJ2_1009_BW_50HZ);
        if(res != KXTJ2_1009_SUCCESS ) //0x2C->BW=100Hz
        {
                return res;
        }
        res = KXTJ2_1009_SetDataFormat(client, KXTJ2_1009_RANGE_2G);
        if(res != KXTJ2_1009_SUCCESS) //0x2C->BW=100Hz
        {
                return res;
        }
	/* 1 step, read data in self-test mode */
        //ST POL = 1,
        res = KXTJ2_1009_SetStPol(client, 1);
        if(res != KXTJ2_1009_SUCCESS)
        {
                return res;
        }
        msleep(30);
        //Set PC1 = 1, selftest enable
        if(sensor_power == false)
        {
            KXTJ2_1009_SetPowerMode(client, true);
            sensor_power = false;
        }
        msleep(50);
        KXTJ2_1009_SetSelfTest_On(client, 1);
        KXTJ2_1009_ReadData(client,st_pol_positive);
        msleep(30);
        for(i=0; i<5; i++)
        {
           KXTJ2_1009_ReadData(client,st_pol_positive);
           st_pol_positive_sum[0] += st_pol_positive[0];
           st_pol_positive_sum[1] += st_pol_positive[1];
           st_pol_positive_sum[2] += st_pol_positive[2];
           GSE_LOG("KXTJ2_1009 positive %d %d %d \n",st_pol_positive[0],st_pol_positive[1],st_pol_positive[2]);
           msleep(30);
        }
        //Set PC1 = 0,ST POL = 0
        KXTJ2_1009_SetSelfTest_On(client, 0);
        res = KXTJ2_1009_SetStPol(client, 0);
        if(res != KXTJ2_1009_SUCCESS)
        {
                return res;
        }
        msleep(30);
	/* 1 step, read data in normal mode */
        //Set PC1 = 1, selftest enable
        if(sensor_power == false)
        {
            KXTJ2_1009_SetPowerMode(client, true);
            sensor_power = false;
        }
        msleep(50);
        KXTJ2_1009_ReadData(client,st_normal);
        msleep(30);
        for(i=0; i<5; i++)
        {
           KXTJ2_1009_ReadData(client,st_normal);
           st_normal_sum[0] += st_normal[0];
           st_normal_sum[1] += st_normal[1];
           st_normal_sum[2] += st_normal[2];
           GSE_LOG("KXTJ2_1009 normal %d %d %d \n",st_normal[0],st_normal[1],st_normal[2]);
           msleep(30);
        }
        //Set PC1 = 0, selftest disable
        KXTJ2_1009_SetSelfTest_On(client, 0);
        if(sensor_power == true)
        {
            KXTJ2_1009_SetPowerMode(client, false);
            sensor_power = true;
        }else{
            KXTJ2_1009_SetPowerMode(client, false);
            sensor_power = false;
        }
        //soft reset
        databuf[0] = 0x1D;
        databuf[1] = 0x80;
        i2c_master_send(client, databuf, 0x2);
        msleep(10);       
        hwmsen_read_block(client, 0x1D, databuf, 0x01);
        while((databuf[0] & 0x80) == 0x80)
        {
           hwmsen_read_block(client, 0x1D, databuf, 0x01);
        }
        KXTJ2_1009_SetBWRate(client, KXTJ2_1009_BW_50HZ);
        KXTJ2_1009_SetPowerMode(client, sensor_power);
        st_pol_positive[0]= st_pol_positive_sum[0] / 5;
        st_pol_positive[1]= st_pol_positive_sum[1] / 5;
        st_pol_positive[2]= st_pol_positive_sum[2] / 5;
        st_normal[0]= st_normal_sum[0] / 5;
        st_normal[1]= st_normal_sum[1] / 5;
        st_normal[2]= st_normal_sum[2] / 5;
        GSE_LOG("KXTJ2_1009 avg. positive %d %d %d  normal %d %d %d\n", st_pol_positive[0], st_pol_positive[1], st_pol_positive[2],
	                          st_normal[0],st_normal[1],st_normal[2]);
/* Test fail threshold, 12bit 2G mode */
#define DIFF_LOW_TH (102)
#define DIFF_HIGH_TH (922)

	/* calculate diff and compare result */
	for (i = 0; i < 3; i++) {
		st_diff[i] = st_pol_positive[i] - st_normal[i];
		if (st_diff[i] < 0)
			st_diff[i] = -st_diff[i];

		if (st_diff[i] < DIFF_LOW_TH || st_diff[i] > DIFF_HIGH_TH) {
			/* test fail */
			test_status = 0;
		}
	}

      //  GSE_LOG("KXTJ2_1009 diff %d %d %d\n",st_diff[0],st_diff[1],st_diff[2]);

	//GSE_LOG("KXTJ3_SelfTest : out - test_status %d", test_status);

	return test_status;
}

static int KXTJ1009_SelfTest(void)
{  //set PC1 = 0,2G,12bit ,ST POL = 1,
        int res;
        int i;
        u8 databuf[2];
        s16 st_pol_positive[3] = {0};
        s16 st_pol_nagetive[3] = {0};
        s16 st_pol_positive_sum[3] = {0};
        s16 st_pol_nagetive_sum[3] = {0};
        s16 st_diff[3] = {0};
        int test_status = 0;

        struct i2c_client *client = kxtj2_1009_i2c_client;

        GSE_LOG("KXTJ1009_SelfTest : in");

        res = KXTJ2_1009_SetBWRate(client, KXTJ2_1009_BW_50HZ);
        if(res != KXTJ2_1009_SUCCESS ) //0x2C->BW=100Hz
        {
                return res;
        }
        res = KXTJ2_1009_SetDataFormat(client, KXTJ2_1009_RANGE_2G);
        if(res != KXTJ2_1009_SUCCESS) //0x2C->BW=100Hz
        {
                return res;
        }
        //ST POL = 1,
        res = KXTJ2_1009_SetStPol(client, 1);
        if(res != KXTJ2_1009_SUCCESS)
        {
                return res;
        }
        msleep(30);
        //Set PC1 = 1, selftest enable
        if(sensor_power == false)
        {
            KXTJ2_1009_SetPowerMode(client, true);
            sensor_power = false;
        }
        msleep(50);
        KXTJ2_1009_SetSelfTest_On(client, 1);
        KXTJ2_1009_ReadData(client,st_pol_positive);
        msleep(30);
        for(i=0; i<5; i++)
        {
           KXTJ2_1009_ReadData(client,st_pol_positive);
           st_pol_positive_sum[0] += st_pol_positive[0];
           st_pol_positive_sum[1] += st_pol_positive[1];
           st_pol_positive_sum[2] += st_pol_positive[2];
           GSE_LOG("KXTJ2_1009 positive %d %d %d \n",st_pol_positive[0],st_pol_positive[1],st_pol_positive[2]);
           msleep(30);
        }
        //Set PC1 = 0,ST POL = 0
        KXTJ2_1009_SetSelfTest_On(client, 0);
        res = KXTJ2_1009_SetStPol(client, 0);
        if(res != KXTJ2_1009_SUCCESS)
        {
                return res;
        }
        msleep(30);
        //Set PC1 = 1, selftest enable
        if(sensor_power == false)
        {
            KXTJ2_1009_SetPowerMode(client, true);
            sensor_power = false;
        }
        msleep(50);
        KXTJ2_1009_SetSelfTest_On(client, 1);
        KXTJ2_1009_ReadData(client,st_pol_nagetive);
        msleep(30);
        for(i=0; i<5; i++)
        {
           KXTJ2_1009_ReadData(client,st_pol_nagetive);
           st_pol_nagetive_sum[0] += st_pol_nagetive[0];
           st_pol_nagetive_sum[1] += st_pol_nagetive[1];
           st_pol_nagetive_sum[2] += st_pol_nagetive[2];
           GSE_LOG("KXTJ2_1009 nagetive %d %d %d \n",st_pol_nagetive[0],st_pol_nagetive[1],st_pol_nagetive[2]);
           msleep(30);
        }
        //Set PC1 = 0, selftest disable
        KXTJ2_1009_SetSelfTest_On(client, 0);
        if(sensor_power == true)
        {
            KXTJ2_1009_SetPowerMode(client, false);
            sensor_power = true;
        }else{
            KXTJ2_1009_SetPowerMode(client, false);
            sensor_power = false;
        }
        //soft reset
        databuf[0] = 0x1D;
        databuf[1] = 0x80;
        i2c_master_send(client, databuf, 0x2);
        msleep(10);       
        hwmsen_read_block(client, 0x1D, databuf, 0x01);
        while((databuf[0] & 0x80) == 0x80)
        {
           hwmsen_read_block(client, 0x1D, databuf, 0x01);
        }
        KXTJ2_1009_SetBWRate(client, KXTJ2_1009_BW_50HZ);
        KXTJ2_1009_SetPowerMode(client, sensor_power);
        st_pol_positive[0]= st_pol_positive_sum[0] / 5;
        st_pol_positive[1]= st_pol_positive_sum[1] / 5;
        st_pol_positive[2]= st_pol_positive_sum[2] / 5;
        st_pol_nagetive[0]= st_pol_nagetive_sum[0] / 5;
        st_pol_nagetive[1]= st_pol_nagetive_sum[1] / 5;
        st_pol_nagetive[2]= st_pol_nagetive_sum[2] / 5;
        GSE_LOG("KXTJ2_1009 sum positive %d %d %d  nagetive %d %d %d\n", st_pol_positive_sum[0], st_pol_positive_sum[1], st_pol_positive_sum[2],
                        st_pol_nagetive_sum[0],st_pol_nagetive_sum[1],st_pol_nagetive_sum[2]);
        for(i=0; i<3; i++)
        {
          st_diff[i] = st_pol_positive[i] - st_pol_nagetive[i];
          if(st_diff[i] < 0)
            st_diff[i] = -st_diff[i];
	}

        GSE_LOG("KXTJ2_1009 diff %d %d %d\n",st_diff[0],st_diff[1],st_diff[2]);

        if (((st_diff[0] > 700)&&(st_diff[0] < 1500))
            && ((st_diff[1] > 700)&&(st_diff[1] < 1500))
            && ((st_diff[2] > 400)&&(st_diff[2] < 1200)))
              test_status = 1;

      //  GSE_LOG("KXTJ1009_SelfTest : out - test_status %d", test_status);

        return test_status;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_SelfTest_value(struct device_driver *ddri, char *buf)
{
        int result;
        struct i2c_client *client = kxtj2_1009_i2c_client;

        if(NULL == client)
        {
                GSE_ERR("kxtj2_1009 i2c client is null!!\n");
                return 0;
        }
	
	if (obj_i2c_data->wai == KXTJ3_WHO_AM_I_ID) {
		result = KXTJ3_SelfTest();
	}
	else {
		result = KXTJ1009_SelfTest();
	}

        return snprintf(buf, sizeof(int), "%d\n", result);
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static ssize_t show_firlen_value(struct device_driver *ddri, char *buf)
{
#ifdef CONFIG_KXTJ2_1009_LOWPASS
	struct i2c_client *client = kxtj2_1009_i2c_client;
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
	if(atomic_read(&obj->firlen))
	{
		int idx, len = atomic_read(&obj->firlen);
		GSE_LOG("len = %2d, idx = %2d\n", obj->fir.num, obj->fir.idx);

		for(idx = 0; idx < len; idx++)
		{
			GSE_LOG("[%5d %5d %5d]\n", obj->fir.raw[idx][KXTJ2_1009_AXIS_X], obj->fir.raw[idx][KXTJ2_1009_AXIS_Y], obj->fir.raw[idx][KXTJ2_1009_AXIS_Z]);
		}
		
		GSE_LOG("sum = [%5d %5d %5d]\n", obj->fir.sum[KXTJ2_1009_AXIS_X], obj->fir.sum[KXTJ2_1009_AXIS_Y], obj->fir.sum[KXTJ2_1009_AXIS_Z]);
		GSE_LOG("avg = [%5d %5d %5d]\n", obj->fir.sum[KXTJ2_1009_AXIS_X]/len, obj->fir.sum[KXTJ2_1009_AXIS_Y]/len, obj->fir.sum[KXTJ2_1009_AXIS_Z]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->firlen));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}
/*----------------------------------------------------------------------------*/
static ssize_t store_firlen_value(struct device_driver *ddri, const char *buf, size_t count)
{
#ifdef CONFIG_KXTJ2_1009_LOWPASS
	struct i2c_client *client = kxtj2_1009_i2c_client;  
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);
	int firlen;

	if(1 != sscanf(buf, "%d", &firlen))
	{
		GSE_ERR("kxtj2_1009 invallid format\n");
	}
	else if(firlen > C_MAX_FIR_LENGTH)
	{
		GSE_ERR("kxtj2_1009 exceeds maximum filter length\n");
	}
	else
	{ 
		atomic_set(&obj->firlen, firlen);
		if(NULL == firlen)
		{
			atomic_set(&obj->fir_en, 0);
		}
		else
		{
			memset(&obj->fir, 0x00, sizeof(obj->fir));
			atomic_set(&obj->fir_en, 1);
		}
	}
#endif    
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("kxtj2_1009 i2c_data obj is null!!\n");
		return 0;
	}
	
	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));     
	return res;    
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("kxtj2_1009 i2c_data obj is null!!\n");
		return 0;
	}
	
	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}	
	else
	{
		GSE_ERR("kxtj2_1009 invalid content: '%s', length = %d\n", buf, (int)count);
	}
	
	return count;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;    
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("kxtj2_1009 i2c_data obj is null!!\n");
		return 0;
	}	
	
	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n", 
	            obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol);   
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	return len;    
}
/*----------------------------------------------------------------------------*/
static ssize_t show_power_status_value(struct device_driver *ddri, char *buf)
{
	u8 databuf[2];    
	u8 addr = KXTJ2_1009_REG_POWER_CTL;
	if(hwmsen_read_block(kxtj2_1009_i2c_client, addr, databuf, 0x01))
	{
		GSE_ERR("kxtj2_1009 read power ctl register err!\n");
		return KXTJ2_1009_ERR_I2C;
	}
    
	if(sensor_power)
		GSE_ERR("kxtj2_1009 is in work mode, sensor_power = %d\n", sensor_power);
	else
		GSE_ERR("kxtj2_1009 is in standby mode, sensor_power = %d\n", sensor_power);

	return snprintf(buf, PAGE_SIZE, "%x\n", databuf[0]);
}

//Added by yuduan.xie for significant on 2017.2.27 begin
static ssize_t show_reg_val(struct device_driver *ddri, char *buf)
{
	int ret = 0;
	u8 rbuf[0x6c];
	int cnt = 0;	
	int len = 0;
	int i = 0;
	//u8 rbuf_1[2];
    struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
	memset(rbuf, 0, sizeof(rbuf));
	
	#if 1
	for(i=0;i<=0x6b;i++)
	{
	    ret = hwmsen_read_block(obj->client, i, &rbuf[i], 1);
	    if(ret < 0)
	    {
		    GSE_ERR("error: %d\n", ret);
		    return -EFAULT;
	    }	
	}
	#endif
	/*
	 ret = hwmsen_read_block(obj->client, 0x29, &rbuf_1[0], 1);
	    if(ret < 0)
	    {
		    GSE_ERR("error: %d\n", ret);
		    return -EFAULT;
	    }	
	ret = hwmsen_read_block(obj->client, 0x6a, &rbuf_1[1], 1);
	    if(ret < 0)
	    {
		    GSE_ERR("error: %d\n", ret);
		    return -EFAULT;
	    }	
	*/
	for(cnt=0;cnt<=0x6b;cnt++)
	{
		//GSE_ERR("reg[0x%x]=0x%x\n", cnt, rbuf[cnt]);
		len += scnprintf(buf+len, PAGE_SIZE-len, "[%2X]%2X,", cnt, rbuf[cnt]);
	}	
	//len += scnprintf(buf+len, PAGE_SIZE-len, "[%2X]%2X,", 0x29, rbuf_1[0]);
	//len += scnprintf(buf+len, PAGE_SIZE-len, "[%2X]%2X,", 0x6a, rbuf_1[1]);
	
	return len;
	
}
static ssize_t store_reg_val(struct device_driver *ddri, const char *buf, size_t count)
{
     int reg;
     int values;
     int res;
     struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
      if(!obj)
      {
          return -1;
      }
     res = sscanf(buf, "%x,%x", &reg, &values);
    if (res == 2)
    {
          KXTJ2_1009_SetPowerMode(obj->client,false);
	 
          if(hwmsen_write_byte(obj->client,reg,values))
          {
                //GSE_ERR("reg =%d write values = %dfailed\n",reg,values);
		 KXTJ2_1009_SetPowerMode(obj->client,true);
		return -EFAULT;
          }
	   KXTJ2_1009_SetPowerMode(obj->client,true);
      }
       else
       {
		//GSE_ERR("invalid content: %s , res = %d\n", buf,res);
	}
	 return count;
}
//Added by yuduan.xie for significant  on 2017.2.27 end
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo,   S_IWUSR | S_IRUGO, show_chipinfo_value,      NULL);
static DRIVER_ATTR(sensordata, S_IWUSR | S_IRUGO, show_sensordata_value,    NULL);
static DRIVER_ATTR(cali,       S_IWUSR | S_IRUGO, show_cali_value,          store_cali_value);
static DRIVER_ATTR(selftest, S_IWUSR | S_IRUGO, show_self_value,  store_self_value);
static DRIVER_ATTR(self,   S_IWUSR | S_IRUGO, show_selftest_value,      store_selftest_value);
static DRIVER_ATTR(firlen,     S_IWUSR | S_IRUGO, show_firlen_value,        store_firlen_value);
static DRIVER_ATTR(trace,      S_IWUSR | S_IRUGO, show_trace_value,         store_trace_value);
static DRIVER_ATTR(status,               S_IRUGO, show_status_value,        NULL);
static DRIVER_ATTR(powerstatus,               S_IRUGO, show_power_status_value,        NULL);
static DRIVER_ATTR(midtest,             S_IRUGO, show_SelfTest_value,      NULL);
static DRIVER_ATTR(regval,               S_IWUSR | S_IRUGO, show_reg_val,        store_reg_val);


/*----------------------------------------------------------------------------*/
static u8 i2c_dev_reg =0 ;

static ssize_t show_register(struct device_driver *pdri, char *buf)
{
	GSE_ERR("KXTJ2_1009 i2c_dev_reg is 0x%2x \n", i2c_dev_reg);

	return 0;
}

static ssize_t store_register(struct device_driver *ddri, const char *buf, size_t count)
{
	i2c_dev_reg = simple_strtoul(buf, NULL, 16);
	GSE_ERR("kxtj2_1009 set i2c_dev_reg = 0x%2x \n", i2c_dev_reg);

	return 0;
}
static ssize_t store_register_value(struct device_driver *ddri, const char *buf, size_t count)
{
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
	u8 databuf[2];  
	unsigned long input_value;
	int res;
	
	memset(databuf, 0, sizeof(u8)*2);    

	input_value = simple_strtoul(buf, NULL, 16);
	GSE_ERR("kxtj2_1009 input_value = 0x%2x \n", (unsigned int)input_value);

	if(NULL == obj)
	{
		GSE_ERR("kxtj2_1009 i2c data obj is null!!\n");
		return 0;
	}

	databuf[0] = i2c_dev_reg;
	databuf[1] = input_value;
	GSE_ERR("kxtj2_1009 databuf[0]=0x%2x  databuf[1]=0x%2x \n", databuf[0],databuf[1]);

	res = i2c_master_send(obj->client, databuf, 0x2);

	if(res <= 0)
	{
		return KXTJ2_1009_ERR_I2C;
	}
	return 0;
	
}

static ssize_t show_register_value(struct device_driver *ddri, char *buf)
{
		struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
		u8 databuf[1];	
		
		memset(databuf, 0, sizeof(u8)*1);	 
	
		if(NULL == obj)
		{
			GSE_ERR("kxtj2_1009 i2c data obj is null!!\n");
			return 0;
		}
		
		if(hwmsen_read_block(obj->client, i2c_dev_reg, databuf, 0x01))
		{
			GSE_ERR("kxtj2_1009 read power ctl register err!\n");
			return KXTJ2_1009_ERR_I2C;
		}

		GSE_ERR("kxtj2_1009 i2c_dev_reg=0x%2x  data=0x%2x \n", i2c_dev_reg,databuf[0]);
	
		return 0;
		
}


static DRIVER_ATTR(i2c,      S_IWUSR | S_IRUGO, show_register_value,         store_register_value);
static DRIVER_ATTR(register,      S_IWUSR | S_IRUGO, show_register,         store_register);


/*----------------------------------------------------------------------------*/
static struct driver_attribute *kxtj2_1009_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
	&driver_attr_self,         /*self test demo*/
	&driver_attr_selftest,     /*self control: 0: disable, 1: enable*/
	&driver_attr_firlen,       /*filter length: 0: disable, others: enable*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,
	&driver_attr_powerstatus,
	&driver_attr_register,
	&driver_attr_i2c,
        &driver_attr_midtest,
        &driver_attr_regval,
};
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_create_attr(struct device_driver *driver) 
{
	int idx, err = 0;
	int num = (int)(sizeof(kxtj2_1009_attr_list)/sizeof(kxtj2_1009_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(0 != (err = driver_create_file(driver, kxtj2_1009_attr_list[idx])))
		{            
			GSE_ERR("kxtj2_1009 driver_create_file (%s) = %d\n", kxtj2_1009_attr_list[idx]->attr.name, err);
			break;
		}
	}    
	return err;
}
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(kxtj2_1009_attr_list)/sizeof(kxtj2_1009_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}
	

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, kxtj2_1009_attr_list[idx]);
	}
	

	return err;
}
/****************************************************************************** 
 * Function Configuration
******************************************************************************/
/*----------------------------------------------------------------------------*/
#ifdef CUSTOM_KERNEL_SENSORHUB
static void kxtj2_1009_irq_work(struct work_struct *work)
{
    struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
    struct scp_acc_hw scp_hw;
    KXTJ2_1009_CUST_DATA *p_cust_data;
    SCP_SENSOR_HUB_DATA data;
    int max_cust_data_size_per_packet;
    int i;
    uint sizeOfCustData;
    uint len;
    char *p = (char *)&scp_hw;

    GSE_FUN();

    scp_hw.i2c_num = obj->hw->i2c_num;
    scp_hw.direction = obj->hw->direction;
    scp_hw.power_id = obj->hw->power_id;
    scp_hw.power_vol = obj->hw->power_vol;
    scp_hw.firlen = obj->hw->firlen;
    memcpy(scp_hw.i2c_addr, obj->hw->i2c_addr, sizeof(obj->hw->i2c_addr));
    scp_hw.power_vio_id = obj->hw->power_vio_id;
    scp_hw.power_vio_vol = obj->hw->power_vio_vol;
    scp_hw.is_batch_supported = obj->hw->is_batch_supported;

    p_cust_data = (KXTJ2_1009_CUST_DATA *)data.set_cust_req.custData;
    sizeOfCustData = sizeof(scp_hw);
    max_cust_data_size_per_packet = sizeof(data.set_cust_req.custData) - offsetof(KXTJ2_1009_SET_CUST, data);
    
    for (i=0;sizeOfCustData>0;i++)
    {
        data.set_cust_req.sensorType = ID_ACCELEROMETER;
        data.set_cust_req.action = SENSOR_HUB_SET_CUST;
        p_cust_data->setCust.action = KXTJ2_1009_CUST_ACTION_SET_CUST;
        p_cust_data->setCust.part = i;
        if (sizeOfCustData > max_cust_data_size_per_packet)
        {
            len = max_cust_data_size_per_packet;
        }
        else
        {
            len = sizeOfCustData;
        }

        memcpy(p_cust_data->setCust.data, p, len);
        sizeOfCustData -= len;
        p += len;
        
        len += offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + offsetof(KXTJ2_1009_SET_CUST, data);
        SCP_sensorHub_req_send(&data, &len, 1);
    }

	//KXTJ2_1009_ResetCalibration
	p_cust_data = (KXTJ2_1009_CUST_DATA *)&data.set_cust_req.custData;
	
	data.set_cust_req.sensorType = ID_ACCELEROMETER;
	data.set_cust_req.action = SENSOR_HUB_SET_CUST;
	p_cust_data->resetCali.action = KXTJ2_1009_CUST_ACTION_RESET_CALI;
	len = offsetof(SCP_SENSOR_HUB_SET_CUST_REQ, custData) + sizeof(p_cust_data->resetCali);
	SCP_sensorHub_req_send(&data, &len, 1);

    obj->SCP_init_done = 1;
}
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_irq_handler(void* data, uint len)
{
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
    SCP_SENSOR_HUB_DATA_P rsp = (SCP_SENSOR_HUB_DATA_P)data;
    
	if(!obj)
	{
		return -1;
	}

    switch(rsp->rsp.action)
    {
        case SENSOR_HUB_NOTIFY:
            switch(rsp->notify_rsp.event)
            {
                case SCP_INIT_DONE:
                    schedule_work(&obj->irq_work);
                    break;
                default:
                    GSE_ERR("kxtj2_1009 Error sensor hub notify");
                    break;
            }
            break;
        default:
            GSE_ERR("kxtj2_1009 Error sensor hub action");
            break;
    }

    return 0;
}

static int kxtj2_1009_setup_irq()
{
    int err = 0;

    err = SCP_sensorHub_rsp_registration(ID_ACCELEROMETER, kxtj2_1009_irq_handler);
    
	return err;
}
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_open(struct inode *inode, struct file *file)
{
	file->private_data = kxtj2_1009_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("kxtj2_1009 null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

#ifdef CONFIG_COMPAT
static long kxtj2_1009_compat_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
    long err = 0;

	void __user *arg32 = compat_ptr(arg);
	
	if (!file->f_op || !file->f_op->unlocked_ioctl)
		return -ENOTTY;
	
  	switch (cmd)
	{
		case COMPAT_GSENSOR_IOCTL_READ_SENSORDATA:
			if (arg32 == NULL)
			{
				err = -EINVAL;
				break;    
			}

			err = file->f_op->unlocked_ioctl(file, GSENSOR_IOCTL_READ_SENSORDATA, (unsigned long)arg32);
			if (err){
				GSE_ERR("GSENSOR_IOCTL_READ_SENSORDATA unlocked_ioctl failed.");
				return err;
			}
		//	GSE_ERR("kxtj2_1009_compat_ioctl wingtech chenzhi COMPAT_GSENSOR_IOCTL_READ_SENSORDATA.");
			break;
		case COMPAT_GSENSOR_IOCTL_SET_CALI:
			if (arg32 == NULL)
			{
				err = -EINVAL;
				break;    
			}

			err = file->f_op->unlocked_ioctl(file, GSENSOR_IOCTL_SET_CALI, (unsigned long)arg32);
			if (err){
				GSE_ERR("GSENSOR_IOCTL_SET_CALI unlocked_ioctl failed.");
				return err;
			}
		//	GSE_ERR("kxtj2_1009_compat_ioctl wingtech chenzhi COMPAT_GSENSOR_IOCTL_SET_CALI.");
			break;
		case COMPAT_GSENSOR_IOCTL_GET_CALI:
			if (arg32 == NULL)
			{
				err = -EINVAL;
				break;    
			}

			err = file->f_op->unlocked_ioctl(file, GSENSOR_IOCTL_GET_CALI, (unsigned long)arg32);
			if (err){
				GSE_ERR("GSENSOR_IOCTL_GET_CALI unlocked_ioctl failed.");
				return err;
			}
		//	GSE_ERR("kxtj2_1009_compat_ioctl wingtech chenzhi  GSENSOR_IOCTL_GET_CALI.");
			break;
		case COMPAT_GSENSOR_IOCTL_CLR_CALI:
			if (arg32 == NULL)
			{
				err = -EINVAL;
				break;    
			}

			err = file->f_op->unlocked_ioctl(file, GSENSOR_IOCTL_CLR_CALI, (unsigned long)arg32);
			if (err){
				GSE_ERR("GSENSOR_IOCTL_CLR_CALI unlocked_ioctl failed.");
				return err;
			}
		//	GSE_ERR("kxtj2_1009_compat_ioctl wingtech chenzhi  COMPAT_GSENSOR_IOCTL_CLR_CALI.");
			break;

		default:
			GSE_ERR("kxtj2_1009_compat_ioctl unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;

	}
	return err;
}
#endif

/*----------------------------------------------------------------------------*/
//static int kxtj2_1009_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long kxtj2_1009_unlocked_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct kxtj2_1009_i2c_data *obj = (struct kxtj2_1009_i2c_data*)i2c_get_clientdata(client);	
	char strbuf[KXTJ2_1009_BUFSIZE];
	void __user *data;
	struct SENSOR_DATA sensor_data;
	long err = 0;
	int cali[3];

	//GSE_FUN(f);
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("kxtj2_1009 access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case GSENSOR_IOCTL_INIT:
			kxtj2_1009_init_client(client, 0);			
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			
			KXTJ2_1009_ReadChipInfo(client, strbuf, KXTJ2_1009_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}				 
			break;	  

		case GSENSOR_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			KXTJ2_1009_SetPowerMode(obj->client, true);
			KXTJ2_1009_ReadSensorData(client, strbuf, KXTJ2_1009_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}
           // GSE_ERR("kxtj2_1009_unlocked_ioctl GSENSOR_IOCTL_READ_SENSORDATA\n");			
			break;

		case GSENSOR_IOCTL_READ_GAIN:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &gsensor_gain, sizeof(struct GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

		case GSENSOR_IOCTL_READ_RAW_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			KXTJ2_1009_ReadRawData(client, strbuf);
			if(copy_to_user(data, &strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;	  
			}
			//GSE_ERR("kxtj2_1009_unlocked_ioctl GSENSOR_IOCTL_READ_RAW_DATA\n");
			break;	  

		case GSENSOR_IOCTL_SET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;	  
			}
			if(atomic_read(&obj->suspend))
			{
				GSE_ERR("kxtj2_1009 Perform calibration in suspend state!!\n");
				err = -EINVAL;
			}
			else
			{
				cali[KXTJ2_1009_AXIS_X] = sensor_data.x * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[KXTJ2_1009_AXIS_Y] = sensor_data.y * obj->reso->sensitivity / GRAVITY_EARTH_1000;
				cali[KXTJ2_1009_AXIS_Z] = sensor_data.z * obj->reso->sensitivity / GRAVITY_EARTH_1000;			  
				err = KXTJ2_1009_WriteCalibration(client, cali);			 
			}
			//GSE_ERR("kxtj2_1009_unlocked_ioctl GSENSOR_IOCTL_SET_CALI\n");
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = KXTJ2_1009_ResetCalibration(client);
			//GSE_ERR("kxtj2_1009_unlocked_ioctl GSENSOR_IOCTL_CLR_CALI\n");
			break;

		case GSENSOR_IOCTL_GET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}
			if(0 != (err = KXTJ2_1009_ReadCalibration(client, cali)))
			{
				break;
			}
			
			sensor_data.x = cali[KXTJ2_1009_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.y = cali[KXTJ2_1009_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			sensor_data.z = cali[KXTJ2_1009_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
			if(copy_to_user(data, &sensor_data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}	
           //  GSE_ERR("kxtj2_1009_unlocked_ioctl GSENSOR_IOCTL_CLR_CALI\n");			
			break;
		

		default:
			//GSE_ERR("kxtj2_1009 unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}
 

/*----------------------------------------------------------------------------*/
static struct file_operations kxtj2_1009_fops = {
	.owner = THIS_MODULE,
	.open = kxtj2_1009_open,
	.release = kxtj2_1009_release,
	.unlocked_ioctl = kxtj2_1009_unlocked_ioctl,
	#ifdef CONFIG_COMPAT
	.compat_ioctl = kxtj2_1009_compat_ioctl,
	#endif
	//.ioctl = kxtj2_1009_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice kxtj2_1009_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &kxtj2_1009_fops,
};
/*----------------------------------------------------------------------------*/
#if !defined(CONFIG_HAS_EARLYSUSPEND) || !defined(USE_EARLY_SUSPEND)
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);    
	int err = 0;
	GSE_FUN();    
#ifdef SIGNIFICANT
	if(significant_enable_status == true)
	{
	    GSE_ERR("kxtj2_suspend--1\n");
		return 0;
	}
#endif

	if(obj == NULL)
	{
		GSE_ERR("kxtj2_1009 null pointer!!\n");
		return -EINVAL;
	}
    mutex_lock(&kxtj2_1009_mutex);
	atomic_set(&obj->suspend, 1);
pre_suspend = sensor_power;
#ifdef CUSTOM_KERNEL_SENSORHUB
	if(0 != (err = KXTJ2_1009_SCP_SetPowerMode(false)))
#else
	if(0 != (err = KXTJ2_1009_SetPowerMode(obj->client,false)))
#endif
	{
		GSE_ERR("kxtj2_1009 write power control fail!!\n");
		mutex_unlock(&kxtj2_1009_mutex);
		return -1;
	}
    mutex_unlock(&kxtj2_1009_mutex);

	//sensor_power = false;     //Add by zhi.chen.hz  
#ifndef CUSTOM_KERNEL_SENSORHUB
	KXTJ2_1009_power(obj->hw, 0);
#endif

	return err;
}
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kxtj2_1009_i2c_data *obj = i2c_get_clientdata(client);        
	int err;
	GSE_FUN();
#ifdef SIGNIFICANT
	if(significant_enable_status == true)
	{
	    GSE_ERR("kxtj2_resume--1");
		return 0;
	}	
#endif
	if(obj == NULL)
	{
		GSE_ERR("kxtj2_1009 null pointer!!\n");
		return -EINVAL;
	}

#ifndef CUSTOM_KERNEL_SENSORHUB
	KXTJ2_1009_power(obj->hw, 1);
#endif
    mutex_lock(&kxtj2_1009_mutex);
#ifdef CUSTOM_KERNEL_SENSORHUB
	if(0 != (err = KXTJ2_1009_SCP_SetPowerMode(enable_status)))
#else
	if(0 != (err = kxtj2_1009_init_client(client, pre_suspend)))
#endif
	{
		GSE_ERR("kxtj2_1009 initialize client fail!!\n");
		mutex_unlock(&kxtj2_1009_mutex);
		return err;        
	}
	atomic_set(&obj->suspend, 0);
    mutex_unlock(&kxtj2_1009_mutex);

	return 0;
}
/*----------------------------------------------------------------------------*/
#else //!defined(CONFIG_HAS_EARLYSUSPEND) || !defined(USE_EARLY_SUSPEND)
/*----------------------------------------------------------------------------*/
static void kxtj2_1009_early_suspend(struct early_suspend *h) 
{
	struct kxtj2_1009_i2c_data *obj = container_of(h, struct kxtj2_1009_i2c_data, early_drv);   
	int err;
	GSE_FUN();    

	if(obj == NULL)
	{
		GSE_ERR("kxtj2_1009 null pointer!!\n");
		return;
	}
	mutex_lock(&kxtj2_1009_mutex);
	atomic_set(&obj->suspend, 1);
#ifdef CUSTOM_KERNEL_SENSORHUB
	if(err = KXTJ2_1009_SCP_SetPowerMode(false))
#else
	if(err = KXTJ2_1009_SetPowerMode(obj->client, false))
#endif
	{
		GSE_ERR("kxtj2_1009 write power control fail!!\n");
		mutex_unlock(&kxtj2_1009_mutex);
		return;
	}
	mutex_unlock(&kxtj2_1009_mutex);

	//sensor_power = false;
#ifndef CUSTOM_KERNEL_SENSORHUB	
	KXTJ2_1009_power(obj->hw, 0);
#endif
}
/*----------------------------------------------------------------------------*/
static void kxtj2_1009_late_resume(struct early_suspend *h)
{
	struct kxtj2_1009_i2c_data *obj = container_of(h, struct kxtj2_1009_i2c_data, early_drv);         
	int err;
	GSE_FUN();

	if(obj == NULL)
	{
		GSE_ERR("kxtj2_1009 null pointer!!\n");
		return;
	}

#ifndef CUSTOM_KERNEL_SENSORHUB
	KXTJ2_1009_power(obj->hw, 1);
#endif
	mutex_lock(&kxtj2_1009_mutex);
#ifdef CUSTOM_KERNEL_SENSORHUB
	if(err = KXTJ2_1009_SCP_SetPowerMode(enable_status))
#else
	if(err = kxtj2_1009_init_client(obj->client, 0))
#endif
	{
		GSE_ERR("kxtj2_1009 initialize client fail!!\n");
		mutex_unlock(&kxtj2_1009_mutex);
		return;        
	}
	atomic_set(&obj->suspend, 0); 
     
	mutex_unlock(&kxtj2_1009_mutex);
 
}
/*----------------------------------------------------------------------------*/
#endif //!defined(CONFIG_HAS_EARLYSUSPEND) || !defined(USE_EARLY_SUSPEND)
/*----------------------------------------------------------------------------*/

// if use  this typ of enable , Gsensor should report inputEvent(x, y, z ,stats, div) to HAL
static int kxtj2_1009_open_report_data(int open)
{
	//should queuq work to report event if  is_report_input_direct=true
	return 0;
}

// if use  this typ of enable , Gsensor only enabled but not report inputEvent to HAL

static int kxtj2_1009_enable_nodata(int en)
{
    int err = 0;
   //Added by yuduan.xie for significant on 2017.2.27 begin
	int value = en;
	if(value == 1)
	{
		enable_status = true;
	}
	else
	{
		enable_status = false;
	}
	//Added by yuduan.xie for significant on 2017.2.27 end
   
	if(((en == 0) && (sensor_power == false)) ||((en == 1) && (sensor_power == true)))
	{
		enable_status = sensor_power;
		GSE_LOG("Gsensor device have updated!\n");
	}

	
#ifdef SIGNIFICANT
	else if(!significant_enable_status)
	{
		err = KXTJ2_1009_SetPowerMode( obj_i2c_data->client, enable_status);	
        return 0;		
	}
#endif
		//enable_status = !sensor_power;
#if 0
		if (atomic_read(&obj_i2c_data->suspend) == 0)
		{
#ifdef CUSTOM_KERNEL_SENSORHUB
            err = KXTJ2_1009_SCP_SetPowerMode(enable_status);
#else//#ifdef CUSTOM_KERNEL_SENSORHUB
			err = KXTJ2_1009_SetPowerMode(obj_i2c_data->client, enable_status);
#endif
			GSE_LOG("Gsensor not in suspend KXTJ2_1009_SetPowerMode!, enable_status = %d\n",enable_status);
		}
		else
		{
			GSE_LOG("Gsensor in suspend and can not enable or disable!enable_status = %d\n",enable_status);
		}
#endif

    if(err != KXTJ2_1009_SUCCESS)
	{
		GSE_ERR("kxtj2_1009_enable_nodata fail!\n");
		return -1;
	}

   // GSE_ERR("kxtj2_1009_enable_nodata OK!\n");
	return 0;
}

static int kxtj2_1009_set_delay(u64 ns)
{
    int err = 0;
    int value;
#ifdef CUSTOM_KERNEL_SENSORHUB
    SCP_SENSOR_HUB_DATA req;
    int len;
#else//#ifdef CUSTOM_KERNEL_SENSORHUB
    int sample_delay;
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB

    value = (int)ns/1000/1000;

#ifdef CUSTOM_KERNEL_SENSORHUB
    req.set_delay_req.sensorType = ID_ACCELEROMETER;
    req.set_delay_req.action = SENSOR_HUB_SET_DELAY;
    req.set_delay_req.delay = value;
    len = sizeof(req.activate_req);
    err = SCP_sensorHub_req_send(&req, &len, 1);
    if (err)
    {
        GSE_ERR("kxtj2_1009 SCP_sensorHub_req_send!\n");
        return err;
    }
#else//#ifdef CUSTOM_KERNEL_SENSORHUB    
	if(value <= 5)
	{
		sample_delay = KXTJ2_1009_BW_200HZ;
	}
	else if(value <= 10)
	{
		sample_delay = KXTJ2_1009_BW_100HZ;
	}
	else
	{
		sample_delay = KXTJ2_1009_BW_50HZ;
	}

	mutex_lock(&kxtj2_1009_mutex);
	err = KXTJ2_1009_SetBWRate(obj_i2c_data->client, sample_delay);
	mutex_unlock(&kxtj2_1009_mutex);
	if(err != KXTJ2_1009_SUCCESS ) //0x2C->BW=100Hz
	{
		GSE_ERR("kxtj2_1009 Set delay parameter error!\n");
		return -1;
	}

	if(value >= 50)
	{
		atomic_set(&obj_i2c_data->filter, 0);
	}
	else
	{	
	#if defined(CONFIG_KXTJ2_1009_LOWPASS)
		priv->fir.num = 0;
		priv->fir.idx = 0;
		priv->fir.sum[KXTJ2_1009_AXIS_X] = 0;
		priv->fir.sum[KXTJ2_1009_AXIS_Y] = 0;
		priv->fir.sum[KXTJ2_1009_AXIS_Z] = 0;
		atomic_set(&priv->filter, 1);
	#endif
	}
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB
    
    GSE_LOG("kxtj2_1009_set_delay (%d)\n",value);

	return 0;
}
/*
static int kxtj2_1009_set_batch(int flags, int64_t period_ns, int64_t timeout)
{
    int err = 0;

#ifdef CUSTOM_KERNEL_SENSORHUB
    uint32_t period_ms;
    uint32_t timeout_ms;
    SCP_SENSOR_HUB_DATA req;
    int len;

    period_ms = period_ns/1000000;
    timeout_ms = timeout/1000000;

    req.batch_req.sensorType = ID_ACCELEROMETER;
    req.batch_req.action = SENSOR_HUB_BATCH;
    req.batch_req.flag = flags;
    req.batch_req.period_ms = period_ms;
    req.batch_req.timeout_ms = timeout_ms;

    len = sizeof(req.batch_req);

    err = SCP_sensorHub_req_send(&req, &len, 1);
    if (err)
    {
        GSE_ERR("kxtj2_1009 SCP_sensorHub_req_send!\n");
        return err;
    }
#endif

    return err;
}
*/

static int kxtj2_1009_get_data(int* x ,int* y,int* z, int* status)
{
#ifdef CUSTOM_KERNEL_SENSORHUB
		SCP_SENSOR_HUB_DATA req;
		int len;
		int err = 0;
#else
		char buff[KXTJ2_1009_BUFSIZE];
#endif

#ifdef CUSTOM_KERNEL_SENSORHUB
		req.get_data_req.sensorType = ID_ACCELEROMETER;
		req.get_data_req.action = SENSOR_HUB_GET_DATA;
		len = sizeof(req.get_data_req);
		err = SCP_sensorHub_req_send(&req, &len, 1);
		if (err)
		{
			GSE_ERR("kxtj2_1009 SCP_sensorHub_req_send!\n");
			return err;
		}

		if (ID_ACCELEROMETER != req.get_data_rsp.sensorType ||
			SENSOR_HUB_GET_DATA != req.get_data_rsp.action ||
			0 != req.get_data_rsp.errCode)
		{
			GSE_ERR("kxtj2_1009 error : %d\n", req.get_data_rsp.errCode);
			return req.get_data_rsp.errCode;
		}

		//sscanf(buff, "%x %x %x", req.get_data_rsp.int16_Data[0], req.get_data_rsp.int16_Data[1], req.get_data_rsp.int16_Data[2]);
		*x = req.get_data_rsp.int16_Data[0];
		*y = req.get_data_rsp.int16_Data[1];
		*z = req.get_data_rsp.int16_Data[2];
		GSE_ERR("kxtj2_1009 x = %d, y = %d, z = %d\n", *x, *y, *z);
		*status = SENSOR_STATUS_ACCURACY_MEDIUM;

		if(atomic_read(&obj_i2c_data->trace) & ADX_TRC_RAWDATA)
		{
			//show data
		}	
#else
	mutex_lock(&kxtj2_1009_mutex);
	KXTJ2_1009_ReadSensorData(obj_i2c_data->client, buff, KXTJ2_1009_BUFSIZE);
	mutex_unlock(&kxtj2_1009_mutex);
	sscanf(buff, "%x %x %x", x, y, z);				
	*status = SENSOR_STATUS_ACCURACY_MEDIUM;
#endif
	return 0;
}

static int kxtj2_1009_set_batch(int flag, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
	return 0;
}

static int kxtj2_1009_set_flush(void)
{
	return acc_flush_report();
}
//Begin deleted by zhi.chen for gsensor calibration  2018/03/02
#if 0
static int kxtj2_1009_factory_enable_sensor(bool enabledisable, int64_t sample_periods_ms)
{
	int err;

	err = kxtj2_1009_enable_nodata(enabledisable == true ? 1 : 0);
	if (err) {
		GSE_ERR("%s enable sensor failed!\n", __func__);
		return -1;
	}

	return 0;
}
static int kxtj2_1009_factory_get_data(int32_t data[3], int *status)
{

	return kxtj2_1009_get_data(&data[0], &data[1], &data[2], status);

}
static int kxtj2_1009_factory_get_raw_data(int32_t data[3])
{
	char strbuf[KXTJ2_1009_BUFSIZE] = { 0 };

	KXTJ2_1009_ReadRawData(kxtj2_1009_i2c_client, strbuf);
	GSE_LOG("support kxtj2_1009_factory_get_raw_data!\n");
	return 0;
}
static int kxtj2_1009_factory_enable_calibration(void)
{
	return 0;
}
static int kxtj2_1009_factory_clear_cali(void)
{
	int err = 0;

	err = KXTJ2_1009_ResetCalibration(kxtj2_1009_i2c_client);
	
	if (err) {
		GSE_ERR("kxtj2_1009_ResetCalibration failed!\n");
		return -1;
	}
	return 0;
}
static int kxtj2_1009_factory_set_cali(int32_t data[3])
{
	int err = 0;
	int cali[3] = { 0 };  //  delete for zhi.chen for bringup

	obj_i2c_data->cali_sw[KXTJ2_1009_AXIS_X] = data[0] * gsensor_gain.x / GRAVITY_EARTH_1000;
	obj_i2c_data->cali_sw[KXTJ2_1009_AXIS_Y] = data[1] * gsensor_gain.y / GRAVITY_EARTH_1000;
	obj_i2c_data->cali_sw[KXTJ2_1009_AXIS_Z] = data[2] * gsensor_gain.z / GRAVITY_EARTH_1000;

	err = KXTJ2_1009_WriteCalibration(kxtj2_1009_i2c_client, cali ); //	delete for zhi.chen for bringup
	if (err) {
		GSE_ERR("kxtj2_1009_WriteCalibration failed!\n");
		return -1;
	}
	return 0;
}
static int kxtj2_1009_factory_get_cali(int32_t data[3])
{
	data[0] = obj_i2c_data->cali_sw[KXTJ2_1009_AXIS_X] * GRAVITY_EARTH_1000 / GRAVITY_EARTH_1000;
	data[1] = obj_i2c_data->cali_sw[KXTJ2_1009_AXIS_Y] * GRAVITY_EARTH_1000 / GRAVITY_EARTH_1000;
	data[2] = obj_i2c_data->cali_sw[KXTJ2_1009_AXIS_Z] * GRAVITY_EARTH_1000 / GRAVITY_EARTH_1000;
	return 0;
}
static int kxtj2_1009_factory_do_self_test(void)
{
	return 0;
}

static struct accel_factory_fops kxtj2_1009_factory_fops = {
	.enable_sensor = kxtj2_1009_factory_enable_sensor,
	.get_data = kxtj2_1009_factory_get_data,
	.get_raw_data = kxtj2_1009_factory_get_raw_data,
	.enable_calibration = kxtj2_1009_factory_enable_calibration,
	.clear_cali = kxtj2_1009_factory_clear_cali,
	.set_cali = kxtj2_1009_factory_set_cali,
	.get_cali = kxtj2_1009_factory_get_cali,
	.do_self_test = kxtj2_1009_factory_do_self_test,
};

static struct accel_factory_public kxtj2_1009_factory_device = {
	.gain = 1,
	.sensitivity = 1,
	.fops = &kxtj2_1009_factory_fops,
};
#endif 
//End deleted by zhi.chen for gsensor calibration 2018/03/02
/*----------------------------------------------------------------------------*/
//extern void app_get_g_sensor_name(char *g_name); 
static int kxtj2_1009_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct kxtj2_1009_i2c_data *obj;
	struct acc_control_path ctl={0};
    struct acc_data_path data={0};
	int err = 0;
	GSE_FUN();
     client->addr = 0x0E;
	/*
	hw = get_accel_dts_func(client->dev.of_node, hw);
	if (!hw) {
		GSE_ERR("kxtj2_1009 get cust_baro dts info fail\n");
		goto exit;
	}
	*/
	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	
	memset(obj, 0, sizeof(struct kxtj2_1009_i2c_data));

	obj->hw = hw;
	GSE_ERR("kxtj2_1009 direction condition before if(0 != (err = hwmsen_get_convert(obj->hw->direction, &obj->cvt))) \n");
	if(0 != (err = hwmsen_get_convert(obj->hw->direction, &obj->cvt)))
	{
		GSE_ERR("kxtj2_1009 invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

#ifdef CUSTOM_KERNEL_SENSORHUB
	INIT_WORK(&obj->irq_work, kxtj2_1009_irq_work);
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB

	obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);
	
	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);

#ifdef CUSTOM_KERNEL_SENSORHUB
	obj->SCP_init_done = 0;
#endif//#ifdef CUSTOM_KERNEL_SENSORHUB

#ifdef CONFIG_KXTJ2_1009_LOWPASS
	if(obj->hw->firlen > C_MAX_FIR_LENGTH)
	{
		atomic_set(&obj->firlen, C_MAX_FIR_LENGTH);
	}	
	else
	{
		atomic_set(&obj->firlen, obj->hw->firlen);
	}
	
	if(atomic_read(&obj->firlen) > 0)
	{
		atomic_set(&obj->fir_en, 1);
	}
	
#endif

	kxtj2_1009_i2c_client = new_client;
//GSE_ERR("kxtj2_1009 init client before if(0 != (err = kxtj2_1009_init_client(new_client, 1))) \n");
	if(0 != (err = kxtj2_1009_init_client(new_client, 1)))
	{
		goto exit_init_failed;
	}
	
	err = misc_register(&kxtj2_1009_device);     //Begin Add by  zhi.chen
	if(0 == err )
	{
		GSE_ERR("kxtj2_1009_device register successs\n");
	}
	if(0 != err )
	{
		GSE_ERR("kxtj2_1009_device register failed\n");
		goto exit_misc_device_register_failed;
	}

   //Begin deleted by zhi.chen for gsensor calibration  2018/03/02
   #if 0
	err = accel_factory_device_register(&kxtj2_1009_factory_device);
	if (err) {
		GSE_ERR("acc_factory register failed.\n");
		goto exit_misc_device_register_failed;
	}
	#endif
	//End deleted by lanying.he for gsensor calibration  2018/03/02
	GSE_ERR("kxtj2_1009 creat attr before if(0 != (err = kxtj2_1009_create_attr(&kxtj2_1009_init_info.platform_diver_addr->driver))) \n");
	
	#ifdef SIGNIFICANT
       INIT_WORK(&obj->eint_work,  eintWork);
    #endif
	
	if(0 != (err = kxtj2_1009_create_attr(&kxtj2_1009_init_info.platform_diver_addr->driver)))
	{
		GSE_ERR("kxtj2_1009 create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

	ctl.open_report_data= kxtj2_1009_open_report_data;
	ctl.enable_nodata = kxtj2_1009_enable_nodata;
	ctl.set_delay  = kxtj2_1009_set_delay;
    ctl.batch = kxtj2_1009_set_batch;
	ctl.flush = kxtj2_1009_set_flush;
	ctl.is_report_input_direct = false;
#ifdef CUSTOM_KERNEL_SENSORHUB
    ctl.is_support_batch = obj->hw->is_batch_supported;
#else
    ctl.is_support_batch = false;
#endif

	err = acc_register_control_path(&ctl);
	if(err)
	{
	 	GSE_ERR("kxtj2_1009 register acc control path err\n");
		goto exit_create_attr_failed;
	}
//GSE_ERR("kxtj2_1009 register control path err = acc_register_control_path(&ctl); \n");
	data.get_data = kxtj2_1009_get_data;
	data.vender_div = 1000;
	err = acc_register_data_path(&data);
	if(err)
	{
	 	GSE_ERR("kxtj2_1009 register acc data path err\n");
		goto exit_create_attr_failed;
	}
#if 0
	err = batch_register_support_info(ID_ACCELEROMETER,ctl.is_support_batch, 1000, 0);
    if(err)
    {
        GSE_ERR("kxtj2_1009 register gsensor batch support err = %d\n", err);
        goto exit_create_attr_failed;
    }
#endif

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(USE_EARLY_SUSPEND)
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = kxtj2_1009_early_suspend,
	obj->early_drv.resume   = kxtj2_1009_late_resume,    
	register_early_suspend(&obj->early_drv);
#endif 

    //app_get_g_sensor_name("kxtj2_1009_3axis");
	
    kxtj2_1009_init_flag =0;
#ifdef SIGNIFICANT
	kxtj2_acc_init_flag =0;
#endif
	GSE_LOG("%s: OK\n", __func__);    
	return 0;

	exit_create_attr_failed:
	misc_deregister(&kxtj2_1009_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	kfree(obj);
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);
    kxtj2_1009_init_flag =-1;
	kxtj2_acc_init_flag =-1;
	return err;
}

/*----------------------------------------------------------------------------*/
static int kxtj2_1009_i2c_remove(struct i2c_client *client)
{
	int err = 0;	
	
	if(0 != (err = kxtj2_1009_delete_attr(&(kxtj2_1009_init_info.platform_diver_addr->driver))))
	{
		GSE_ERR("kxtj2_1009_delete_attr fail: %d\n", err);
	}
	
/*	if(0 != (err = misc_deregister(&kxtj2_1009_device)))
	{
		GSE_ERR("kxtj2_1009 misc_deregister fail: %d\n", err);
	}
*/

	kxtj2_1009_i2c_client = NULL;
	i2c_unregister_device(client);
	//accel_factory_device_deregister(&kxtj2_1009_factory_device);  
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int kxtj2_1009_remove(void)
{
    struct acc_hw *hw = get_cust_acc();

    GSE_FUN();    
    KXTJ2_1009_power(hw, 0);    
    i2c_del_driver(&kxtj2_1009_i2c_driver);
    return 0;
}
//Begin added by zhi.chen for significant by 2018/03/05 
#ifdef SIGNIFICANT
static irqreturn_t eint_func(int irq , void *desc) //liupeng begin
{
    struct kxtj2_1009_i2c_data *priv = obj_i2c_data;
//  GSE_LOG("GSE into  interrupt liupeng !\n");
    disable_irq_nosync(obj_i2c_data->irq); //liupeng
    schedule_work(&priv->eint_work);
    return IRQ_HANDLED;
}

static void eintWork(struct work_struct *work)
{
   u8 databuf[2] = {0}; 
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;
	//GSE_ERR("KXTJ2_EintWork--1\n");
	if(obj == NULL)
	{
		GSE_ERR("obj_i2c_data is null pointer!!\n");
		goto kxtj2_eint_work_exit;
	}	
		if(hwmsen_read_byte(obj->client, 0x1A, databuf))   
		{
			GSE_ERR("%s read KXTJ2_1009_FUNC_CK_GATE register err!\n", __func__);
			
		}

    //   GSE_ERR("KXTJ2_1009_EintWork--2+databuf[0] = %d\n",databuf[0]);
	//if(0x2 & databuf[0])
	//{
		//add the action when receive the significant motion
		step_notify(TYPE_SIGNIFICANT);   //delete by zhi.chen.hz by 2018/03/05  sure to configure
	//}

	memset(databuf,0,sizeof(u8)*2);
kxtj2_eint_work_exit:
	//mt_eint_unmask(CUST_EINT_kxtj2_1009_NUM);
	
	enable_irq(obj_i2c_data->irq);  //liupeng
}

int kxtj2_setup_eint(void)
{
    int ret;
    u32 ints[2] = { 0, 0 };
    obj_i2c_data->irq_node = of_find_compatible_node(NULL,NULL,"mediatek, GSE_1-eint");
    if (obj_i2c_data->irq_node)
    {
        of_property_read_u32_array(obj_i2c_data->irq_node, "debounce", ints, ARRAY_SIZE(ints));
        //of_property_read_u32_array(node, "interrupts", ints1, ARRAY_SIZE(ints1));

        gpio_set_debounce(ints[0], ints[1]);  //delete by zhi.chen.hz by 2018/03/05  sure to configure
        obj_i2c_data->irq = irq_of_parse_and_map(obj_i2c_data->irq_node, 0);
        if (!obj_i2c_data->irq)
        {
            GSE_ERR("irq_of_parse_and_map fail!!\n");
            return -EINVAL;
        }
        ret = request_irq(obj_i2c_data->irq, eint_func,  IRQF_TRIGGER_HIGH, "mediatek, GSE_1-eint", NULL);
        if (ret != 0)
        {
            GSE_ERR("%s  EINT IRQ LINE NOT AVAILABLE\n",__func__);
        }
        else
        {
            GSE_ERR("%s set EINT finished, obj_i2c_data->irq=%d, acceldebounce=??\n",
                     __func__,obj_i2c_data->irq);
        }
    }
    else
    {
        GSE_ERR("%s can't find compatible node\n", __func__);
    }
    enable_irq_wake(obj_i2c_data->irq);
    return 0;
}


int kxtj2_set_acc_interrupter(bool flag)
{
    int res =0;
      u8 databuf[2] = {0};
	struct kxtj2_1009_i2c_data *obj = obj_i2c_data;

	if(flag)
	{
		GSE_ERR("KXTJ2_1009_enable_significant flag = %d--1\n",flag);
		
	       significant_enable_status = true;
		   
	        KXTJ2_1009_SetPowerMode(obj->client, false); 
		

               
		res =  KXTJ2_1009_setMotionDetectFunc(obj->client,true);
		if(res !=0)
		{
		    GSE_ERR("KXTJ2_1009_SetMotionDetectEnable faild\n");
		}

		res = KXTJ2_1009_SetIntEnable(obj->client,true);
                if(res !=0)
		{
		    GSE_ERR("KXTJ2_1009_SetIntEnable faild\n");
		}

		KXTJ2_1009_SetPowerMode(obj->client, true); 
		if(hwmsen_read_byte(obj->client, 0x1A, databuf))   
		{
			GSE_ERR("%s read KXTJ2_1009_FUNC_CK_GATE register err!\n", __func__);
			
		}
	}
	else
	{
		GSE_ERR("KXTJ2_1009_enable_significant flag = %d--2\n",flag);
		
		significant_enable_status = false;
		
		KXTJ2_1009_SetPowerMode(obj->client, false); 
		res = KXTJ2_1009_SetIntEnable(obj->client,false);
                if(res !=0)
		{
		    GSE_ERR("bma2xx_SetIntEnable faild\n");
		}
		res =  KXTJ2_1009_setMotionDetectFunc(obj->client,false);
		if(res !=0)
		{
		    GSE_ERR("bma2xx_SetMotionDetectEnable faild\n");
		}
	
		KXTJ2_1009_SetPowerMode(obj->client, true); 
		if(!enable_status)
		{
			KXTJ2_1009_SetPowerMode(obj->client, false); 
		}
	}
	
	return res;
}
#endif
//End added by zhi.chen for significant by 2018/03/05
extern char gsensor_module_name[256];
static int  kxtj2_1009_local_init(void)
{
	struct acc_hw *hw = get_cust_acc();

	KXTJ2_1009_power(hw, 1);
	if(i2c_add_driver(&kxtj2_1009_i2c_driver))
	{
		GSE_ERR("kxtj2_1009 add driver error\n");
		return -1;
	}
	if(-1 == kxtj2_1009_init_flag)
	{
	   return -1;
	}
    sprintf(gsensor_module_name,"KXTJ2:KIONIX:NA");//add by zhi.chen for sencond gsensor deviceinfo
	return 0;
}

/*----------------------------------------------------------------------------*/
static int __init kxtj2_1009_init(void)
{
 int err = -1;
   struct device_node *node = NULL;
	GSE_FUN();
	node = of_find_compatible_node(NULL, NULL, "mediatek,kxtj3");
	 if(node!=NULL)
    {
        err = get_accel_dts_func(node, hw);
        if (err < 0) {
		    ACC_PR_ERR("get cust_baro dts info fail\n");
		    
	    }
    }
	acc_driver_add(&kxtj2_1009_init_info);
	return 0;    
}
/*----------------------------------------------------------------------------*/
static void __exit kxtj2_1009_exit(void)
{
	GSE_FUN();
}
/*----------------------------------------------------------------------------*/
module_init(kxtj2_1009_init);
module_exit(kxtj2_1009_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("KXTJ2_1009 I2C driver");
MODULE_AUTHOR("Dexiang.Liu@mediatek.com");