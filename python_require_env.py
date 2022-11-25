import os
# ram_report&rom_report need this
cmd = 'python -m pip install -r scripts/footprint/requirements.txt'
res = os.popen(cmd).read()
print(res)
# kconfig need this
cmd = 'python -m pip install -r scripts/kconfig/requirements.txt'
res = os.popen(cmd).read()
print(res)

# docs need this
cmd = 'python -m pip install -r doc/requirements.txt'
res = os.popen(cmd).read()
print(res)