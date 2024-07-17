import hashlib
import os
import sys
def calc_sha256(file_name):
    sha256 = hashlib.sha256()
    with open(file_name, 'rb') as f:
        while True:
            data = f.read(8192)
            if not data:
                break
            sha256.update(data)
    return sha256.hexdigest()

sha256 = calc_sha256(sys.argv[1])
print("file %s sha256: %x", sys.argv[1], sha256)
