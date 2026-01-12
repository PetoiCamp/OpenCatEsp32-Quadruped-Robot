#ifndef _IMU42670P_H_
#define _IMU42670P_H_
#include "ICM42670P.h"
// You need to install the ICM42670p library via Arduino IDE's library manager

#define MEAN_FILTER_SIZE 4
class imu42670p : public ICM42670 {
public:
  float yaw, pitch, roll, yawLag, yawDrift;
  float ax_real, ay_real, az_real, gx_real, gy_real, gz_real;
  float ypr[3];
  float yprHistory[MEAN_FILTER_SIZE][3];
  int8_t index;
  bool firstRound;
  float a_real[3];
  float offset_accel[3];  // imu offset data
  float offset_gyro[3];

  inv_imu_sensor_event_t imuData;  // imu raw data
  inv_imu_sensor_event_t prevData;

  // init
  imu42670p(TwoWire &i2c, bool address_lsb);

  // set
  int init(uint16_t odr, uint16_t accel_fsr, uint16_t gyro_fsr);

  // get
  float getAccelRatio(uint8_t accel_fsr);
  float getGyroRatio(uint8_t gyro_fsr);
  void getOffset(int num);

  // print IMU data
  float getTemperature();
  void transformIMUData();
  void transformIMUDataWithOffset();
  void getImuGyro();
  void printRealworldData();

  // fusion functio
  void MadgwickQuaternionUpdate(float ax, float ay, float az, float gyrox, float gyroy, float gyroz, float deltaT);

private:
  uint32_t now = 0;
  float deltaT = 0.0;
  uint32_t lastUpdate, firstUpdate;
  float accel_ratio;                        // accel FS_SEL
  float gyro_ratio;                         // gyro FS_SEL
  float q[4] = { 1.0f, 0.0f, 0.0f, 0.0f };  // vector to hold quaternion
};
#endif
