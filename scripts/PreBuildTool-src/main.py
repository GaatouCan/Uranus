import proto
import sql
import xlsx2json

import sys
import json
import os

def main(argv):
    CONFIG_JSON = "config.json"

    for arg in argv:
        if arg.startswith('--config='):
            CONFIG_JSON = arg[9:]

    if os.getcwd().endswith("src"):
        parent_dir = os.path.dirname(os.getcwd())
        os.chdir(parent_dir)

    if os.getcwd().endswith("dist"):
        parent_dir = os.path.dirname(os.getcwd())
        os.chdir(parent_dir)

    config = {}
    with open(CONFIG_JSON, 'r') as file:
        config = json.load(file)

    if not config["only_xlsx"]:
        proto.generate_protobuf_define(
            config["protobuf"]["dir"],
            config["protobuf"]["gen"],
            config["protobuf"]["def"],
            config["protobuf"]["except"],
            config["protobuf"]["include_dir"]
        )

        sql.generate_orm_clazz(
            config["sql"]["sql"],
            config["sql"]["gen"],
            config["sql"]["desc"]
        )
    else:
        print('-- Only generate configuration files')
    
    # 生成配置
    xlsx2json.generate_config_json(
        config["xlsx"]["xlsx"], 
        config["xlsx"]["json"]
    )

if __name__ == "__main__":
    main(sys.argv)