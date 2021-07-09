# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

import os
import pytest
from harness import log
from harness.interface.defs import Method, Endpoint
from harness.request import Transaction, Request,TransactionError

from harness.harness import Harness
import binascii


def send_file(harness: Harness, file: str, where: str) -> Transaction:

    fileSize = os.path.getsize(file)


    with open(file, 'rb') as l_file:
        file_data = l_file.read()
        fileCrc32 = format((binascii.crc32(file_data) & 0xFFFFFFFF), '08x')

    body = {"fileName": where + file,
            "fileSize": fileSize, "fileCrc32": fileCrc32}

    log.debug(f"sending {body}")

    ret = harness.request(Endpoint.FILESYSTEM, Method.PUT, body)

    log.debug(f"response {ret.response}")
    assert ret.response.body["txID"] != 0

    txID = ret.response.body["txID"]
    chunkSize = ret.response.body["chunkSize"]

    chunkNo = 1
    sent = 0

    from functools import partial
    import base64
    import time
    with open(file, 'rb') as l_file:
        for chunk in iter(partial(l_file.read, chunkSize), b''):
            data = base64.standard_b64encode(chunk).decode()

            body = {"txID": txID, "chunkNo": chunkNo, "data": data}
            ret = harness.request(Endpoint.FILESYSTEM, Method.PUT, body)
            time.sleep(.003)
            chunkNo += 1
            sent = sent + chunkSize
            pp = (sent / fileSize) * 100
            t_time = ret.send_time + ret.read_time
            kbps = len(data)*8/t_time/1024
            log.info(f"sent: {sent} of {fileSize} is: {pp:.2f}% in time: {t_time:.3f} kbps: {kbps:.3f}")

    return ret


@pytest.mark.usefixtures("phone_unlocked")
@pytest.mark.rt1051
def test_update(harness: Harness):
    filename = "update.tar"
    file = os.getcwd() + "/" + filename

    import time
    start = time.time()
    try:
        send_file(harness, filename, "/sys/user/")
        result = harness.request(Endpoint.DEVELOPERMODE, Method.POST, {"update": True, "reboot": True})
        log.info(f"ini time: {result.read_time:.3f}")
    except TransactionError as err:
        log.error(f"{err.message} Status: {err.status}")
    duration = time.time() - start
    log.info(f"update done {duration:.3f}!")
