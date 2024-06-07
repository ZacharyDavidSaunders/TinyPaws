import asyncio, requests
from bleak import BleakClient

SERVICE_UUID = "12345678-1234-1234-1234-123456789abc"
CHARACTERISTIC_UUID = "12345678-1234-1234-1234-123456789def"
DEVICE_ADDRESS = "8F:3D:2E:5E:03:84" # Replace with Your Device MAC address
PUSHOVER_TITLE = "TinyPaws"
PUSHOVER_URL = "https://api.pushover.net/1/messages.json"
PUSHOVER_CONFIG = # ["<ZAC_API_KEY>","<ZAC_USER_KEY>","<ZAC_DEVICE>"]# ZAC
#,["<BASSAM_API_KEY>","<BASSAM_USER_KEY>","<BASSAM_DEVICE>"]
#,["<DANIEL_API_KEY>","<DANIEL_USER_KEY>","<DANIEL_DEVICE>"]
#,"<PIN_API_KEY>","<PIN_USER_KEY>","<PIN_DEVICE>"]

bark_count = 0


def SendPushNotification(message):
    
    for list in PUSHOVER_CONFIG:
        request = {
            "token": list[0],
            "user": list[1],
            "device": list[2],
            "title": PUSHOVER_TITLE,
            "message": message
        }
                    
        print(f"Sending push with message: {message}...")
                    
        if requests.post(PUSHOVER_URL, data = request).status_code == 200:
            pass
        else:
            print(f"Error sending push notification")

def notification_handler(sender, data):
    global bark_count
    message = data.decode()
    print(f"Notification from {sender}: {message}")
    if message == "Bark Detected!":
        SendPushNotification("Bark detected!")
        bark_count += 1
        print(f"Bark count: {bark_count}")
        with open("bark_count.txt", "w") as file:
            file.write(f"Bark count: {bark_count}")

async def run():
    async with BleakClient(DEVICE_ADDRESS) as client:
        connected = await client.is_connected()
        print(f"Connected: {connected}")
        if connected:
            await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
            print("Receiving notifications...")
            try:
                await asyncio.Future()
            except asyncio.CancelledError:
                pass
            await client.stop_notify(CHARACTERISTIC_UUID)

if __name__ == "__main__":
    asyncio.run(run())
    
