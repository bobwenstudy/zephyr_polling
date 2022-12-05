import os
import sys

def compile_code(app, port, chipset):
    params = (' APP=%s PORT=%s CHIPSET=%s') % (app, port, chipset)
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

if __name__ == '__main__':
    for app in app_sets:
        for port in port_sets:
            for chipset in chipset_sets:
                res = compile_code(app, port, chipset)
                if(res != 0):
                    sys.exit(res)
    
    sys.exit(0)