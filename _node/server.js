// Server for monitoring GPS tracker

process.env.TZ = 'Europe/Moscow'; // does not work in windows

const fs = require('fs');
const net = require('net');
const querystring = require('querystring');

function parseHttpHeader(header) {
    const match = /(^[^ ]+) ([^? ]+)(\?([^ ]+))?/m.exec(header);
    return {
        method: match[1],
        url: match[2],
        params: querystring.parse(match[4])
    };
}
const server = net.createServer((c) => {
    server.getConnections((err, count) => console.log("client connected " + count));
    c.on('end', () => console.log('client disconnected'));
    c.on('data', (data) => {
        console.log('data');
        if(('' + data).startsWith('GET ')) {
            const header = parseHttpHeader(data);
            if (header.url === '/status') { // Browser connected via HTTP - send status page
                console.log('HTTP GET /status');
                c.write(
                    'HTTP/1.1 200 OK\r\n' +
                    'Cache-Control: public, max-age=0\r\n' +
                    'Content-type: text/html\r\n' +
                    '\r\n');
                c.end(logger.status());
            } else {
                console.log('HTTP GET ' + header.url);
                c.end();
            }
        } else { // Tracker connected via TCP - decode and save GPS data
            console.log('TCP');
            console.log(data);
            fs.appendFileSync('log.bin', data);

        }
    });
    c.on('error', (err) => {
        if (err.code === 'ECONNRESET')
            console.log('client disconnected (ECONNRESET)');
        else
            console.log(err.stack);
    });
});
server.on('error', (err) => {
    throw err;
});
server.listen(20300, () => {
    console.log('TCP and HTTP Server running on port 20300');
});
