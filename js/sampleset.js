
class SampleSet {
  constructor(options = {}) {
    const {samples = [], sampleRate = 1000} = options;

    this.samples = samples;
    this.sampleRate = sampleRate;
  }


  size() {
    return this.samples.length;
  }


  getSamples() {
    return this.samples;
  }


  getSampleRate() {
    return this.sampleRate;
  }
}
