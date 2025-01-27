import os
import json

# 新数据库描述文件
LASTEST_DATABASE_JSON = "struct/struct.json"

target_table_list = {}

# 从生成文件导入最新表结构
with open(LASTEST_DATABASE_JSON, 'r', encoding='utf-8') as file:
    target_table_list = json.load(file)
    print("导入最新数据库结构")

for sql_file, tables in target_table_list.items():
    for table in tables:
        print(table["origin"])