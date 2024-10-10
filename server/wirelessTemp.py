import time, struct, sys
from datetime import datetime
from pyrf24 import RF24, RF24_PA_MAX,RF24_PA_LOW, RF24_DRIVER, RF24_250KBPS
from paho.mqtt import client as mqtt_client

CSN_PIN = 0  # aka CE0 on SPI bus 0: /dev/spidev0.0
if RF24_DRIVER == "MRAA":
    CE_PIN = 15  # for GPIO22
elif RF24_DRIVER == "wiringPi":
    CE_PIN = 3  # for GPIO22
else:
    CE_PIN = 22
radio = RF24(CE_PIN, CSN_PIN)

address = [bytearray([0xA0, 0xC1, 0xC2, 0xC3, 0x20]), bytearray([0x20, 0xC3, 0xC2, 0xC1, 0xA0])]


broker = NotImplemented
port = 1883
topic1 = NotImplemented
client_id = NotImplemented
username = NotImplemented
password = NotImplemented

def now() -> str:
    return datetime.now().strftime('%H:%M:%S.%f')

def receive():
    received_packets = []
    counter = 0
    radio.listen = True
    start = time.monotonic()
    got_first_payload = False
    while 1:
        time.sleep(0.002)

        if got_first_payload and time.monotonic() >= (start + 0.1):
            return received_packets[0], len(received_packets)
            
        has_payload, pipe_number = radio.available_pipe()
        if has_payload:
            got_first_payload = True
            counter+=1
            length = radio.payload_size
            received = radio.read(length)
            temp, id, ds18b20_errno, millis = struct.unpack('>hHHI', received[0:10])
            received_packets.append((temp, id, ds18b20_errno, millis))
            print(f"{now()}:Receiv {length}B, {temp}, {id}, {ds18b20_errno}, {millis}") #pipe1
            start = time.monotonic()

    # recommended behavior is to keep in TX mode while idle
    radio.listen = False

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
    # For paho-mqtt 2.0.0, you need to add the properties parameter.
    # def on_connect(client, userdata, flags, rc, properties):
        if rc == 0:
            pass
        else:
            print(f"{now()}:Failed to connect, return code %d\n", rc)
    # Set Connecting Client ID
    #client = mqtt_client.Client(client_id)
    # For paho-mqtt 2.0.0, you need to set callback_api_version.
    client = mqtt_client.Client(client_id=client_id, callback_api_version=mqtt_client.CallbackAPIVersion.VERSION2)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def publish(client, topic, data):
    result = client.publish(topic, data, 1, True)
    status = result[0]
    if status == 0:
        print(f"{now()}:Packet mqtt has been sent")
    else:
        print(f"{now()}:Failed to send message to topic: {topic}")


if __name__ == "__main__":
    print(f"{now()}:wirelessTemp starts")
    if not radio.begin():
        print(f"{now()}:nRF24L01 hardware isn't responding")
    radio.channel = 90
    radio.set_pa_level(RF24_PA_LOW )
    radio.open_tx_pipe(address[1])  
    radio.open_rx_pipe(1, address[1])  
    radio.open_rx_pipe(0, address[0])  
    radio.payload_size = 10
    radio.data_rate = RF24_250KBPS
    

    while(1):
        payload, count = receive()
        try:
            client = connect_mqtt()
            publish(client, topic1, f"{payload[0]/100}")
            client.disconnect()
        except Exception as err:
            print(f'{now()}:MQTT error: ', type(err).__name__, err)


