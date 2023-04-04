import os
import sys
import argparse

def get_example_list():
    path = 'example'
    app_list = []

    files = os.listdir(path)
    for file in files:
        file_path = os.path.join(path, file)

        if os.path.isdir(file_path):
            app_list.append(file)
    
    return app_list

def compile_code(params):
    # compile code
    cmd = 'make clean'
    res = os.system(cmd)
    if res != 0:
        return res
    cmd = 'make all -j' + params
    print(cmd)
    res = os.system(cmd)
    if res != 0:
        return res

    return 0


port_sets = ['windows_libusb_win32', 
             'windows_serial', ]

chipset_sets = ['ats2851', 
                'common', 
                'csr8510', 
                'csr8910', 
                'pts_dongle',]

# app_sets = ['app_test',
#             'beacon', 
#             'broadcaster', 
#             'central', 
#             'central_gatt_write', 
#             'central_hr', 
#             'central_ht',
#             'eddystone',
#             'ibeacon',
#             'observer',
#             'peripheral',
#             'peripheral_csc',
#             'peripheral_dis',
#             'peripheral_esp',
#             'peripheral_gatt_write',
#             'peripheral_hids',
#             'peripheral_hr',
#             'peripheral_ht',
#             'peripheral_throughput', ]

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cpu-arch",
                        help="Windows libusb need this.")
    parser.add_argument("--full-check",
                        action="store_true",
                        default=False,
                        help="For normal build.")
                        
    return parser.parse_args()

if __name__ == '__main__':
    args = parse_args()

    #params = ' V=1'
    params = ''
    cpu_arch = args.cpu_arch
    if cpu_arch != None:
        params += (' CPU_ARCH=%s') % (cpu_arch)

    app_sets = get_example_list()

    res = compile_code(params)
    if(res != 0):
        sys.exit(res)

    full_check = args.full_check
    if full_check:
        total_work_cnt = 0
        for app in app_sets:
            for port in port_sets:
                for chipset in chipset_sets:
                    total_work_cnt += 1

        current_work_cnt = 0
        for app in app_sets:
            for port in port_sets:
                for chipset in chipset_sets:
                    current_work_cnt += 1
                    print("=================================================================================")
                    print("Total Work Cnt: %d, Current Cnt: %d, Process: %.2f%%" 
                        % (total_work_cnt, current_work_cnt, current_work_cnt * 100.0 / total_work_cnt))
                    print("=================================================================================")

                    params_full = params + (' APP=%s PORT=%s CHIPSET=%s') % (app, port, chipset)
                    res = compile_code(params_full)
                    if(res != 0):
                        sys.exit(res)
    
    sys.exit(0)