class Serial {

  constructor() {
    this.port = false;
    this.baudrate = 115200;
    this.stop = false;
  }

  async connect(baudRate = 115200) {
    // Get all serial ports the user has previously granted the website access
    // to.
    this.port = await navigator.serial.requestPort();

    console.log(this.port);

    await this.port.open({baudRate: 115200});

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

          console.log(value);
          var str = String.fromCharCode.apply(null, value);
          console.log(str);
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

// /    return new TextDecoder("utf-8").decode(bufferValue);

    // const textEncoder = new TextEncoderStream();
    // const writableStreamClosed = textEncoder.readable.pipeTo(this.port.writable);
    // const writer = textEncoder.writable.getWriter();

    // await writer.write(command);

    console.log('>> wrote ' + command);

    // Allow the serial port to be closed later.
    writer.releaseLock();

    // async disconnect() {
    //   if (!this.port) return;

    //   this.stop = true;

    //   console.log('disconnected from serial port');
    // }
  }
}


