const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');

const PORT_STRING = '/dev/tty.SLAB_USBtoUART';
const NUM_LEDS = 50;
const HEADER = Buffer.from([190, 49, 196]);

const port = new SerialPort(PORT_STRING, { 
    baudRate: 9600 
}, err => {
    if (err) {
        console.error('Could not open serial port.', err)
    }
    else {
        console.log('Serial port open.');
    }
});
const parser = port.pipe(new Readline({ delimiter: '\n' }));

let r = 0;
let g = 0;
let b = 0;

function preventOverflow() {
    
    const overflowInternal = num => {
        if (num > 255) {
            return 0;
        }
        else if (num < 0) {
            return 255;
        }
        return num;
    }

    r = overflowInternal(r);
    g = overflowInternal(g);
    b = overflowInternal(b);
}

setInterval(() => {
    let data = [];

    for (let ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++) {
        r += 1;
        g += 1 / 3;
        b += 1 / 2;
        preventOverflow();
        data = [Math.floor(r), Math.floor(g), Math.floor(b), ...data];
    }

    const checksumBuffer = Buffer.allocUnsafe(4);
    const checkSum = data.reduce((a, b) => a + b);
    checksumBuffer.writeUInt32BE(checkSum, 0);

    const dataBuffer = Buffer.concat([HEADER, Buffer.from(data), checksumBuffer]);


    port.write(dataBuffer, (error, bytesWritten) => {
        if (error) {
            console.error('Write error.', error);
        }
        console.log('Frame written.');
        // console.log(JSON.stringify(dataBuffer.toJSON().data));
    });
}, 1000);

parser.on('data', data => {
    console.log(data);
});
