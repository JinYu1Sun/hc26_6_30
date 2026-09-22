#ifndef _AHRS_H_
#define _AHRS_H_

#include <stdint.h>
#include <stdbool.h>

typedef int8_t          int8;
typedef int16_t        int16;
typedef int32_t	       int32;
typedef uint8_t	       uint8;
typedef uint16_t       uint16;
typedef uint32_t	   uint32;
typedef uint64_t 	   uint64;

#pragma pack(1)
typedef struct st_ahrs_header
{
   uint8 uc_head;
   uint8 uc_msg_type;
   uint8 uc_access:1; // match E_ACCESS_PROPERTY    
   uint8 uc_ack:2; // match E_MSG_ACK
   uint8 uc_reserve:5;   
   uint8 uc_msg_extend;
}ST_AHRS_HEADER;
#pragma pack()

#define AHRS_COM_MSG_SIZE 36
#define AHRS_HEADER_SIZE sizeof(ST_AHRS_HEADER)
#define AHRS_SUM_OFFSET_SIZE 2
#define AHRS_CHECK_SUM_SIZE (AHRS_COM_MSG_SIZE - AHRS_SUM_OFFSET_SIZE)
#define BASE_PLAYLOAD_SIZE (AHRS_COM_MSG_SIZE - AHRS_HEADER_SIZE - AHRS_SUM_OFFSET_SIZE)

#pragma pack(1)
typedef struct st_euler_data
{
//    float f_roll;
   float f_pitch;
   float f_roll;
   float f_yaw;
}ST_EULER_DATA;
#pragma pack()

#pragma pack(1)
typedef struct st_quaternions_data    //四元数
{
   float f_q0;
   float f_q1;
   float f_q2;
   float f_q3;
}ST_QUATERNIONS_DATA; 
#pragma pack()

#pragma pack(1)
typedef struct st_axis_data           //各轴数据
{
   float f_axis_x;
   float f_axis_y;
   float f_axis_z;
}ST_AXIS_DATA;
#pragma pack()

#pragma pack(1)
typedef struct st_ahrs_data         //ahrs_data里包含欧拉角，四元数数据
{
	ST_EULER_DATA st_euler_data;              //12
	ST_QUATERNIONS_DATA st_quaternions_data;  //16
	// float AGV_TRACK[3];
}ST_AHRS_DATA;                                //28
#pragma pack()

#pragma pack(1)
typedef struct st_ahrs_all_data         //所有数据
{	
	ST_EULER_DATA st_euler_data;
	ST_AXIS_DATA st_acc_data;
    ST_AXIS_DATA st_gyro_data;
	ST_AXIS_DATA st_mag_data;
}ST_AHRS_ALL_DATA; //12*4=48
#pragma pack()

#pragma pack(1)
typedef struct st_ahrs_raw_data    //原数据里则是包含加速度，陀螺仪，磁力计三种硬件的原始数据。
{
	ST_AXIS_DATA st_acc_data;  //12
	ST_AXIS_DATA st_gyro_data; //12
	ST_AXIS_DATA st_mag_data;  //12
}ST_AHRS_RAW_DATA;             //36
#pragma pack()

#pragma pack(1)
typedef struct st_software_version
{
   uint8 uac_software_version[8];   
}ST_SOFTWARE_VERSION;
#pragma pack()

#pragma pack(1)
typedef struct st_mag_cali_param
{
   float f_mag_cali_param[9];   
}ST_MAG_CALI_PARAM_MSG;
#pragma pack()

#pragma pack(1)
typedef struct st_ahrs_com_msg
{
    ST_AHRS_HEADER st_header; /*size is 4 bytes //4 */
    union
    {
        /* base type */
        uint8 uc_mag_cali_percent;              //1
        ST_AHRS_RAW_DATA st_ahrs_raw_data;      //float 4 *9 =36    
        ST_AHRS_DATA st_ahrs_data;              //4*4+4*3 = 28 
        ST_SOFTWARE_VERSION st_software_version;//1
        ST_MAG_CALI_PARAM_MSG st_mag_cali_param;//4
        uint8 auc_playload[BASE_PLAYLOAD_SIZE]; //1 *the max playload size 36bytes*/        
    }u_palyload;
    uint16 us_check_num;                        //2
}ST_AHRS_COM_MSG; /*the base communication message,match E_BASE_TYPE*/

void CopeSerialData42(char ucData[], unsigned short usLength, ST_AHRS_DATA &AHRS, ST_AHRS_RAW_DATA &RAW_AHRS, int& get_data);
void CopeSerialData54(char ucData[], unsigned short usLength, ST_AHRS_ALL_DATA &AHRS, ST_AHRS_RAW_DATA &RAW_AHRS, int& get_data);

#pragma pack()

#define using_hex2 1

#pragma pack(1) 
#if using_hex1
#define DATA_LEN 93
#define HEAD_00 01
typedef struct gnavp16  //
{
	uint32 head;  //7+15*4+3*2+8*2+4 =93
  // uint8 head1;
	// uint8 head2;
	// uint8 head3;
	// uint8 head4;
	uint8 hour;
	uint8 min;
	uint8 sec;
	uint16 msec;
	float yaw;
	float pitch;
	float roll;
	double latitude;
	double longitude;
	float  altitude;
	float  Ve;
	float  Vn;
	float  Vu;
	float  baseline;
	uint8  Nsv1;
	uint8  Nsv2;
	uint8  status;
	uint8  speed_status;
	float  vehicle_speed;
	float  accx;
	float  accy;
	float  accz;
	float  gyrox;
	float  gyroy;
	float  gyroz;
	uint16 reserved;
	uint16 check_sum;
}GNAVP16_t;
#endif
#if using_hex2
#define DATA_LEN 101
#define HEAD_00 02
typedef struct gnavp16  
{
	uint32 head;  
	uint16 week;
	double sec;
	float yaw;
	float pitch;
	float roll;
	double latitude;
	double longitude;
	float  altitude;
	float  Ve;
	float  Vn;
	float  Vu;
	float  baseline;
	uint8  Nsv1;
	uint8  Nsv2;
	uint8  gps_status;
	uint8  heading_status;
	uint8  status;
	uint8  vehicle_align;
	uint8  rtcm_status;
	uint8  reserved;
	uint8  speed_status;
	float  vehicle_speed;
	float  accx;
	float  accy;
	float  accz;
	float  gyrox;
	float  gyroy;
	float  gyroz;
	uint16 check_sum;
}GNAVP16_t;
#endif
#endif /*_AHRS_H_*/
