class Serial {

  constructor(readFn) {

    // readFn is a callback which gets the serail data as it is read.
    this.readFn = readFn;

    this.port = false;
    this.baudrate = 500000;
    this.stop = false;

    // This is where the serial data is buffered as it arrives.
    this.frameBuffer = [];
  }

  async connect(baudRate = 115200) {
    // Get all serial ports the user has previously granted the website access
    // to.
    this.port = await navigator.serial.requestPort();

    console.log(this.port);

    await this.port.open({baudRate});

    this.stop = false;

    while (!this.stop && this.port.readable) {
      const reader = this.port.readable.getReader();

      try {
        while (!this.stop) {

          const {value, done} = await reader.read();

          if (done) {
            console.log('*** canceled', error);
            break;
          }

          // Append the data to the current frame buffer.
          // The buffer is then passed on to the read callback. It will process
          // the data buffer and return back a new frame buffer. For example, if the
          // the frame buffer is consumed, the callback should return a new empty buffer.
          const data = Array.from(value);
          this.frameBuffer = this.frameBuffer.concat(data);
          this.frameBuffer = this.readFn(this.frameBuffer);
        }

      } catch (error) {
        console.log('*** error', error);
      } finally {
        console.log('*** finished');
        reader.releaseLock();

        if (this.stop) {
          this.port.close();
        }
      }
    }
  }

  async writeCommand(command)
  {
    if (!this.port) return;

    let data = new TextEncoder("utf-8").encode(command);

    const writer = this.port.writable.getWriter();
    await writer.write(data);

    console.log('>> wrote ' + command);

    // Allow the serial port to be closed later.
    writer.releaseLock();
  }
}


