#!/usr/bin/python3

import glob,os,sys


def get_header_def_val(prefix_str, file_name):
    file_name_up = file_name.split(".")[0].upper()
    if len(prefix_str) == 0:
        format_str = "_ZEPHYR_POLLING_%s_H_" % (file_name_up)
    else:
        format_str = "_ZEPHYR_POLLING_%s_%s_H_" % (prefix_str.upper(), file_name_up)
    return format_str

def clear_content(pfile):
    pfile.seek(0)
    pfile.truncate()

def guard(prefix_str, file_name):
    file = open(file_name, 'r+')
    content = file.read()
    
    file_name_format = get_header_def_val(prefix_str, file_name)
    guard_begin = '#ifndef ' + file_name_format + '\n'    \
            '#define ' + file_name_format + '\n\n' 
    guard_end = '\n#endif /* %s */' % (file_name_format)
    new_content = guard_begin + content + guard_end
    
    if content.find(guard_begin) >= 0:
        return

    clear_content(file) 
    file.write(new_content)

if __name__ == '__main__':
    search_dir = ""
    if len(sys.argv) == 1:
        sys.exit('Please provide a directory')
    else:
        search_dir = sys.argv[1]
    
    prefix = ""
    if len(sys.argv) == 3:
        prefix = sys.argv[2]
    
    # enter search directory
    os.chdir(search_dir)

    for file in glob.glob("*.h"):
        guard(prefix, file)