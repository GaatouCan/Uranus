import sys
import os
import json

from update import update_table

def main(argv):

    CONFIG_JSON = "config.json"

    for arg in argv:
        if arg.startswith('--config='):
            CONFIG_JSON = arg[9:]

    CONFIG_JSON = CONFIG_JSON.strip()

    if os.getcwd().endswith("src"):
        parent_dir = os.path.dirname(os.getcwd())
        os.chdir(parent_dir)

    if os.getcwd().endswith("dist"):
        parent_dir = os.path.dirname(os.getcwd())
        os.chdir(parent_dir)

    cfg = {}
    with open(CONFIG_JSON, 'r') as file:
        cfg = json.load(file)

    update_table(config=cfg)

if __name__ == "__main__":
    main(sys.argv)