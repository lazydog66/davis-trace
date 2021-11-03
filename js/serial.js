class Serial {

  constructor() {

    this.port = false;
    this.baudrate = 115200;
  }

  async connect(port, baudRate = 115200) {

    this.port = port;
    this.baudRate = 115200;

    await port.open({ baudRate: this.baudRate });


  }


}