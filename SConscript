from building import *
import rtconfig

def get_path_files(dirs, file_ext):
    path_files = []
    # print('dir: ' + str(dirs))
    for dir in dirs:
        # print('dir: ' + dir)
        path_files += Glob(dir + '/' + file_ext)
    return path_files

cwd = GetCurrentDir()
path = []
src = []

C_DIRS = []

app = GetConfigValue('PKG_ZEPHYR_POLLING_EXAMPLE').replace('"', '')
chipset = GetConfigValue('PKG_ZEPHYR_POLLING_CHIPSET').replace('"', '')
porting = GetConfigValue('PKG_ZEPHYR_POLLING_PORTING').replace('"', '')

# print("app: %s" % app)
# print("chipset: %s" % chipset)
# print("porting: %s" % porting)

# Include stack info
stack_path = cwd + '/src'
path.append(stack_path)

C_DIRS.append(stack_path)
C_DIRS.append(stack_path + '/base')
C_DIRS.append(stack_path + '/bluetooth')
C_DIRS.append(stack_path + '/common')
C_DIRS.append(stack_path + '/drivers')
C_DIRS.append(stack_path + '/host')
C_DIRS.append(stack_path + '/logging')
C_DIRS.append(stack_path + '/utils')
C_DIRS.append(stack_path + '/services')

tmp_path = cwd + '/example/' + app
path.append(tmp_path)
C_DIRS.append(tmp_path)

tmp_path = cwd + '/chipset/'
path.append(tmp_path)
tmp_path = tmp_path + chipset
path.append(tmp_path)
C_DIRS.append(tmp_path)

tmp_path = cwd + '/platform/'
path.append(tmp_path)
tmp_path = tmp_path + 'rtthread'
path.append(tmp_path)
C_DIRS.append(tmp_path)

tmp_path = cwd + '/porting/'
path.append(tmp_path)
tmp_path = tmp_path + porting
path.append(tmp_path)
C_DIRS.append(tmp_path)

src = get_path_files(C_DIRS, '*.c')

# print("src: %s" % src)

# if rtconfig.CROSS_TOOL == 'keil':
#     LOCAL_CCFLAGS += ' --gnu --diag_suppress=111'
    
group = DefineGroup('zephyr_polling', src, depend = ['PKG_USING_ZEPHYR_POLLING'], CPPPATH = path)

Return('group')
