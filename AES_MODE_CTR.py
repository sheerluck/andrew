"""."""
# Copyright (c) 2012 Asim Ihsan (asim dot ihsan at gmail dot com)
# Distributed under the MIT/X11 software license, see the accompanying
# file license.txt or http://www.opensource.org/licenses/mit-license.php.

import io
import os
import struct
from typing import Final, Sequence

from Crypto.Cipher import AES
from Crypto.Cipher._mode_ctr import CtrMode
from Crypto.Hash import HMAC, SHA256
from Crypto.Util import Counter

import argon2
import click


SALT_LEN: Final = 16
NONCE_LEN: Final = 8
HMAC_LEN: Final = 32
header_format: Final = ''.join([">",
                                f"{SALT_LEN}s",    # Password salt
                                f"{NONCE_LEN}s",   # CTR nonce
                                f"{SALT_LEN}s"])   # HMAC salt


def get_hmac(key: bytes, salts: Sequence[bytes]) -> HMAC.HMAC:
    """."""
    hmac_password_derived = argon2.hash_password_raw(
        hash_len=32, password=key, salt=salts[2],  # hmac_salt
        type=argon2.low_level.Type.ID)
    bytes_to_hmac = b''.join(salts)
    return HMAC.new(key=hmac_password_derived,
                    msg=bytes_to_hmac,
                    digestmod=SHA256)


def get_aes(key: bytes, salts: Sequence[bytes]) -> CtrMode:
    """."""
    cipher_password_derived = argon2.hash_password_raw(
        hash_len=32, password=key, salt=salts[0],  # password_salt
        type=argon2.low_level.Type.ID)
    ctr_counter = Counter.new(64,
                              prefix=salts[1],  # nonce
                              initial_value=5988931115977,
                              little_endian=True)
    ctr = AES.new(key=cipher_password_derived,
                  mode=AES.MODE_CTR,
                  counter=ctr_counter)
    # make mypy happy
    assert isinstance(ctr, CtrMode)
    return ctr


def encrypt_string(plaintext: bytes, key: bytes):
    """."""
    plaintext_obj = io.BytesIO(plaintext)
    ciphertext_obj = io.BytesIO()
    encrypt_file(plaintext_obj, ciphertext_obj, key)
    return ciphertext_obj.getvalue()


def decrypt_string(ciphertext: bytes, key: bytes):
    """."""
    plaintext_obj = io.BytesIO()
    ciphertext_obj = io.BytesIO(ciphertext)
    decrypt_file(ciphertext_obj, plaintext_obj, key)
    return plaintext_obj.getvalue()


def get_size(file_obj) -> int:
    """."""
    file_obj.seek(0, os.SEEK_END)
    fsize = file_obj.tell()
    file_obj.seek(0, os.SEEK_SET)
    return fsize - (SALT_LEN + NONCE_LEN + SALT_LEN + HMAC_LEN)


def get_salts(file_obj) -> Sequence[bytes]:
    """."""
    salts_block = file_obj.read(SALT_LEN + NONCE_LEN + SALT_LEN)
    try:
        salts = struct.unpack(header_format, salts_block)
    except struct.error:
        raise ValueError("Start of body is invalid.")
    return salts


def decrypt_file(ciphertext_file_obj,
                 plaintext_file_obj,
                 key: bytes,
                 chunk_size=4096):
    """."""
    ciphertext_size = get_size(ciphertext_file_obj)
    salts = get_salts(ciphertext_file_obj)
    hmac_object = get_hmac(key, salts)

    # ------------------------------------------------------------------------
    #   First pass: stream in the ciphertext object into the HMAC object
    #   and verify that the HMAC is correct.
    # ------------------------------------------------------------------------
    ciphertext_file_pos = ciphertext_file_obj.tell()
    ciphertext_bytes_read = 0
    while True:
        bytes_remaining = ciphertext_size - ciphertext_bytes_read
        current_chunk_size = min(bytes_remaining, chunk_size)
        ciphertext_chunk = ciphertext_file_obj.read(current_chunk_size)
        if ciphertext_chunk == b'':
            break
        ciphertext_bytes_read += len(ciphertext_chunk)
        hmac_object.update(ciphertext_chunk)
    hmac = ciphertext_file_obj.read()
    hmac_calculated = hmac_object.digest()
    if hmac != hmac_calculated:
        raise ValueError(f"{hmac=} != {hmac_calculated=}")

    # ------------------------------------------------------------------------
    #   Second pass: stream in the ciphertext object and decrypt it into the
    #   plaintext object.
    # ------------------------------------------------------------------------
    cipher_ctr = get_aes(key, salts)
    ciphertext_file_obj.seek(ciphertext_file_pos, os.SEEK_SET)
    ciphertext_bytes_read = 0
    while True:
        bytes_remaining = ciphertext_size - ciphertext_bytes_read
        current_chunk_size = min(bytes_remaining, chunk_size)
        ciphertext_chunk = ciphertext_file_obj.read(current_chunk_size)
        if ciphertext_chunk == b'':
            break
        ciphertext_bytes_read += len(ciphertext_chunk)
        plaintext_chunk = cipher_ctr.decrypt(ciphertext_chunk)
        plaintext_file_obj.write(plaintext_chunk)
    if ciphertext_bytes_read != ciphertext_size:
        msg = f"second pass {ciphertext_bytes_read=} != {ciphertext_size=}"
        raise ValueError(msg)


def encrypt_file(plaintext_file_obj,
                 ciphertext_file_obj,
                 key: bytes,
                 chunk_size=4096):
    """."""
    salts = [os.urandom(x) for x in [SALT_LEN, NONCE_LEN, SALT_LEN]]
    cipher_ctr = get_aes(key, salts)
    hmac_object = get_hmac(key, salts)
    header = struct.pack(header_format, *salts)
    ciphertext_file_obj.write(header)

    # ------------------------------------------------------------------------
    #   Stream in the input file and stream out ciphertext into the
    #   ciphertext file.
    # ------------------------------------------------------------------------
    while True:
        plaintext_chunk = plaintext_file_obj.read(chunk_size)
        if plaintext_chunk == b'':
            break
        ciphertext_chunk = cipher_ctr.encrypt(plaintext_chunk)
        ciphertext_file_obj.write(ciphertext_chunk)
        hmac_object.update(ciphertext_chunk)

    # ------------------------------------------------------------------------
    #   Write the HMAC to the ciphertext file.
    # ------------------------------------------------------------------------
    hmac = hmac_object.digest()
    ciphertext_file_obj.write(hmac)


@click.command()
@click.option('--test', default=False, help='try --test=True')
@click.option('--file', default="/tmp/LOL.mp4", help='path to file')
@click.option('--fkey', default="key-10Mb.dat", help='path to file')
@click.option('--command', default="encrypt", help='encrypt/decrypt')
def main(test: str, file: str, fkey: str, command: str) -> int:
    """First line should be in imperative mood; try rephrasing."""
    if test:
        key, plaintext = os.urandom(9_000), os.urandom(9_000)
        ciphertext = encrypt_string(plaintext, key)
        decryptedtext = decrypt_string(ciphertext, key)
        print("it works!" if plaintext == decryptedtext else "wtf?")
        from binascii import hexlify
        ahash = argon2.hash_password_raw(
                hash_len=32, password=b"hop", salt=b"1234567812345678",
                type=argon2.low_level.Type.ID)
        print(hexlify(ahash))
        return 0
    fres = f"{file}.{command}ed"
    with open(file, 'rb') as f, \
         open(fres, 'w+b') as w, \
         open(fkey, 'rb') as k:
        if command == "encrypt":
            encrypt_file(f, w, key=k.read())
        elif command == "decrypt":
            decrypt_file(f, w, key=k.read())
        else:
            raise ValueError(f"'{command}' must be 'encrypt' or 'decrypt'")
    return 0


if __name__ == "__main__":
    exit(main())
