import express from 'express';
import fs from 'fs';
import path from 'path';
import { createObjectCsvWriter } from 'csv-writer';

const app = express();
const PORT = 3000;

const writers = new Map();

const getCsvWriter = (source) => {
    const filename = `results_${source}.csv`;
    if (!writers.has(source)) {
        const writer = createObjectCsvWriter({
            path: filename,
            header: [
                { id: 'timestamp', title: 'Timestamp' },
                { id: 'protocol', title: 'Protocol' },
                { id: 'size', title: 'Payload Size (bytes)' },
                { id: 'device_time', title: 'Device Active Time (ms)' },
                { id: 'client_ip', title: 'Client IP' },
                { id: 'client_port', title: 'Client Port' },
                { id: 'header_size', title: 'Header Size (bytes)' },
                { id: 'qos_or_method', title: 'QoS / Method' }
            ],
            append: fs.existsSync(filename)
        });
        writers.set(source, writer);
    }
    return writers.get(source);
};

app.use(express.json()); // Support JSON parsing for active time extraction

app.post('/api/data/:source', async (req, res) => {
    const { source } = req.params;
    const timestamp = process.hrtime.bigint().toString();

    res.status(200).send('OK');

    try {
        const payloadSize = JSON.stringify(req.body).length;
        const headerSize = JSON.stringify(req.headers).length;
        const totalSize = payloadSize + headerSize;
        
        // Extract performance metrics from device payload
        const deviceTime = req.body.device_active_ms || 0;

        const csvWriter = getCsvWriter(source);
        await csvWriter.writeRecords([{
            timestamp: timestamp,
            protocol: 'HTTP',
            size: totalSize,
            device_time: deviceTime,
            client_ip: req.socket.remoteAddress || '',
            client_port: req.socket.remotePort || '',
            header_size: headerSize,
            qos_or_method: req.method
        }]);

        const logTime = new Date().toISOString();
        console.log(`\x1b[32m[${logTime}]\x1b[0m [HTTP][${source}] Recv: ${totalSize}B | Device Time: ${deviceTime}ms`);
    } catch (err) {
        console.error(`Error logging HTTP data for ${source}:`, err);
    }
});

app.listen(PORT, () => {
    console.log(`HTTP Server running on http://localhost:${PORT}`);
});
