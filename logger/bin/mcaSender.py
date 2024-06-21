#! /usr/bin/env python3#! /usr/bin/python3

import time
import logging
import json
import subprocess
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler

from influxdb import InfluxDBClient

with open('config.json', 'r') as f:
    config = json.load(f)

class myLoggingEventHandler(LoggingEventHandler):
    def on_modified(self, event):
        if event.is_directory == False:
            target_filepath = event.src_path
            if 'temp.tmp' not in target_filepath and 'RepeatSpectrumTest' not in target_filepath:
                cmd = ['scp', target_filepath, config['scp']['host'] + ':' + config['scp']['path']]
                subprocess.run(cmd)
                print("scp " + target_filepath + " to " + config['scp']['host'] + ':' + config['scp']['path'])


if __name__ == "__main__":
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