#! /usr/bin/env python3

import os
import time
import logging
import numpy as np
from matplotlib import pyplot as plt
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler

CH = 8192

def plotresult(eventfilepath):
    plt.clf()
    plt.cla()
    plt.close()

    offset_live = 7
    offset_real = 8
    offset_count = 12
    with open(eventfilepath, "r") as f:
        lines = f.readlines()
        livetime = float(lines[offset_live].split()[2].replace('\n', ''))
        realtime = float(lines[offset_real].split()[2].replace('\n', ''))
        counts = np.array([int(i.replace('\n', '')) for i in lines[offset_count:CH+offset_count]]) # for 8192 channels

    # initialize logfile
    if not os.path.isfile(logfilepath):
        with open(logfilepath, 'w') as f:
            for i in range(CH+2):
                f.write('0\n')

    with open(logfilepath, 'r') as f:
        all_data = [float(i.replace('\n', '')) for i in f.readlines()]
    livetime_total = livetime + float(all_data[0])
    realtime_total = realtime + float(all_data[1])
    all_counts = counts + all_data[2:CH+2]

    with open('./countsLog.txt', 'w') as f:
        f.write(str(livetime_total)+'\n')
        f.write(str(realtime_total)+'\n')
        for counts in all_counts:
            f.write(str(counts)+'\n')

    x = np.arange(1, CH+1, 1)
    # plt.title('All Events')
    # plt.xlabel('channels')
    # plt.ylabel('events')
    # fig = plt.figure()
    plt.hist(x, weights=all_counts/2, bins=CH, color="blue")
    plt.xlim(0, CH)
    # plt.title('All Events')
    ax = plt.gca()
    ax.set_xlabel('channels')
    ax.set_ylabel('events')
    y_min, y_max = ax.get_ylim()
    plt.text(x=CH*0.5, y=y_max*0.90, s="livetime total : "+str('{:.2f}'.format(livetime_total/2)))
    plt.text(x=CH*0.5, y=y_max*0.85, s="realtime total : "+str('{:.2f}'.format(realtime_total/2)))
    plt.text(x=CH*0.5, y=y_max*0.80, s="total events   : "+str(int(np.sum(all_counts/2))))
    plt.pause(.001)

class myLoggingEventHandler(LoggingEventHandler):
    def on_modified(self, event):
        if event.is_directory == False:
            target_filepath = event.src_path
            if 'temp.tmp' not in target_filepath and 'RepeatSpectrumTest' not in target_filepath:
                plotresult(target_filepath)

if __name__ == "__main__":
    datadirpath = "C:\\Users\\msgc\\miraclue\\bgo\\data\\20240118"
    # path = sys.argv[1] if len(sys.argv) > 1 else '.'
    logfilepath = './countsLog.txt'

    logging.basicConfig( 
        level = logging.INFO,
        format = '%(asctime)s - %(message)s',
        datefmt = '%Y-%m-%d %H:%M:%S' 
    )
    event_handler = myLoggingEventHandler()
    observer = Observer()
    observer.schedule(event_handler, datadirpath, recursive = True)
    observer.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
        os.remove(logfilepath)
    observer.join()