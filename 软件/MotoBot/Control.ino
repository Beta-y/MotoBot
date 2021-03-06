/*
 * Author: Xuechao Zhang
 * Date: July 24th, 2020
 * Description: 转向、平衡、飞轮、驱动电机控制
 */

void servo_control(){
  // 双舵机测试
  for (int pos = 45; pos <= 135; pos += 30)
  {
    steer_servo.write(pos);
    balance_servo.write(pos);
    Serial.println(pos);
    delay(1000);
  }
  for (int pos = 135; pos >= 45; pos -= 30)
  {
    steer_servo.write(pos);
    balance_servo.write(pos);
    Serial.println(pos);
    delay(1000);
  }
}

void balance_control(){
//  static float last_roll;
  static float roll_sum;


  if (abs(roll - last_roll)>10)
  {
    roll = last_roll; // 去掉异常值
    Serial.print("fuckfuck");
  }

  roll_sum += roll;
  roll_sum = constrain(roll_sum, -1000, 1000); // 限幅
  
  //flywheel_target = bl_kp * (roll - 0) + bl_ki * roll_sum + bl_kd * (roll - last_roll);

  // 串级一下
  flywheel_target = bl_kp * (roll - flywheel_speed * 0.003) + bl_ki * roll_sum + bl_kd * (roll - last_roll);
  
  //flywheel_target = millis() / 10 % 1000 - millis() / 10 % 200; // 速度环调试
  if (digitalRead(BUTTON0)==HIGH) // 速度开环
    pwm_out = flywheel_PID(flywheel_target);
  else
    pwm_out = flywheel_target / 20;
    
  pwm_out = constrain(pwm_out, -200, 200); // 限幅
  if (pwm_out < 5 && pwm_out > -5) pwm_out = 0; //防止烧电机
  if (pwm_out > 0) // 方向控制
  {
    digitalWrite(FLYWHEEL_DIR, LOW);
    analogWrite(FLYWHEEL, 255.0 - pwm_out);
  }
  else
  {
    digitalWrite(FLYWHEEL_DIR, HIGH);
    analogWrite(FLYWHEEL, 255.0 + pwm_out);
  }
  last_roll = roll;
}

int flywheel_PID(int target) // 速度环
{ 
  static int fw_pwm, error, error_last = 0, error_sum;
  error = target - flywheel_speed;
  error_sum += error;
  error_sum = constrain(error_sum, -10, 10); // 限幅
  fw_pwm += fw_kp * error + fw_ki * error_sum + fw_kd * (error - error_last);
  error_last = error;
  return fw_pwm;
}

void flywheel_encoder(){ // 感觉可以减少一半的中断触发 只看下降沿
  // ENA脚下降沿中断触发
  if (digitalRead(ENCODER_A) == LOW)
  {
    // 查询ENB的电平以确认是顺时针还是逆时针旋转
    if (digitalRead(ENCODER_B) == LOW)
      flywheel_position[1]++;
  }
  // ENA脚上升沿中断触发
  else
  {
    // 查询ENB的电平以确认是顺时针还是逆时针旋转
    if (digitalRead(ENCODER_B) == LOW)
      flywheel_position[1]--;
  }
}

void flywheel_readspeed() // 作差计算速度
{
  flywheel_speed = (flywheel_position[1] - flywheel_position[0])/elapsedTime;
  flywheel_position[0] = flywheel_position[1];
}
