import os
import sys
import argparse

def compile_code(params):
    # compile code
    cmd = 'make clean'
    res = os.system(cmd)
    if res != 0:
        return res
    cmd = 'make all' + params
    print(cmd)
    res = os.system(cmd)
    if res != 0:
        return res

    return 0

# app_sets = ['beacon', 
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
app_sets = ['beacon']

# port_sets = ['windows_libusb_win32', 
#              'windows_serial', ]
port_sets = ['windows_serial', ]

# chipset_sets = ['ats2851', 
#                 'common', 
#                 'csr8510', 
#                 'csr8910', 
#                 'pts_dongle',]
chipset_sets = ['ats2851',]

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cpu-arch",
                        help="Windows libusb need this.")
                        
    return parser.parse_args()

if __name__ == '__main__':
    args = parse_args()

    params = ' V=1'
    cpu_arch = args.cpu_arch
    if cpu_arch != None:
        params += (' CPU_ARCH=%s') % (cpu_arch)

    res = compile_code(params)
    if(res != 0):
        sys.exit(res)

    for app in app_sets:
        for port in port_sets:
            for chipset in chipset_sets:
                params_full = params + (' APP=%s PORT=%s CHIPSET=%s') % (app, port, chipset)
                res = compile_code(params_full)
                if(res != 0):
                    sys.exit(res)
    
    sys.exit(0)