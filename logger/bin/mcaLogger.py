#! /usr/bin/python3

import time
import logging
import json
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler
import datetime

from influxdb import InfluxDBClient

with open('config.json', 'r') as f:
    config = json.load(f)

class myLoggingEventHandler(LoggingEventHandler):
    def on_modified(self, event):

        if event.is_directory == False:
            target_filepath = event.src_path
            if 'temp.tmp' not in target_filepath and 'RepeatSpectrumTest' not in target_filepath:
                with open(target_filepath, "r") as f:
                    lines = f.readlines()
                    livetime = float(lines[7].split()[2].replace('\n', ''))
                    realtime = float(lines[8].split()[2].replace('\n', ''))
                    events = sum(int(i.replace('\n', '')) for i in lines[12:8203]) # for 8192 channels

                event_rate_real = events / realtime
                event_rate_live = events / livetime
                utc = datetime.datetime.utcnow()
                print("real rate: " + str(event_rate_real))
                print("live rate: " + str(event_rate_live))
                print("utc      : " + str(utc))
                print("--------------------------------------------------")
                json_data = [
                    {
                        'measurement' : config['influxdb']['measurement'],
                        'fields' : {
                            'event_rate_live' : event_rate_live,
                            'event_rate_real' : event_rate_real,
                        },
                        'time' : utc,
                        'tags' : {
                            'host'   : config['influxdb']['tag']['host'],
                            'device' : config['influxdb']['tag']['device'],
                        }
                    }
                ]
                result = client.write_points(json_data)


if __name__ == "__main__":
    client = InfluxDBClient( 
        host = config['influxdb']['host'],
        port = config['influxdb']['port'],
        username = config['influxdb']['username'],
        password = config['influxdb']['password'],
        database = config['influxdb']['database'],
    )

    logging.basicConfig( 
        level = logging.INFO,
        format = '%(asctime)s - %(message)s',
        datefmt = '%Y-%m-%d %H:%M:%S' 
    )
    path = config['watchdog']['storage']
    event_handler = myLoggingEventHandler()
    observer = Observer()
    observer.schedule(event_handler, path, recursive = True)
    observer.start()
    try:
        while True:
            time.sleep(config['watchdog']['interval'])
    except KeyboardInterrupt:
        observer.stop()
    observer.join()