
// This class implements an osilloscope view.
class TraceView {
  constructor(el) {
    // The root element for the view.
    // This should be the canvas element forthe view.
    this.canvas = document.querySelector(el);
    this.context = this.canvas.getContext('2d');

    // The sample set is cached.
    this.sampleSet = [];
    this.sampleRate = 10000;

    // The default timebase is 100ms per major division.
    this.setTimebase(100);

    this.majorDivisionsHrz = 10;
    this.majorDivisionsVrt = 6;
    this.minorDivisionsPerMajor = 5;

    //  Define the inset value in pixels of the actual display area.
    this.viewMargins = 20;

    // Define the dimensions of the display area.
    this.canvasHeight = this.canvas.height;
    this.canvasWidth = this.canvas.width;
    this.viewWidth = this.canvasWidth - this.viewMargins * 2;
    this.viewHeight = this.canvasHeight - this.viewMargins * 2;
  }


  save() {
    let blob = new Blob(['hello world!'], {type: 'text/plain;charset=utf-8'});
    saveAs(blob, 'samples.txt');
  }


  // Set the horizontal timebase in milliseconds per major division for the
  // trace.
  setTimebase(ms) {
    this.timebase = ms;

    // Shift the drawing origin in by the margins, and use a normal y-axes
    // direction.
    const context = this.canvas.getContext('2d');
    context.translate(this.viewMargins, this.viewHeight);
    context.scale(1, -1);
  }


  // Redraw the current sample set.
  // OPtionally, set a new time base for the horizontal scale.
  redraw(options = {}) {
    if (options.timebase) this.setTimebase(options.timebase);

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
  }


  // Draw the contents of thecurrent sample buffer.
  drawSamples() {
    this.setDrawingScales();

    this.context.beginPath();


    this.context.strokeStyle = 'yellow';
    this.context.lineWidth = 3;

    let first = true;
    this.samples.forEach((sample, index) => {
      let x = index * 1000 / this.sampleRate * this.viewHrzScale;
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
  setSamples(samples, sampleRate) {
    this.samples = samples.slice();
    this.sampleRate = sampleRate;
  }

}
