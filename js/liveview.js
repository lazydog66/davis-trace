
// This class implements an osilloscope view.
class TraceView {
  constructor(el) {
    // The root element for the view.
    // This should be the canvas element forthe view.
    this.canvas = document.querySelector(el);
    this.context = this.canvas.getContext('2d');

    // The sample set is cached.
    // A sample set contains the actual samples plus other information, eg the
    // sample rate.
    this.sampleSet = false;

    // The default timebase is 100ms per major division.
    this.setTimebase(100);

    this.majorDivisionsHrz = 15;
    this.majorDivisionsVrt = 5;
    this.minorDivisionsPerMajor = 5;

    //  Define the inset value in pixels of the actual display area.
    this.viewMargins = 40;

    // Define the dimensions of the display area.
    this.canvasHeight = this.canvas.height;
    this.canvasWidth = this.canvas.width;
    this.viewWidth = this.canvasWidth - this.viewMargins * 2;
    this.viewHeight = this.canvasHeight - this.viewMargins * 2;
  }


  save() {
    const json = JSON.stringify(this.sampleSet);
    let blob = new Blob([json], { type: 'text/json;charset=utf-8' });

    const filename = 'samples-' + this.sampleSet.timestampStr() + '.json';
    saveAs(blob, filename);
  }

  load(fileList) {

    console.log('load file');
    console.log(fileList);
    console.log('reading ' + fileList.item(0));

    // Check if the file is an image.
    // if (file.type && !file.type.startsWith('image/')) {
    //   console.log('File is not an image.', file.type, file);
    //   return;
    // }

    const reader = new FileReader();

    reader.onloadend = (event) => {
      console.log('samples loaded');
      const samples = JSON.parse(event.target.result);
      const sampleSet = new SampleSet(samples);
      this.setSamples(sampleSet);
      this.redraw();
    };

    reader.onerror = (ev) => console.log(ev);

    reader.readAsText(fileList.item(0));
  }


  // Set the horizontal timebase in milliseconds per major division for the
  // trace.
  setTimebase(ms, offset = 0) {
    this.timebase = ms;
    this.offset = offset ? offset : 0;

    // Shift the drawing origin in by the margins, and use a normal y-axes
    // direction.
    const context = this.canvas.getContext('2d');
    context.translate(this.viewMargins, this.viewHeight);
    context.scale(1, -1);
  }


  // Add a delta to the horizontal offset and redraw the trace.
  scroll(offset) {
    this.offset += offset;
    this.redraw();
  }


  // Set the horizontal offset and redraw the trace.
  setScroll(offset) {
    this.offset = offset;
    this.redraw();
  }


  // Redraw the current sample set.
  // OPtionally, set a new time base for the horizontal scale.
  redraw(options = {}) {
    if (options.timebase)
      this.setTimebase(options.timebase, options.offset);

    this.setDrawingScales();
    this.drawFrame();
    this.drawSamples();
  }


  setDrawingScales() {
    // Shift the drawing origin i by the margins, and use a normal y-axes
    // direction.
    this.context.resetTransform();
    this.context.translate(
      this.viewMargins, this.canvasHeight - this.viewMargins);
    this.context.scale(1, -1);

    // The horizontal scale is the number of pixels per millisecond.
    this.viewHrzScale =
      this.viewWidth / (this.majorDivisionsHrz * this.timebase);

    // The vertical scale is the number of pixels per unit value.
    this.viewVrtScale = this.viewHeight / 255;
  }


  // Draw the backgroudn view/
  drawFrame() {
    this.context.resetTransform();

    this.context.fillStyle = 'rgb(204, 153, 255)';
    this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);

    // Inset the view grid by the margins.
    this.context.translate(this.viewMargins, this.viewMargins);

    // Draw the minor scale.
    this.context.strokeStyle = 'white';
    this.context.lineWidth = 0.25;

    this.context.beginPath();

    let minorDivisions = this.majorDivisionsHrz * this.minorDivisionsPerMajor;
    for (let i = 0; i < minorDivisions; ++i) {
      this.context.moveTo(this.viewWidth * i / minorDivisions, 0);
      this.context.lineTo(this.viewWidth * i / minorDivisions, this.viewHeight);
    }

    minorDivisions = this.majorDivisionsVrt * this.minorDivisionsPerMajor;
    for (let i = 0; i < minorDivisions; ++i) {
      this.context.moveTo(0, this.viewHeight * i / minorDivisions);
      this.context.lineTo(this.viewWidth, this.viewHeight * i / minorDivisions);
    }

    this.context.stroke();

    // Draw the major scale.
    this.context.strokeStyle = 'white';
    this.context.lineWidth = 0.5;

    this.context.beginPath();

    for (let i = 0; i < this.majorDivisionsHrz; ++i) {
      this.context.moveTo(this.viewWidth * i / this.majorDivisionsHrz, 0);
      this.context.lineTo(
        this.viewWidth * i / this.majorDivisionsHrz, this.viewHeight);
    }

    for (let i = 0; i < this.majorDivisionsVrt; ++i) {
      this.context.moveTo(0, this.viewHeight * i / this.majorDivisionsVrt);
      this.context.lineTo(
        this.viewWidth, this.viewHeight * i / this.majorDivisionsVrt);
    }

    this.context.stroke();

    // Draw the bounding box.
    this.context.lineWidth = 1.0;
    this.context.beginPath();
    this.context.moveTo(this.viewWidth, this.viewHeight);
    this.context.lineTo(0, this.viewHeight);
    this.context.lineTo(0, 0);
    this.context.lineTo(this.viewWidth, 0);
    this.context.lineTo(this.viewWidth, this.viewHeight);
    this.context.stroke();

    // Draaw the horizontal scale.
    this.context.font = '12px sans-serif';
    this.context.textAlign = 'center';

    for (let i = 0; i <= this.majorDivisionsHrz; ++i) {
      const x = i * this.viewWidth / this.majorDivisionsHrz;
      const y = -8;

      let label;

      if (this.timebase < 200)
        label = this.offset + i * this.timebase + ' ms';
      else
        label = (this.offset + i * this.timebase) / 1000 + ' s';

      this.context.strokeText(label, x, y);
      this.context.strokeText(label, x, y + this.viewHeight + 28);
    }
  }


  // Draw the contents of thecurrent sample buffer.
  drawSamples() {
    if (!this.sampleSet) return;

    this.setDrawingScales();

    this.context.beginPath();

    this.context.strokeStyle = 'yellow';
    this.context.lineWidth = 3;

    const samples = this.sampleSet.getSamples();
    const sampleRate = this.sampleSet.getSampleRate();

    let first = true;
    samples.forEach((sample, index) => {
      let x = (index * 1000 / sampleRate  - this.offset) * this.viewHrzScale;
      let y = sample * this.viewVrtScale;

      if (first) {
        this.context.moveTo(x, y);
        first = false;
      } else
        this.context.lineTo(x, y);
      // this.context.fillRect(x, y, 3, 3);
    });

    this.context.stroke();
  }


  // Draw a sample set.
  // Along with the samples, a timebase is supplied. This is the sample rate
  // that the samples were taken at.
  setSamples(sampleSet) {
    this.sampleSet = sampleSet;
  }
}
