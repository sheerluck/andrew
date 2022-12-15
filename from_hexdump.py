import sh
import gzip
import bz2
import tarfile

hexdump = """\
00000000: 1f8b 0808 ... data2.bin
...
00000250: ... 3d02 0000 BB9<sh="""

lines = hexdump.split("\n")
bo = []
for line in lines:
    head, *wat, tail = line.split()
    bi = []
    for w in wat:
        try:
            x = int(w, 16)
            y = x.to_bytes(2, 'big')
            bi.append(y)
        except:
            pass
    ba = b"".join(bi)
    bo.append(ba)

with open("/tmp/wat.txt", "wb") as f:
    for b in bo:
        f.write(b)

counter = 0
while True:
    print(f"{counter}", end=": ")
    t = str(sh.file("/tmp/wat.txt"))
    print(t[14:-1])
    if "gzip compressed data" in t:
        sh.mv("/tmp/wat.txt", "/tmp/wat.gz")
        with gzip.open('/tmp/wat.gz', 'rb') as gf:
            file_content = gf.read()
            with open("/tmp/wat.txt", "wb") as f:
                f.write(file_content)
    elif "bzip2 compressed data" in t:
        sh.mv("/tmp/wat.txt", "/tmp/wat.bz2")
        with bz2.open("/tmp/wat.bz2", "rb") as bf:
            file_content = bf.read()
            with open("/tmp/wat.txt", "wb") as f:
                f.write(file_content)
    elif "POSIX tar archive" in t:
        sh.mv("/tmp/wat.txt", "/tmp/wat.tar")
        with tarfile.open("/tmp/wat.tar", "r") as tf:
            tname = tf.getnames()[0]

            import os

            def is_within_directory(directory, target):

                abs_directory = os.path.abspath(directory)
                abs_target = os.path.abspath(target)

                prefix = os.path.commonprefix([abs_directory, abs_target])

                return prefix == abs_directory

            def safe_extract(tar, path=".", members=None, *, numeric_owner=False):

                for member in tar.getmembers():
                    member_path = os.path.join(path, member.name)
                    if not is_within_directory(path, member_path):
                        raise Exception("Attempted Path Traversal in Tar File")

                tar.extractall(path, members, numeric_owner=numeric_owner) 


            safe_extract(tf, path="/tmp/", members=tf)
            with (open(f"/tmp/{tname}", "rb") as r,
                  open(f"/tmp/wat.txt", "wb") as f):
                file_content = r.read()
                f.write(file_content)
    else:
        break
    counter += 1
