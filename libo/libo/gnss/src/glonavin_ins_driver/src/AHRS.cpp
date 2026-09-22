#include "AHRS.h"
#include "string.h"
#include <stdio.h>
#include <iostream>


typedef union
{
	float fdata;
	unsigned long ldata;
}FloatLongType;

void CopeSerialData42(char ucData[], unsigned short usLength, ST_AHRS_DATA& AHRS, ST_AHRS_RAW_DATA& RAW_AHRS, int& get_data)
{
  static unsigned char chrTemp[2000];
  static unsigned char ucRxCnt = 0;
  static unsigned short usRxLength = 0;
  get_data = 0;
  memcpy(chrTemp, ucData, usLength);
  usRxLength += usLength;
  while (usRxLength >= 42)
  {

    if (chrTemp[0] != 0x55)
    {
      usRxLength--;
      memcpy(&chrTemp[0], &chrTemp[1], usRxLength);
      continue;
    }
    if(chrTemp[1] != 0xD1)
    {
      usRxLength--;
      memcpy(&chrTemp[0], &chrTemp[2], usRxLength);
      continue;
    }
    switch (chrTemp[3])
    {
      case 0x01://
      {
        memcpy(&RAW_AHRS,&chrTemp[4],36);
        get_data = 1;
        break;
      }
      case 0x02://02上报两个数据分别为欧拉角和四元数
      {
        memcpy(&AHRS, &chrTemp[4], 28);
        get_data = 2;
        break;
      }
    }
    usRxLength -= 42;
    memcpy(&chrTemp[0], &chrTemp[11], usRxLength);
  }
}
//从缓存读取数据到本司imu 对应格式。
void CopeSerialData54(char ucData[], unsigned short usLength, ST_AHRS_ALL_DATA& AHRS, ST_AHRS_RAW_DATA& RAW_AHRS, int& get_data)
{
  static unsigned char chrTemp[2000];
  static unsigned char ucRxCnt = 0;
  static unsigned short usRxLength = 0;
  get_data = 0;
  memcpy(chrTemp, ucData, usLength);   //usLength数据长度
  usRxLength += usLength;
  while (usRxLength >= 54)
  {

    if (chrTemp[0] != 0x55)//数据起始位
    {
      usRxLength--;
      memcpy(&chrTemp[0], &chrTemp[1], usRxLength);
      continue;
    }
    if(chrTemp[1] != 0xD1)//如果数据不在起始位，则依次让数据向前移位，以保证起始位对齐
    {
      usRxLength--;
      memcpy(&chrTemp[0], &chrTemp[2], usRxLength);
      continue;
    }
    switch (chrTemp[3])                 //头文件总体四字节，起始位16进制数分别是55 d1 00 02
    {
      case 0x01:                        //获取欧拉角及四元数
      {
        memcpy(&AHRS,&chrTemp[4],28);
        get_data = 1;
        break;
      }
      case 0x02:                        //有欧拉角，及三种原始数据
      {
        memcpy(&AHRS, &chrTemp[4], 48);
       // memcpy(&RAW_AHRS, &chrTemp[16], 36);
        get_data = 2;
        break;
      }
    }
    usRxLength -= 54;
    memcpy(&chrTemp[0], &chrTemp[54], usRxLength);
  }

}
