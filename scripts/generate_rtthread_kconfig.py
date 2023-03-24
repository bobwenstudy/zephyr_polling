import os


def get_example_list():
    path = 'example'
    app_list = []

    files = os.listdir(path)
    for file in files:
        file_path = os.path.join(path, file)

        if os.path.isdir(file_path):
            app_list.append(file)
    
    return app_list






port_sets = ['rtthread_uart', 
             'rtthread_artpi',]

chipset_sets = ['artpi_ap6212', 
                'csr8910', 
                'pts_dongle',
                'common', ]

def get_kconfig_list(type, sets):
    for set in sets:
        tmp_str = "PKG_ZEPHYR_POLLING_%s_%s" % (type, set.upper())
        print("config %s" % tmp_str)
        print("    bool \"%s\"" % set)
        print("")
    
    print("")
    print("")
    for set in sets:
        tmp_str = "PKG_ZEPHYR_POLLING_%s_%s" % (type, set.upper())
        print("default \"%s\"    if %s" % (set, tmp_str))
    print("")
    print("")
    print("")

if __name__ == '__main__':
    app_sets = get_example_list()

    get_kconfig_list("EXAMPLE", app_sets)
    get_kconfig_list("CHIPSET", chipset_sets)
    get_kconfig_list("PORT", port_sets)

