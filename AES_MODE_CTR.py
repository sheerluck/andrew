"""."""
# Copyright (c) 2012 Asim Ihsan (asim dot ihsan at gmail dot com)
# Distributed under the MIT/X11 software license, see the accompanying
# file license.txt or http://www.opensource.org/licenses/mit-license.php.

import io
import os
import struct

from Crypto.Cipher import AES
from Crypto.Hash import HMAC, SHA256
from Crypto.Protocol.KDF import PBKDF2
from Crypto.Util import Counter

import click


SALT_LEN = 16
NONCE_LEN = 8
pbkdf2_count = 3_222_111
pbkdf2_dk_len = 32


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


def decrypt_file(ciphertext_file_obj,
                 plaintext_file_obj,
                 key: bytes,
                 chunk_size=4096):
    """."""
    header_format = ">QH"
    header_size = struct.calcsize(header_format)
    header_string = ciphertext_file_obj.read(header_size)
    try:
        header = struct.unpack(header_format, header_string)
    except struct.error:
        raise ValueError("Header is invalid.")
    ciphertext_size = header[0]
    hmac_size = header[1]

    encrypted_string_format = ''.join([">",
                                       "%ss" % (SALT_LEN, ),
                                       "%ss" % (NONCE_LEN, ),
                                       "%ss" % (SALT_LEN, )])
    encrypted_string_size = struct.calcsize(encrypted_string_format)
    body_string = ciphertext_file_obj.read(encrypted_string_size)
    try:
        body = struct.unpack(encrypted_string_format, body_string)
    except struct.error:
        raise ValueError("Start of body is invalid.")
    password_salt = body[0]
    nonce = body[1]
    hmac_salt = body[2]

    hmac_password_derived = PBKDF2(password=key,
                                   salt=hmac_salt,
                                   dkLen=pbkdf2_dk_len,
                                   count=pbkdf2_count,
                                   hmac_hash_module=SHA256)
    elems_to_hmac = [password_salt, nonce, hmac_salt]
    bytes_to_hmac = b''.join(elems_to_hmac)
    hmac_object = HMAC.new(key=hmac_password_derived,
                           msg=bytes_to_hmac,
                           digestmod=SHA256)

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
    if ciphertext_bytes_read != ciphertext_size:
        msg = f"first pass {ciphertext_bytes_read=} != {ciphertext_size=}"
        raise ValueError(msg)

    # the rest of the file is the HMAC.
    hmac = ciphertext_file_obj.read()
    if len(hmac) != hmac_size:
        msg = f"{len(hmac)=} != {hmac_size=}"
        raise ValueError(msg)

    hmac_calculated = hmac_object.digest()
    if hmac != hmac_calculated:
        msg = f"{hmac=} != {hmac_calculated=}"
        raise ValueError(msg)

    # ------------------------------------------------------------------------
    #   Second pass: stream in the ciphertext object and decrypt it into the
    #   plaintext object.
    # ------------------------------------------------------------------------
    cipher_password_derived = PBKDF2(password=key,
                                     salt=password_salt,
                                     dkLen=pbkdf2_dk_len,
                                     count=pbkdf2_count,
                                     hmac_hash_module=SHA256)
    ctr_counter = Counter.new(64,
                              prefix=nonce,
                              initial_value=2506025630791,
                              little_endian=True)
    assert len(cipher_password_derived) == 32
    cipher_ctr = AES.new(key=cipher_password_derived,
                         mode=AES.MODE_CTR,
                         counter=ctr_counter)
    ciphertext_file_obj.seek(ciphertext_file_pos, os.SEEK_SET)
    ciphertext_bytes_read = 0
    while True:
        bytes_remaining = ciphertext_size - ciphertext_bytes_read
        current_chunk_size = min(bytes_remaining, chunk_size)
        ciphertext_chunk = ciphertext_file_obj.read(current_chunk_size)
        end_of_file = ciphertext_chunk == b''
        ciphertext_bytes_read += len(ciphertext_chunk)
        plaintext_chunk = cipher_ctr.decrypt(ciphertext_chunk)
        plaintext_file_obj.write(plaintext_chunk)
        if end_of_file:
            break
    if ciphertext_bytes_read != ciphertext_size:
        msg = f"second pass {ciphertext_bytes_read=} != {ciphertext_size=}"
        raise ValueError(msg)


def encrypt_file(plaintext_file_obj,
                 ciphertext_file_obj,
                 key: bytes,
                 chunk_size=4096):
    """."""
    password_salt = os.urandom(SALT_LEN)
    cipher_password_derived = PBKDF2(password=key,
                                     salt=password_salt,
                                     dkLen=pbkdf2_dk_len,
                                     count=pbkdf2_count,
                                     hmac_hash_module=SHA256)
    nonce = os.urandom(NONCE_LEN)
    ctr_counter = Counter.new(64,
                              prefix=nonce,
                              initial_value=2506025630791,
                              little_endian=True)
    assert len(cipher_password_derived) == 32
    cipher_ctr = AES.new(key=cipher_password_derived,
                         mode=AES.MODE_CTR,
                         counter=ctr_counter)

    hmac_salt = os.urandom(SALT_LEN)
    hmac_password_derived = PBKDF2(password=key,
                                   salt=hmac_salt,
                                   dkLen=pbkdf2_dk_len,
                                   count=pbkdf2_count,
                                   hmac_hash_module=SHA256)
    elems_to_hmac = [password_salt, nonce, hmac_salt]
    bytes_to_hmac = b''.join(elems_to_hmac)
    hmac_object = HMAC.new(key=hmac_password_derived,
                           msg=bytes_to_hmac,
                           digestmod=SHA256)

    header_format = ''.join([">",
                             "Q",               # Length of ciphertext
                             "H",               # Length of HMAC
                             f"{SALT_LEN}s",    # Password salt
                             f"{NONCE_LEN}s",   # CTR nonce
                             f"{SALT_LEN}s"])   # HMAC salt
    header = struct.pack(header_format,
                         0,        # This is the ciphertext size, wrong for now
                         hmac_object.digest_size,
                         password_salt,
                         nonce,
                         hmac_salt)
    ciphertext_file_obj.write(header)

    # ------------------------------------------------------------------------
    #   Stream in the input file and stream out ciphertext into the
    #   ciphertext file.
    # ------------------------------------------------------------------------
    ciphertext_size = 0
    while True:
        plaintext_chunk = plaintext_file_obj.read(chunk_size)
        end_of_file = plaintext_chunk == b''
        ciphertext_chunk = cipher_ctr.encrypt(plaintext_chunk)
        ciphertext_size += len(ciphertext_chunk)
        ciphertext_file_obj.write(ciphertext_chunk)
        hmac_object.update(ciphertext_chunk)
        if end_of_file:
            break

    # ------------------------------------------------------------------------
    #   Write the HMAC to the ciphertext file.
    # ------------------------------------------------------------------------
    hmac = hmac_object.digest()
    ciphertext_file_obj.write(hmac)

    # ------------------------------------------------------------------------
    #   Go back to the header and update the ciphertext size.
    # ------------------------------------------------------------------------

    # Read in.
    ciphertext_file_obj.seek(0, os.SEEK_SET)
    header_format = ">Q"
    header_size = struct.calcsize(header_format)
    header = ciphertext_file_obj.read(header_size)

    # Modify.
    header_elems = list(struct.unpack(header_format, header))
    header_elems[-1] = ciphertext_size

    # Write out.
    header = struct.pack(header_format, *header_elems)
    ciphertext_file_obj.seek(0, os.SEEK_SET)
    ciphertext_file_obj.write(header)


@click.command()
@click.option('--test', default="False", help='try --test=True')
@click.option('--file', default="/tmp/LOL.mp4", help='path to file')
@click.option('--fkey', default="key-10Mb.dat", help='path to file')
@click.option('--command', default="encrypt", help='encrypt/decrypt')
def main(test: str, file: str, fkey: str, command: str) -> int:
    """First line should be in imperative mood; try rephrasing."""
    if test == "True":
        key, plaintext = os.urandom(9_000), os.urandom(9_000)
        ciphertext = encrypt_string(plaintext, key)
        decryptedtext = decrypt_string(ciphertext, key)
        print("it works!" if plaintext == decryptedtext else "wtf?")
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
