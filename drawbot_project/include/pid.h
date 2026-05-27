#pragma once

class PIDController {
 public:
  PIDController(float kp = 0.0f, float ki = 0.0f, float kd = 0.0f);

  void setTunings(float kp, float ki, float kd);
  void setOutputLimits(float minOutput, float maxOutput);
  void reset();
  float compute(float setpoint, float measurement, float dtSeconds);

 private:
  float kp_;
  float ki_;
  float kd_;
  float minOutput_;
  float maxOutput_;
  float integral_;
  float previousError_;
  bool hasPrevious_;
};
