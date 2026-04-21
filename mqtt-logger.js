import mqtt from 'mqtt';
import fs from 'fs';
import { createObjectCsvWriter } from 'csv-writer';

const MQTT_URL = 'mqtt://localhost:1883';
const TOPIC_PREFIX = 'sensors/';

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

const client = mqtt.connect(MQTT_URL);

client.on('connect', () => {
    console.log(`MQTT Connected to ${MQTT_URL}`);
    client.subscribe(`${TOPIC_PREFIX}+`, (err) => {
        if (!err) {
            console.log(`Subscribed to topic: ${TOPIC_PREFIX}+`);
        }
    });
});

client.on('message', async (topic, message, packet) => {
    const timestamp = process.hrtime.bigint().toString();
    const source = topic.replace(TOPIC_PREFIX, '');

    const payloadSize = message ? message.length : 0;
    const topicLength = topic.length;
    const estimatedmqttOverhead = 2 + topicLength;
    const totalSize = payloadSize + estimatedmqttOverhead;

    let deviceTime = 0;
    try {
        const payload = JSON.parse(message.toString());
        deviceTime = payload.device_active_ms || 0;
    } catch (e) {}

    try {
        const csvWriter = getCsvWriter(source);
        await csvWriter.writeRecords([{
            timestamp: timestamp,
            protocol: 'MQTT',
            size: totalSize,
            device_time: deviceTime,
            client_ip: 'N/A (Broker)', 
            client_port: 'N/A',
            header_size: estimatedmqttOverhead,
            qos_or_method: packet ? packet.qos : 0
        }]);

        const logTime = new Date().toISOString();
        console.log(`\x1b[32m[${logTime}]\x1b[0m [MQTT][${source}] Recv: ${totalSize}B | Device Time: ${deviceTime}ms`);
    } catch (err) {
        console.error(`Error logging MQTT data for ${source}:`, err);
    }
});

client.on('error', (err) => {
    console.error('MQTT Connection error:', err);
});
