import pymysql
import json

def update_table(config: dict):
    SOURCE_DATABASE = config["project"]
    """源数据库名称"""

    TEMPORARY_DATABASE = SOURCE_DATABASE + "_temp"
    """临时数据库名称"""

    LASTEST_DATABASE_JSON = config["sql"]["desc"]
    """新数据库描述文件"""

    db_config = config["database"]
    """数据库连接信息"""

    origin_table_list = []
    """当前数据库表信息"""

    sql_list = []
    """要执行的SQL语句列表"""

    lastest_table_list = {}
    """最新表结构信息"""

    lastest_table_name = []
    """所有最新的表名"""

    sql2gen = {
        'BIGINT': "int64",
        'bigint': "int64",

        'BIGINT UNSIGNED': "uint64",
        'bigint unsigned': "uint64",

        'INT': "int32",
        'int': "int32",

        'INT UNSIGNED': "uint32",
        'int unsigned': "uint32",

        'SMALLINT': "int16",
        'smallint': "int16",

        'SMALLINT UNSIGNED': "uint16",
        'smallint unsigned': "uint16",

        'TINYINT': "int8",
        'tinyint': "int8",

        'TINYINT UNSIGNED': "uint8",
        'tinyint unsigned': "uint8",

        'DOUBLE': "double",
        'double': "double",

        'FLOAT': "float",
        'float': "float",

        'VARCHAR(255)': "string",
        'varchar(255)': "string",

        'TEXT': "text",
        'text': "text",

        'BLOB': "blob",
        'blob': "blob"
    }

    # sql类型
    gen2sql = {
        "int64": "BIGINT",
        "uint64": "BIGINT UNSIGNED",

        "int32": "INT",
        "uint32": "INT UNSIGNED",

        "int16": "SMALLINT",
        "uint16": "SMALLINT UNSIGNED",

        "int8": "TINYINT",
        "uint8": "TINYINT UNSIGNED",

        "float": "FLOAT",
        "double": "DOUBLE",

        "string": "VARCHAR(255)",
        "text": "TEXT",
        "blob": "BLOB"
    }

    try:
        # 连接到源数据库
        source_conn = pymysql.connect(host=db_config["host"],
            port=db_config["port"],
            user=db_config["user"],
            password=db_config["password"]
        )
        source_cursor = source_conn.cursor()

        source_cursor.execute(f"CREATE DATABASE IF NOT EXISTS {SOURCE_DATABASE}")
        source_cursor.execute(f"USE {SOURCE_DATABASE}")

        # 获取源数据库所有表
        source_cursor.execute("SHOW TABLES")
        source_table_list = source_cursor.fetchall()

        # 连接到目标数据库服务器
        temporary_conn = pymysql.connect(
            host=db_config["host"],
            port=db_config["port"],
            user=db_config["user"],
            password=db_config["password"]
        )
        temporary_cursor = temporary_conn.cursor()

        # 删除原来临时数据库
        temporary_cursor.execute(f"DROP DATABASE IF EXISTS {TEMPORARY_DATABASE}")
        print(f"正在清理临时数据库 `{TEMPORARY_DATABASE}` ...")

        # 创建临时数据库
        temporary_cursor.execute(f"CREATE DATABASE IF NOT EXISTS {TEMPORARY_DATABASE}")
        print(f"正在创建临时数据库 `{TEMPORARY_DATABASE}` ...")

        for (name,) in source_table_list:
            # 获取每个表的创建语句
            source_cursor.execute(f"SHOW CREATE TABLE {name}")
            create_table_sql = source_cursor.fetchone()[1]
            
            # 在临时数据库中创建表
            temporary_cursor.execute(f"USE {TEMPORARY_DATABASE}")
            temporary_cursor.execute(create_table_sql)
            print(f"表 `{name}` 已复制到临时数据库")

            source_cursor.execute(f'DESCRIBE {name}')
            desc = source_cursor.fetchall()

            info = {
                "name": name, 
                "field": {},
                "key": []
            }

            for column in desc:
                field_name = column[0]

                info["field"][field_name] = {
                    "name": column[0],
                    "type": column[1],
                    "null": column[2],
                    "key": column[3],
                    "default": column[4],
                }

                # 修饰一下数据
                if sql2gen[column[1]]:
                    info["field"][field_name]["type"] = sql2gen[column[1]]

                info["field"][field_name]["null"] = column[2] == "YES"

                if column[3] != "":
                    info["key"].append(field_name)

                if not info["field"][field_name]["default"]:
                    info["field"][field_name]["default"] = ""

            origin_table_list.append(info)

        temporary_conn.commit()
        print(f"临时数据库 `{TEMPORARY_DATABASE}` 创建成功")

    
        # 从生成文件导入最新表结构
        with open(LASTEST_DATABASE_JSON, 'r', encoding='utf-8') as file:
            lastest_table_list = json.load(file)
            print("导入最新数据库结构")


        temporary_cursor.execute(f"USE {TEMPORARY_DATABASE}")
        for sql_file, tables in lastest_table_list.items():
            for table in tables:
                # 添加到全部表名
                lastest_table_name.append(table["name"])

                # 查找原来的表定义
                origin_table_info = next((elem for elem in origin_table_list if elem["name"] == table["name"]), None)

                # 如果原来没有这个表就直接执行创表语句
                if origin_table_info == None:
                    print(f"正在创建新表 `{table["name"]} `")
                    temporary_cursor.execute(table["origin"])

                    sql_list.append(table["origin"])
                    continue

                lastest_filed_name = []
                modify = False

                # 比较新结构的每一个字段
                for name, field in table["field"].items():
                    lastest_filed_name.append(name)
                
                    # 添加字段
                    if name not in origin_table_info["field"].keys():
                        sql_str = f"ALTER TABLE {table["name"]} ADD COLUMN {name} {gen2sql[field["type"]]}"

                        if not field["null"]:
                            sql_str += " NOT NULL"

                        if field["default"] != "":
                            sql_str += f" DEFAULT {field["default"]}"

                        if field["comment"] and field["comment"] != "":
                            sql_str += f" COMMENT '{field["comment"]}'"

                        temporary_cursor.execute(sql_str)
                        sql_list.append(sql_str)
                        modify = True
                        continue

                    origin_field = origin_table_info["field"][name]

                    # 类型或非空有变化
                    if origin_field["type"] != field["type"] or origin_field["null"] != field["null"]:
                        sql_str = f"ALTER TABLE {table["name"]} MODIFY COLUMN {name} {gen2sql[field["type"]]}"

                        if field["null"]:
                            sql_str += " NULL"
                        else:
                            sql_str += " NOT NULL"

                        if field["comment"] and field["comment"] != "":
                            sql_str += f" COMMENT '{field["comment"]}'"

                        temporary_cursor.execute(sql_str)
                        sql_list.append(sql_str)
                        modify = True
                    
                    # 默认值有变化
                    if origin_field["default"] != field['default']:
                        sql_str = f"ALTER TABLE {table["name"]} ALTER COLUMN {name}"
                    
                        if field["default"] == "":
                            sql_str += " DROP DEFAULT"
                        else:
                            sql_str += f" SET DEFAULT '{field["default"]}'"

                        temporary_cursor.execute(sql_str)
                        sql_list.append(sql_str)
                        modify = True

                if set(origin_table_info["key"]) != set(table["key"]):
                    # 先删除主键
                    drop_str = f"ALTER TABLE {table["name"]} DROP PRIMARY KEY"
                
                    temporary_cursor.execute(drop_str)
                    sql_list.append(drop_str)

                    add_str = f"ALTER TABLE {table["name"]} ADD PRIMARY KEY ({",".join(table["key"])})"
                    temporary_cursor.execute(add_str)
                    sql_list.append(add_str)

                    modify = True
                
                for name, field in origin_table_info["field"].items():
                    if name not in table["field"].keys():
                        sql_str = f"ALTER TABLE {table["name"]} DROP COLUMN {name}"

                        temporary_cursor.execute(sql_str)
                        sql_list.append(sql_str)
                        modify = True

                if modify:
                    print(f"更新表 `{table["name"]}` ")

        # 删除旧表
        for table in origin_table_list:
            if table["name"] not in lastest_table_name:
                sql_str = f"DROP TABLE {table["name"]}"
                temporary_cursor.execute(sql_str)
                sql_list.append(sql_str)

                print(f"表 `{table["name"]}` 已删除")

        temporary_conn.commit()
        print(f"临时数据库 `{TEMPORARY_DATABASE}` 结构更新成功，正在应用至数据库 `{SOURCE_DATABASE}` ...")

        for str in sql_list:
            print(str)
            source_cursor.execute(str)

        source_conn.commit()
        print(f"数据库 `{SOURCE_DATABASE}` 更新成功")

        temporary_cursor.execute(f'DROP DATABASE {TEMPORARY_DATABASE}')
        temporary_conn.commit()
        print(f"临时数据库 `{TEMPORARY_DATABASE}` 已删除")
    
    except pymysql.MySQLError as e:
        print(f"数据库操作错误：{e}")
    finally:
        # 关闭连接
        if source_conn:
            source_cursor.close()
            source_conn.close()
        if temporary_conn:
            temporary_cursor.close()
            temporary_conn.close()

    # 生成JSON数据文件
    # with open('table_info.json', 'w', encoding='utf-8') as file:
    #     file.write(json.dumps(origin_table_list, indent=4, ensure_ascii=False))