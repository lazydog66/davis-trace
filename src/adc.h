#pragma once

#include <Arduino.h>

// This is the maximum size of the sample set.
// A buffer of this size is allocated, so the size should be appropriate for
// the particular Arduino the progra is running on.
constexpr uint16_t k_max_sample_set_size = 10000;

class adc
{

public:

  // Use a singleton for access to the adc.
  static adc& get() {
    static adc the_one_and_only;
    return the_one_and_only;
  }

  // Return the sampling frequency used for the current sample set.
  uint32_t sample_rate() const { return sample_rate_; }

  // Return the number of samples in the current sample set.
  uint16_t sample_set_size() const { return sample_set_size_; }

  // Return the actual sample set.
  const uint8_t* samples() const { return samples_; }

  // Start a new sample set.
  // Returns true if started, otherwise false if the current set hasn't finished yet.
  bool start(uint16_t size, uint8_t sub_samples_size, uint8_t pin);

  // Return whether a sample set is ready.
  bool finished() const {return sample_set_ready_; }

  // Service the adc convertion.
  void service();

private:

  adc();
  ~adc();

  void start_timer();

  void stop_timer();

  // THe sampling frequency used for the current sample set.
  uint32_t sample_rate_ = 1000;

  // This is the adc pin to use for acquiring the samples.
  uint8_t adc_pin_ = 255;

  // The numbe of samples in the sample set.
  uint16_t sample_set_size_ = 0;

  // The sample set buffer.
  uint8_t samples_[k_max_sample_set_size];

  // The adc sampling frequency is set determined by the frequency of
  // of timer 1 and the sub samlping size.
  float sample_frequency_ = 0.f;

  // This will be true if a sample set is currently being acquired.
  volatile bool sampling_ = false;

  // This will be true once  a sample set has been fully acquired.
  volatile bool sample_set_ready_ = false;

  // COutns the number of samples taken.
  volatile uint16_t sample_index_ = 0;

  // This is the number of samples still to be collected durinage an acquisition.
  volatile uint32_t samples_remaining_ = 0;

  // The numebr of sub samples that make up a sample.
  volatile uint8_t sub_samples_ = 0;

  // Counts the number of sub samples being taken during an acquisition.
  volatile uint8_t sub_samples_index_ = 0;

  // The sub samlpe accumulator.
  // Samples are created by averaging the sub samples.
  volatile uint16_t sub_sample_accumulator_ = 0;


};
