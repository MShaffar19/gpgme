#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import absolute_import, division, unicode_literals

# Copyright (C) 2018 Ben McGinnes <ben@gnupg.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License and the GNU
# Lesser General Public Licensefor more details.
#
# You should have received a copy of the GNU General Public License and the GNU
# Lesser General Public along with this program; if not, see
# <http://www.gnu.org/licenses/>.

import gpg
import sys

if len(sys.argv) == 3:
    ciphertext = sys.argv[1]
    newfile = sys.argv[2]
elif len(sys.argv) == 2:
    ciphertext = sys.argv[1]
    newfile = input("Enter path and filename of file to save decrypted data to: ")
else:
    ciphertext = input("Enter path and filename of encrypted file: ")
    newfile = input("Enter path and filename of file to save decrypted data to: ")

with open(ciphertext, "rb") as cfile:
    plaintext, result, verify_result = gpg.Context().decrypt(cfile)

with open(newfile, "wb") as nfile:
    nfile.write(plaintext)