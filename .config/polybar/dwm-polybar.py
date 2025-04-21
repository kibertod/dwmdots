#!/bin/python3
import os
import json
from sys import stdout

layout = "[]="
tags = {1: 1, 2: 2, 4: 3, 8: 4, 16: 5, 32: 6, 64: 7, 128: 8, 256: 9}
active_tag = 0


def main():
    evstream = os.popen("dwm-msg subscribe layout_change_event tag_change_event")

    if evstream is None:
        exit()

    buffer = ""

    while True:
        global active_tag
        global layout
        if not evstream.readable():
            exit()
        try:
            buffer += evstream.readline()
        except:
            evstream.close()
            exit()
        try:
            event: dict = json.loads(buffer)
            buffer = ""
            if "tag_change_event" in event.keys():
                active_tag = tags[event["tag_change_event"]["new_state"]["selected"]]
            if "layout_change_event" in event.keys():
                layout = event["layout_change_event"]["new_symbol"]
            res = "\n"
            for i in range(1, 10):
                if i == active_tag:  # pyright: ignore unbound
                    res += f"|{i}|"
                else:
                    res += f" {i} "
            res += layout  # pyright: ignore unbound
            print(res)
            stdout.flush()
        except:
            pass


if __name__ == "__main__":
    main()
