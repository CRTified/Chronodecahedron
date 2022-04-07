from argparse import ArgumentParser
from datetime import datetime
import os
import paho.mqtt.client as mqtt


class Chronodecahedron():
    def __init__(self):
        self.battery_level = None
        self.last_seen = None
        self.side = None

    def __str__(self):
        return "{:2} {:3} {}".format(self.side,
                                     self.battery_level or -1,
                                     self.last_seen or "Unknown")


def log_state(userdata):
    """
    Logs a timestmap and the state of the chronodecahedron to stdout and
    the logfile.
    """
    now = datetime.now().replace(microsecond=0).isoformat()
    print(now, str(userdata[0]))
    if userdata[1] is not None:
        with open(userdata[1], "a") as f:
            print(now, str(userdata[0]), file=f)


def on_message(client, userdata, msg):
    """
    Callback function for new messages.
    Sets the monitoring state of the chronodecahedron to the received message.
    """
    if "chronodecahedron_side" in msg.topic:
        new_side = int(msg.payload.decode())
        if userdata[0].side != new_side:
            userdata[0].side = new_side
            log_state(userdata)
    if "battery_level" in msg.topic:
        userdata[0].battery_level = int(msg.payload.decode())
    if "last_seen" in msg.topic:
        userdata[0].last_seen = msg.payload.decode()


if __name__ == '__main__':
    # Parse arguments
    parser = ArgumentParser(
        description="Watch MQTT topics of a given deive for nonvolatile logs."
    )
    parser.add_argument("-s", "--server",
                        default="localhost",
                        help="MQTT host")
    parser.add_argument("-p", "--port",
                        default=1883,
                        type=int,
                        help="MQTT port")
    parser.add_argument("-u", "--user",
                        default=os.environ.get("CHRONO_MQTT_USER"),
                        help="MQTT user")
    parser.add_argument("-w", "--password",
                        default=os.environ.get("CHRONO_MQTT_PASS"),
                        help="MQTT password")
    parser.add_argument("-d", "--device",
                        default="timecube",
                        help="ESPHome hostname, used for MQTT topic")
    parser.add_argument("-l", "--logfile",
                        help="Path to logfile")

    args = parser.parse_args()

    client = mqtt.Client(userdata=(Chronodecahedron(), args.logfile))
    # Register callback
    client.on_message = on_message
    # Connect with auth
    print("Connecting")
    if args.user is not None:
        client.username_pw_set(args.user, args.password)
    client.connect(args.server, args.port, 60)
    print("Connected to MQTT")

    # Subscribe to all relevant topics
    for sensor in ["chronodecahedron_side", "battery_level", "last_seen"]:
        client.subscribe("{}/sensor/{}/state".format(args.device, sensor))

    print("Completed Subscription")

    # Loop forever
    client.loop_forever()
