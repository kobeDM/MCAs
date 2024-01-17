#! /usr/bin/python3

import sys
import time
import logging
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler
import datetime

from influxdb import InfluxDBClient

class ScalerLoggingEventHandler(LoggingEventHandler):
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
                print("utc      : " + utc )
                print("--------------------------------------------------")
                json_data = [
                    {
                        'measurement' : 'mca',
                        'fields' : {
                            'event_rate_live' : event_rate_live,
                            'event_rate_real' : event_rate_real,
                        },
                        'time' : utc,
                        'tags' : {
                            'host'   : 'BGO',
                            'device' : 'BGO',
                        }
                    }
                ]
                result = client.write_points(json_data)


if __name__ == "__main__":

    client = InfluxDBClient( 
        host = "10.37.0.214",
        port = "8086",
        username = "root",
        password = "root",
        database = "miraclue"
    )

    logging.basicConfig( 
        level = logging.INFO,
        format = '%(asctime)s - %(message)s',
        datefmt = '%Y-%m-%d %H:%M:%S' 
    )
    # path = sys.argv[1] if len(sys.argv) > 1 else '.'
    path = 'C:\\Users\\Ryota Namai\\Desktop\\20240116'
    event_handler = ScalerLoggingEventHandler()
    observer = Observer()
    observer.schedule(event_handler, path, recursive = True)
    observer.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()