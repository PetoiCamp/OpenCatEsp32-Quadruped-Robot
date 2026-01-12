#include "petoi_icm42670p.h"

imu42670p::imu42670p(TwoWire &i2c, bool address_lsb)
    : ICM42670(i2c, address_lsb)
{
  Serial.println("Constructor"); // why is it unused ("Constructor" is not printed during bootup)
}

int imu42670p::init(uint16_t odr, uint16_t accel_fsr, uint16_t gyro_fsr)
{
  int rc = 0;
  rc |= inv_imu_set_accel_fsr(&icm_driver, accel_fsr_g_to_param(accel_fsr));
  rc |= inv_imu_set_accel_frequency(&icm_driver, accel_freq_to_param(odr));
  rc |= inv_imu_enable_accel_low_noise_mode(&icm_driver);

  rc |= inv_imu_set_gyro_fsr(&icm_driver, gyro_fsr_dps_to_param(gyro_fsr));
  rc |= inv_imu_set_gyro_frequency(&icm_driver, gyro_freq_to_param(odr));
  rc |= inv_imu_enable_gyro_low_noise_mode(&icm_driver);

  // update accel ratio
  accel_ratio = 32768 / accel_fsr;
  // update gyro ratio
  gyro_ratio = 131.0 / (gyro_fsr / 250);
  yawDrift = 0;
  index = 0;
  for(int8_t h = 0; h < MEAN_FILTER_SIZE; h++)
    yprHistory[h][0] = yprHistory[h][1] = yprHistory[h][2] = 0;
  firstRound = true;
  return rc;
}

void imu42670p::getOffset(int num)
{
  inv_imu_sensor_event_t temp;

  Serial.println("Start IMU calibration...");
  Serial.println("Please put the sensor on a leveled plane!");
  Serial.println("Calculate mean");
  for (int i = 0; i < num; i++)
  {
    getDataFromRegisters(temp);
    offset_accel[0] += temp.accel[0];
    offset_accel[1] += temp.accel[1];
    offset_accel[2] += temp.accel[2];
    offset_gyro[0] += temp.gyro[0];
    offset_gyro[1] += temp.gyro[1];
    offset_gyro[2] += temp.gyro[2];
    Serial.print(temp.gyro[0]);
    Serial.print('\t');
    Serial.print(temp.gyro[1]);
    Serial.print('\t');
    Serial.print(temp.gyro[2]);
    Serial.println('\t');
    delay(5);
  }

  offset_accel[0] = offset_accel[0] / num;
  offset_accel[1] = offset_accel[1] / num;
  offset_accel[2] = offset_accel[2] / num - accel_ratio; // Z axis with G
  offset_gyro[0] = offset_gyro[0] / num;
  offset_gyro[1] = offset_gyro[1] / num;
  offset_gyro[2] = offset_gyro[2] / num;

  //  Serial.print("ICM42670 offset:\t");
  //  for (byte i = 0; i < 3; i++) {
  //    Serial.print(offset_gyro[i]);
  //    Serial.print('\t');
  //  }
  //  Serial.println();
}

float imu42670p::getTemperature()
{
  float temp = imuData.temperature / 128 + 25;
  Serial.print("Temperature in C - ");
  Serial.println(temp);
  return temp;
}

float imu42670p::getAccelRatio(uint8_t accel_fsr)
{

  // get accel sensitivity
  return accel_ratio;
}

float imu42670p::getGyroRatio(uint8_t gyro_fsr)
{
  // get gyro sensitivity
  return gyro_ratio;
}

void imu42670p::transformIMUData()
{
  ax_real = imuData.accel[0] / accel_ratio;
  ay_real = imuData.accel[1] / accel_ratio;
  az_real = imuData.accel[2] / accel_ratio;
  gx_real = imuData.gyro[0] / gyro_ratio;
  gy_real = imuData.gyro[1] / gyro_ratio;
  gz_real = imuData.gyro[2] / gyro_ratio;
}

void imu42670p::transformIMUDataWithOffset()
{
  a_real[0] = ax_real = (imuData.accel[0] - offset_accel[0]) / accel_ratio;
  a_real[1] = ay_real = (imuData.accel[1] - offset_accel[1]) / accel_ratio;
  a_real[2] = az_real = (imuData.accel[2] - offset_accel[2]) / accel_ratio;
  gx_real = (imuData.gyro[0] - offset_gyro[0]) / gyro_ratio;
  gy_real = (imuData.gyro[1] - offset_gyro[1]) / gyro_ratio;
  gz_real = (imuData.gyro[2] - offset_gyro[2]) / gyro_ratio;
}

void imu42670p::printRealworldData()
{
  // Format data for Serial Plotter
  Serial.print("Acc:");
  Serial.print(ax_real);
  Serial.print("\t");
  Serial.print(ay_real);
  Serial.print("\t");
  Serial.print(az_real);
  Serial.print(" |\t");
  // Serial.print("\tGyroX:");
  // Serial.print(gx_real);
  // Serial.print("\tGyroY:");
  // Serial.print(gy_real);
  // Serial.print("\tGyroZ:");
  // Serial.println(gz_real);
}

// Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
// (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
// which fuses acceleration and rotation rate to produce a quaternion-based estimate of relative
// device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
// The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
// but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
void imu42670p::MadgwickQuaternionUpdate(float ax, float ay, float az, float gyrox, float gyroy, float gyroz, float deltaT)
{
  float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];         // short name local variable for readability
  float norm;                                               // vector norm
  float f1, f2, f3;                                         // objetive funcyion elements
  float J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33; // objective function Jacobian elements
  float qDot1, qDot2, qDot3, qDot4;
  float hatDot1, hatDot2, hatDot3, hatDot4;
  float gerrx, gerry, gerrz;                                // gyro bias error
  static float gbiasx = 0.0f, gbiasy = 0.0f, gbiasz = 0.0f; // gyro bias (static to maintain state)

  float GyroMeasError = PI * (40.0f / 180.0f);    // gyroscope measurement error in rads/s (start at 60 deg/s), then reduce after ~10 s to 3
  float beta = sqrt(3.0f / 4.0f) * GyroMeasError; // compute beta
  float GyroMeasDrift = PI * (2.0f / 180.0f);     // gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
  float zeta = sqrt(3.0f / 4.0f) * GyroMeasDrift;

  // Auxiliary variables to avoid repeated arithmetic
  float _halfq1 = 0.5f * q1;
  float _halfq2 = 0.5f * q2;
  float _halfq3 = 0.5f * q3;
  float _halfq4 = 0.5f * q4;
  float _2q1 = 2.0f * q1;
  float _2q2 = 2.0f * q2;
  float _2q3 = 2.0f * q3;
  float _2q4 = 2.0f * q4;

  // Normalise accelerometer measurement
  norm = sqrt(ax * ax + ay * ay + az * az);
  if (norm == 0.0f)
    return; // handle NaN
  norm = 1.0f / norm;
  ax *= norm;
  ay *= norm;
  az *= norm;

  // Compute the objective function and Jacobian
  f1 = _2q2 * q4 - _2q1 * q3 - ax;
  f2 = _2q1 * q2 + _2q3 * q4 - ay;
  f3 = 1.0f - _2q2 * q2 - _2q3 * q3 - az;
  J_11or24 = _2q3;
  J_12or23 = _2q4;
  J_13or22 = _2q1;
  J_14or21 = _2q2;
  J_32 = 2.0f * J_14or21;
  J_33 = 2.0f * J_11or24;

  // Compute the gradient (matrix multiplication)
  hatDot1 = J_14or21 * f2 - J_11or24 * f1;
  hatDot2 = J_12or23 * f1 + J_13or22 * f2 - J_32 * f3;
  hatDot3 = J_12or23 * f2 - J_33 * f3 - J_13or22 * f1;
  hatDot4 = J_14or21 * f1 + J_11or24 * f2;

  // Normalize the gradient
  norm = sqrt(hatDot1 * hatDot1 + hatDot2 * hatDot2 + hatDot3 * hatDot3 + hatDot4 * hatDot4);
  hatDot1 /= norm;
  hatDot2 /= norm;
  hatDot3 /= norm;
  hatDot4 /= norm;

  // Compute estimated gyroscope biases
  gerrx = _2q1 * hatDot2 - _2q2 * hatDot1 - _2q3 * hatDot4 + _2q4 * hatDot3;
  gerry = _2q1 * hatDot3 + _2q2 * hatDot4 - _2q3 * hatDot1 - _2q4 * hatDot2;
  gerrz = _2q1 * hatDot4 - _2q2 * hatDot3 + _2q3 * hatDot2 - _2q4 * hatDot1;

  // Compute and remove gyroscope biases
  gbiasx += gerrx * deltaT * zeta;
  gbiasy += gerry * deltaT * zeta;
  gbiasz += gerrz * deltaT * zeta;
  gyrox -= gbiasx;
  gyroy -= gbiasy;
  gyroz -= gbiasz;

  // Compute the quaternion derivative
  qDot1 = -_halfq2 * gyrox - _halfq3 * gyroy - _halfq4 * gyroz;
  qDot2 = _halfq1 * gyrox + _halfq3 * gyroz - _halfq4 * gyroy;
  qDot3 = _halfq1 * gyroy - _halfq2 * gyroz + _halfq4 * gyrox;
  qDot4 = _halfq1 * gyroz + _halfq2 * gyroy - _halfq3 * gyrox;

  // Compute then integrate estimated quaternion derivative
  q1 += (qDot1 - (beta * hatDot1)) * deltaT;
  q2 += (qDot2 - (beta * hatDot2)) * deltaT;
  q3 += (qDot3 - (beta * hatDot3)) * deltaT;
  q4 += (qDot4 - (beta * hatDot4)) * deltaT;

  // Normalize the quaternion
  norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4); // normalise quaternion
  norm = 1.0f / norm;
  q[0] = q1 * norm;
  q[1] = q2 * norm;
  q[2] = q3 * norm;
  q[3] = q4 * norm;

  // transform quaternion to euler
  yprHistory[index][0] = -(atan2(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3])) * 180.0f / PI;
  float diff = yprHistory[index][0] - yprHistory[(index + MEAN_FILTER_SIZE - 1) % MEAN_FILTER_SIZE][0]; // index + MEAN_FILTER_SIZE - 1 to avoid mod negative number
  if (abs(diff) < 0.1)
    yawDrift += diff;
  ypr[0] = yaw = yprHistory[index][0] - yawDrift;
  int8_t prevCount = firstRound ? index : MEAN_FILTER_SIZE;
  int8_t newCount = firstRound ? index + 1 : MEAN_FILTER_SIZE;
  ypr[1] = ypr[1] * prevCount - yprHistory[index][1];
  ypr[2] = ypr[2] * prevCount - yprHistory[index][2];
  yprHistory[index][1] = pitch = (asin(2.0f * (q[1] * q[3] - q[0] * q[2]))) * 180.0f / PI;
  if (az < 0) // the raw pitch won't exceed 90 degrees
    yprHistory[index][1] = (pitch < 0 ? -1 : 1) * 180 - pitch;
  yprHistory[index][2] = roll = (atan2(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3])) * 180.0f / PI;
  ypr[1] = (ypr[1] + yprHistory[index][1]) / newCount;
  ypr[2] = (ypr[2] + yprHistory[index][2]) / newCount;
  if (firstRound && newCount >= MEAN_FILTER_SIZE)
    firstRound = false;
  index = (index + 1) % MEAN_FILTER_SIZE;
}
void imu42670p::getImuGyro()
{
  getDataFromRegisters(imuData);
  if (imuData.accel[0] != prevData.accel[0] || imuData.accel[1] != prevData.accel[1] || imuData.accel[2] != prevData.accel[2])
  { // only calculate if the gyro data is updated
    for (byte i = 0; i < 3; i++)
      prevData.accel[i] = imuData.accel[i];
    // print realWorld data
    // transformIMUData();
    transformIMUDataWithOffset();
    now = micros();
    deltaT = ((now - lastUpdate) / 1000000.0f); // set integration time by time elapsed since last filter update
    lastUpdate = now;
    // if (lastUpdate - firstUpdate > 10000000uL) {
    //   beta = 0.041;  // decrease filter gain after stabilized
    //   zeta = 0.015;  // increase gyro bias drift gain after stabilized
    // }
    // Pass gyro rate as rad/s
    MadgwickQuaternionUpdate(ax_real, ay_real, az_real, gx_real * PI / 180.0f, gy_real * PI / 180.0f, gz_real * PI / 180.0f, deltaT);

    // Serial.print(yawLag);
    // Serial.print(" ");
    // Serial.print(yawDrift);
    // Serial.print(" ");
    //    char temp[10];
    //    for (byte i = 0; i < 3; i++) {
    //      sprintf(temp, "%8.2f", ypr[i]*(i==1?-1:1)+10);
    //      Serial.print(temp);
    //      Serial.print(" ");
    //    }
    //    printRealworldData();
    // Serial.print("\tdeltaT:");
    // Serial.print("\t");
    // Serial.print(deltaT);
    // Serial.print("\trt:");
    // Serial.print("\t");
    // Serial.print(1.0f / deltaT, 2);
    // Serial.print(" Hz");
  }
}
