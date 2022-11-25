import os
import sys
from kconfiglib import Kconfig
from menuconfig import menuconfig

def project_menuconfig(config='.config', file='Kconfig'):
    # os.environ['MENUCONFIG_STYLE'] = 'default path=fg:black,bg:white separator=fg:white,bg:blue,bold selection=fg:white,bg:red,bold help=path'
    # os.environ['KCONFIG_CONFIG_HEADER'] = '#\n# Automatically generated file: Do not edit !!!\n#\n'
    # os.environ['KCONFIG_CONFIG'] = config

    # 判断是否在Windows下执行, 若是则启动一个独立的命令行提示符窗口来执行menuconfig
    if sys.platform.startswith('win'):
        os.system('start /wait cmd.exe /c menuconfig.exe %s' % file)
        #write_autoconf
        #Kconfig(filename=config)
        kconf = Kconfig(file, warn_to_stderr=False,
                    suppress_traceback=True)
        print(kconf.load_config(config, False))
        print(kconf.load_config('prj.config', False))
        print(kconf.load_config('overwrite.config', False))
        #print(kconf.write_config(config))
        print(kconf.write_autoconf("test.h"))
        # os.system('start /wait cmd.exe /c guiconfig.exe %s' % file)
    else:
        menuconfig(Kconfig(filename=file))
        # guiconfig(Kconfig(filename=file))


project_menuconfig()
