#include <Arduino.h>

#include "adc.h"

constexpr uint16_t k_frame_sample_points = 768 * 3;
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

void send_data_frame(const uint8_t* data, uint16_t data_length, uint32_t sample_rate) {
  // Each data frame consists of an
  //    8 byte header
  //    4 sample rate
  //    2 byte data length
  //    n data bytes
  static uint8_t header[] = { 39, 39, 39, 36, 36, 36, 0, 255 };

  // Send the header first.
  Serial.write(header, sizeof(header));
  Serial.write((uint8_t*)&sample_rate, 4);
  Serial.write((uint8_t*)&data_length, 2);

  // Now send the actual data.
  Serial.write(data, data_length);
}

void sample(uint8_t channel, uint16_t sample_rate, uint16_t sample_count) {

  // Bounds check on the desired sample set.
  // Note, the sampling rate used is not guaranteed to be the exact value asked for.
  uint16_t sub_samples = k_max_sample_rate / sample_rate;
  channel = min(max(0, channel), 7);
  sample_count = min(sample_count, k_max_sample_set_size);

  // Start a new sample set.
  adc& sampler = adc::get();

  if (sampler.start(sample_count, sub_samples, channel))
  {  // Wait for it to finish.
    while (!sampler.finished());
    led_flash(1);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200 * 1);
  //  Serial.begin(115200);

  analogReference(EXTERNAL);

  analogRead(A0);
  analogRead(A1);
  analogRead(A2);
  analogRead(A3);
  analogRead(A4);
  analogRead(A5);
  analogRead(A6);
  analogRead(A7);

  // Test 490.196 Hz signal on pins 6 & 7 (timer 3).
  //    pin 6 50% duty cycle
  //    pin 8 25% duty cycle
  analogWrite(6, 128);
  analogWrite(7, 64);

  Serial.println(String("OCR3A=") + String(OCR3A));
  Serial.println(String("OCR3B=") + String(OCR3B));
  Serial.println(String("TCCR3A=") + String(TCCR3A));
  Serial.println(String("TCCR3B=") + String(TCCR3B));
  Serial.println(String("TCCR3C=") + String(TCCR3C));
}


// Parse a sample command of the form,
//    sample-<channel>-<sample-rate>-<sample-count>
// Returns true if the command was recognised as a valid sample command.
bool parse_sample_command(const String& str, uint8_t& channel, uint16_t& sample_rate, uint16_t& sample_count)
{
  if (!str.startsWith(F("sample-"))) return false;

  // Keep the parameters part of the command string.
  String parameters = str.substring(7);

  // Get the channel number.
  channel = parameters[0] - '0';
  if (channel < 0 || channel > 7) return false;

  if (parameters[1] != '-') return false;
  parameters = parameters.substring(2);

  // Get the sample rate.
  sample_rate = parameters.toInt();
  if (sample_rate == 0) return false;

  parameters = parameters.substring(String(sample_rate).length());
  if (parameters[0] != '-') return false;

  parameters = parameters.substring(1);
  sample_count = parameters.toInt();
  if (sample_count == 0) return false;

  return true;
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


    // Look for the sample command, format is,
    //    sample-<channel>-<sample-rate>-<sample-count>
    uint8_t channel;
    uint16_t sample_rate;
    uint16_t sample_count;

    if (parse_sample_command(command, channel, sample_rate, sample_count))
    {
      sample(channel, sample_rate, sample_count);
      send_data_frame(adc::get().samples(), adc::get().sample_set_size(), adc::get().sample_rate());
    }
    else
      led_flash(10);
  }
}
