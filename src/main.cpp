#include <Arduino.h>

#include "adc.h"


constexpr uint16_t k_frame_sample_points = 768;
constexpr float k_frame_wave_length = k_frame_sample_points / 2;
constexpr uint16_t k_sample_frame_buffer_size = 25000;

constexpr uint8_t k_board_led_pin = 0;

uint8_t sample_buffer[k_sample_frame_buffer_size];


void led_on() {
  digitalWrite(k_board_led_pin, true);
}

void led_off()
{
  digitalWrite(k_board_led_pin, false);
}

void led_flash(int n)
{

  pinMode(k_board_led_pin, OUTPUT);

  for (auto i = 0; i < n; ++i)
  {
    digitalWrite(k_board_led_pin, true);
    led_on();
    digitalWrite(k_board_led_pin, false);
    led_off();
  }
}

void send_data_frame(const uint8_t* data, unsigned int length) {
  // Each data frame consists of an
  //    8 byte header
  //    2 byte data length
  //    n data bytes
  //    2 byte data length
  static uint8_t header[] = { 39, 39, 39, 36, 36, 36, 0, 255 };

  // Send the header first.
  uint16_t data_length = length;
  Serial.write(header, sizeof(header));
  Serial.write((uint8_t *)&data_length, 8);

  // Now send the actual data.
  Serial.write(data, data_length);
}

void sample()
{
  // Start a new sample set.
  adc& sampler = adc::get();

  if (!sampler.start(k_frame_sample_points, 1, A7))
    led_flash(3);;

  // Wait for it to finish.
  while (!sampler.finished());
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

      if (c != -1 && c != '\r')
      {
        if (c == '\n') break;
        command += String((char)c);
      }
    }

    if (command == String("sample-channel-0"))
    {
      sample();
      send_data_frame(adc::get().samples(), adc::get().sample_set_size());
    }
  }
}
