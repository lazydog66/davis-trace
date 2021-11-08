#include <Arduino.h>

#include "adc.h"

constexpr uint16_t k_frame_sample_points = 768 * 1;
constexpr float k_frame_wave_length = k_frame_sample_points / 2;

constexpr uint8_t k_board_led_pin = 0;

void led_on() { digitalWrite(k_board_led_pin, true); }

void led_off() { digitalWrite(k_board_led_pin, false); }

void led_flash(int n) {
  pinMode(k_board_led_pin, OUTPUT);

  for (auto i = 0; i < n; ++i) {
    digitalWrite(k_board_led_pin, true);
    led_on();
    delay(50);
    digitalWrite(k_board_led_pin, false);
    led_off();
    delay(50);
  }
}

void send_data_frame(const uint8_t* data, unsigned int length) {
  // Each data frame consists of an
  //    8 byte header
  //    2 byte data length
  //    n data bytes
  //    2 byte data length
  static uint8_t header[] = {39, 39, 39, 36, 36, 36, 0, 255};

  // Send the header first.
  uint16_t data_length = length;
  Serial.write(header, sizeof(header));
  Serial.write((uint8_t*)&data_length, 8);

  // Now send the actual data.
  Serial.write(data, data_length);
}

void sample(int pin) {

  // Start a new sample set.
  adc& sampler = adc::get();

  if (sampler.start(k_frame_sample_points, 1, pin))
  {  // Wait for it to finish.
    while (!sampler.finished())
      ;
    led_flash(pin + 1);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  analogReference(EXTERNAL);

  analogRead(A0);
  analogRead(A1);
  analogRead(A2);
  analogRead(A3);
  analogRead(A4);
  analogRead(A5);
  analogRead(A6);
  analogRead(A7);

  // Test signal on pins 6 & 7 (timer 3).
  analogWrite(6, 128);
  analogWrite(7, 64);
}

void loop() {
  while (true) {
    String command;

    while (true) {
      int c = Serial.read();

      if (c != -1 && c != '\r') {
        if (c == '\n') break;
        command += String((char)c);
      }
    }

    if (command == String("sample-channel-0"))
      sample(0);
    else if (command == String("sample-channel-1"))
      sample(1);
    else if (command == String("sample-channel-2"))
      sample(2);
    else if (command == String("sample-channel-3"))
      sample(3);
    else if (command == String("sample-channel-4"))
      sample(4);
    else if (command == String("sample-channel-5"))
      sample(5);
    else if (command == String("sample-channel-6"))
      sample(6);
    else if (command == String("sample-channel-7"))
      sample(7);

    send_data_frame(adc::get().samples(), adc::get().sample_set_size());
  }
}
