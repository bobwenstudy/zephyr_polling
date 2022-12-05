import os
import sys

# ram_report&rom_report need this
cmd = 'python -m pip install -r scripts/footprint/requirements.txt'
res = os.system(cmd)
if res != 0:
    sys.exit(res)
# kconfig need this
cmd = 'python -m pip install -r scripts/kconfig/requirements.txt'
res = os.system(cmd)
if res != 0:
    sys.exit(res)

# docs need this
cmd = 'python -m pip install -r doc/requirements.txt'
res = os.system(cmd)
if res != 0:
    sys.exit(res)