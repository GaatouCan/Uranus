import pandas
import os
import json

def generate_config_json(src: str, dist: str):
    """将xlsx配置表转换为json文件"""

    assert src, 'Xlsx input path error'
    assert dist, 'Json output path error'

    print(f'-- Xlsx input path: {os.getcwd()}\\{src}')
    print(f'-- JSON output path: {os.getcwd()}\\{dist}')

    if not os.path.exists(dist):
        os.makedirs(dist)
        print('-- Creating Json output floder')

    count = 0
    for root, dirs, files in os.walk(src):
        for file in files:
            if not file.endswith('.xlsx'):
                continue

            if file.startswith('~'):
                continue

            file_path = os.path.join(root, file)
            df = pandas.read_excel(file_path)
            print(f'-- \tLoaded {file_path}')

            df = df.drop([0])

            columns = []
            output_data = {}

            row_by_iloc = df.iloc[0]

            for col in df.columns:
                if 's' in row_by_iloc[col]:
                    columns.append(col)

            for idx, row in df.iterrows():
                if idx <= 1:
                    continue

                item = {}
                for f in columns:
                    item[f] = row[f]

                output_data[item["id"]] = item

            json_dir = root.replace(src, dist)
            json_file = os.path.join(json_dir, file[:-5] + '.json')

            if not os.path.exists(json_dir):
                os.makedirs(json_dir)

            with open(json_file, 'w', encoding='utf-8') as file:
                file.write(json.dumps(output_data, indent=4, ensure_ascii=False))
            
            print(f'-- \tGenerated {json_file}')

            # df.to_csv('%s/%s.csv' % (dist_dir, file[:-5]), index=False)
            count += 1

    print(f'-- {count} files has done')
