import os
import sys
import argparse
import shutil

def generate_autoconfig_h(app, clear_all):
    params = (' APP=%s') % (app)
    app_path = ('example/%s') % (app)
    autoconfig_h_path = 'output/autoconfig.h'
    # generate autoconfig
    cmd = 'make clean'
    res = os.system(cmd)
    if res != 0:
        return res

    if clear_all:
        try:
            os.remove(app_path + '/autoconfig.h') # 这个可以删除单个文件，不能删除文件夹
        except BaseException as e:
            print(e)
    else:
        cmd = 'make ' + autoconfig_h_path + params
        print(cmd)
        res = os.system(cmd)
        if res != 0:
            return res

        shutil.copy(autoconfig_h_path, app_path)

    return 0

def get_example_list():
    path = 'example'
    app_list = []

    files = os.listdir(path)
    for file in files:
        file_path = os.path.join(path, file)

        if os.path.isdir(file_path):
            app_list.append(file)
    
    return app_list

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--clear-all",
                        help="clear all generate autoconfig.h.")
                        
    return parser.parse_args()

if __name__ == '__main__':
    args = parse_args()

    clear_all = args.clear_all

    total_work_cnt = 0

    app_sets = get_example_list()
    for app in app_sets:
        total_work_cnt += 1

    current_work_cnt = 0
    for app in app_sets:
        current_work_cnt += 1
        print("=================================================================================")
        print("Total Work Cnt: %d, Current Cnt: %d, Process: %.2f%%" 
            % (total_work_cnt, current_work_cnt, current_work_cnt * 100.0 / total_work_cnt))
        print("=================================================================================")

        res = generate_autoconfig_h(app, clear_all)
        if(res != 0):
            sys.exit(res)
    
    sys.exit(0)