#include <Arduino.h>

#include "adc.h"


constexpr uint16_t k_frame_sample_points = 768;
constexpr float k_frame_wave_length = k_frame_sample_points / 2;
constexpr uint16_t k_sample_frame_buffer_size = 25000;

uint8_t sample_buffer[k_sample_frame_buffer_size];










void send_data_frame(const uint8_t* data, unsigned int length) {
  static uint8_t header[] = { 2, 7, 2, 8, 2, 4, 2, 8, 2, 4, 4, 5, 9, 0, 4, 5 };

  // Ensure the data frame is preceeded by a short period of inactivity on the
  // serial interface.
  delay(10);

  // Send the header first.
  Serial.write(header, sizeof(header));
  Serial.write((unsigned int)length);

  // Now send the actual data.
  Serial.write(data, length);
}

void sample()
{
  adc& sampler = adc::get();

  sampler.start(k_frame_sample_points, 1, A7);
  while (!sampler.finished());
}

void flash_led(int n)
{
  constexpr uint8_t k_board_led_pin = 0;

  pinMode(k_board_led_pin, OUTPUT);

  for (auto i =0 ; i < n; ++i)
  {
    digitalWrite(k_board_led_pin, true);
    delay(100);
    digitalWrite(k_board_led_pin, false);
    delay(100);
  }

}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {

  while (true)
  {
    String command;

    while (true)
    {
      int c = Serial.read();

      if (c != -1 && c !='\r')
      {
        if (c == '\n') break;
        command += String((char)c);
      }
    }

    if (command == String("sample-channel-0"))
    {
      flash_led(1);

      sample();
      send_data_frame(adc::get().samples(), adc::get().sample_set_size());
    }
  }
}
