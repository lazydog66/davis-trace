#include <Arduino.h>

constexpr uint16_t k_frame_sample_points = 768;
const float k_frame_wave_length = k_frame_sample_points / 2;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void send_data_frame(uint8_t* data, unsigned int length) {
  static uint8_t header[] = {2, 7, 2, 8, 2, 4, 2, 8, 2, 4, 4, 5, 9, 0, 4, 5};

  // Ensure the data frame is preceeded by a short period of inactivity on the
  // serial interface.
  delay(10);

  // Send the header first.
  Serial.write(header, sizeof(header));
  Serial.write((unsigned int)length);

  // Now send the actual data.
  Serial.write(data, length);
}

void loop() {
  // Create the waveform.
  uint8_t wave[k_frame_sample_points];

  for (uint16_t i = 0; i < k_frame_sample_points; ++i) {
    float amplitude = (1.0 + sin(2.f * PI * i / k_frame_wave_length)) * 0.5f * 255.f;
    wave[i] = round(amplitude);
  }

  // Send the waveform to the client.
  send_data_frame(wave, k_frame_sample_points);

  // Delay before the next frame is sent.
  delay(500);
}