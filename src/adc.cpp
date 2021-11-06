#include <Arduino.h>

#include "adc.h"

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// Setting a value of 1 here gives a timer 1 frequency of 31.25 KHz.
constexpr int k_timer_1_comapre_to = 1;

// This is the frequency that timer 1 is set to run at.
constexpr float k_timer_1_frequency = 31250.f;

// This value gdrives the adc rate at ~ 38.5 KHz, which is faster than timer1.
constexpr uint8_t k_adc_prescaler = 32;

// The currently active adc object.
static adc* current_adc = nullptr;


// Utility function to start a new adc conversion.
static void analog_trigger(uint8_t pin)
{
  // Setting bit ADSC starts the conversion.
  sbi(ADCSRA, ADSC);
}


// Utility function to return whether the adc value is ready to be read.
static bool analog_ready() { return !bit_is_set(ADCSRA, ADSC); }


// Utility function to read the current adc value.
// The adc is set up to sample at 8 bit resolution and left align the result, ie ADCH contains the result.
static uint8_t analog_read() { return ADCH; }


// Initialise the adc.
// The clock prescaler determines the frequency at which the adc will run at.
// In addition, the adc is set to use 8 bit resolution and left align the result.
// Left laigning te result makes reading the adc value easier.
static void init_adc_clock_prescaler(uint8_t value)
{
  static uint8_t adc_prescale = 0;

  // Nothing to do if using the current prescaler value.
  if (value == adc_prescale) return;

  switch (value) {

    case 2: {
        sbi(ADCSRA, ADPS0);
        cbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS2);
        break;
      }

    case 4: {
        cbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS2);
        break;
      }

    case 8: {
        sbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        cbi(ADCSRA, ADPS2);
        break;
      }

    case 16: {
        cbi(ADCSRA, ADPS0);
        cbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
      }

    case 32: {
        sbi(ADCSRA, ADPS0);
        cbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
      }

    case 64: {
        cbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
      }

    case 128:
    default: {
        sbi(ADCSRA, ADPS0);
        sbi(ADCSRA, ADPS1);
        sbi(ADCSRA, ADPS2);
        break;
      }
  }
}

// The background task frequency is 6250 Hz, which also sets the adc rate.
constexpr uint16_t k_task_timer_value = 9;
constexpr float k_adc_sampling_freq = 16e6 / (256.f * (k_task_timer_value + 1));

// Initialise the adc task and timer.
// The adc is driven directly by timer 1 interrupts.
static void initialise_adc(int adc_select_pin, int isr_prescaler, int adc_prescaler)
{
  // Set up timer 1 interrupts.
  noInterrupts();

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  OCR1A = k_task_timer_value;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);

  // Make sure digital inputs are disabled on he adc channels.
  sbi(DIDR0, ADC5D);
  sbi(DIDR0, ADC6D);
  sbi(DIDR0, ADC7D);

  // Speed up the adc clock divider.
  // The adc should be driven at a rate faster than the timer 1 interrupts.
  init_adc_clock_prescaler(k_adc_prescaler);

  // Initialise the adc by performing a conversion.
  // According to the spec sheet, the first adc conversion takes 25 cycles as
  // opposed to 13 for subsequent conversions. This is a bit of a problem because
  // the adc task runs at 6250 Hz which is too fast for 25 cycles. By performing a conversion
  // now we can initalise the adc and get the 25 cycle start up over with.
  analog_trigger(adc_select_pin);
  delay(1);

  interrupts();
}



adc::adc()
{
  initialise_adc(A7, k_timer_1_comapre_to, k_adc_prescaler);

  current_adc = this;
}


// Start a new sample set.
// Returns true if started, otherwise false if the current set hasn't finished yet.
bool adc::start(uint16_t size, uint8_t sub_samples_size, uint8_t pin)
{
  // Wait for the current set to finish.
  if (sampling_) return false;

  // Set up which adc channel we're sampling on.
  if (pin != adc_pin_) {
    // Set up the adc for the selected channel and left align the result.
    // Left aligning allows the 8 bit value to be read straight from ADCH.
    // Setting bit 6 selects AVCC as the voltage reference.
    // See https://www.arnabkumardas.com/arduino-tutorial/adc-register-description/.
    // pin = analogPinToChannel(pin);
    ADMUX = (1 << 6) | (1 << 5) | (pin & 0x07);

    adc_pin_ = pin;
  }

  sample_index_ = 0;
  sample_set_size_ = size;
  samples_remaining_ = size * sub_samples_size;

  sub_samples_ = sub_samples_size;
  sub_samples_index_ = sub_samples_size;

  sample_set_ready_ = false;

  analog_trigger(adc_pin_);

  // The isr routine will start sampling soon after this is set.
  sampling_ = true;

  return true;
}


// This is the adc service routine.
// All it does is service the sampler adc.
void adc::service()
{
  // If not in a sample set the nothing to do.
  if (!sampling_) return;

  // We're expecting a sample, so return if it's not ready yet.
  if (!analog_ready()) return;

  // If this isn't the last sample in the set then trigger another.
  if (--samples_remaining_ != 0) analog_trigger(adc_pin_);

  // Read the adc value and update the sub sample value.
  sub_sample_accumulator_ += analog_read();

  if (--sub_samples_index_ == 0) {
    uint16_t value = sub_sample_accumulator_ / sub_samples_;
    samples_[sample_index_++] = value;
    sub_samples_index_ = sub_samples_;
  }

  if (samples_remaining_ == 0)
  {
    sampling_ = false;
    sample_set_ready_ = true;
  }
}


// This is timer 1's service routine.
// All we do in it is service the sampler.
ISR(TIMER1_COMPA_vect)
{
  if (current_adc) current_adc->service();
}

