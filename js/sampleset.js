
class SampleSet {
  constructor(options = {}) {
    const {samples = [], sampleRate = 1000, time = Date.now()} = options;

    this.samples = samples;
    this.sampleRate = sampleRate;
    this.time = time;
    this.dateTime = new Date(time).toLocaleString();
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


  timestampStr() {
    // Use a timestamp to identify the samples.
    const time = new Date(this.time);

    const y = time.getFullYear();
    const d = String(time.getDate()).padStart(2, '0');
    const mm = String(time.getMonth() + 1).padStart(2, '0');  // January is 0!
    const h = String(time.getHours()).padStart(2, '0');
    const m = String(time.getMinutes()).padStart(2, '0');
    const s = String(time.getSeconds()).padStart(2, '0');

    return y + '-' + mm + '-' + d + '_' + h + '-' + m + '-' + s;
  }
}
