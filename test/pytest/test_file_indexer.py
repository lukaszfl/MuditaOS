# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
from harness.harness import Harness
import pytest
import eyed3
import sqlite3
import shutil
import os
import time
from harness.api.filesystem import get_file, put_file

def remove_file(harness, filename):
    body = {"fs": True, "removeFile": filename}
    return harness.endpoint_request("developerMode", "put", body)

def rename_file(harness, filename, destfilename):
    body = {"fs": True, "renameFile": filename, "destfilename": destfilename}
    return harness.endpoint_request("developerMode", "put", body)

def list_files(harness, directory):
    body = {"fs": True, "listDir": directory}
    return harness.endpoint_request("developerMode", "get", body)["body"][directory]

def add_tags(filename, title, artist, album):
    audioFile = eyed3.load(filename)
    audioFile.tag.title = title
    audioFile.tag.artist = artist
    audioFile.tag.album = album
    audioFile.tag.save()

def check_db(harness, fileName):
    try:
        os.remove(fileName)
    except OSError:
        pass
    get_file(harness, fileName, "./", "/sys/user/")
    fs_resp = list_files(harness, "/sys/user/music")
    fs_list = [item["path"] for item in fs_resp]
    con = sqlite3.connect(fileName)
    cur = con.cursor()
    db_tuple = cur.execute('SELECT path FROM files')
    db_list = [item[0] for item in db_tuple]
    for a in fs_list:
        print("FS: " +a)
    for b in db_list:
        print("DB: " +b)
    result = all(elem in db_list for elem in fs_list)
    con.close()
    try:
        os.remove(fileName)
    except OSError:
        pass
    assert result is True

def check_db_tags(harness, fileNameDb, assetFileName, title, artist, album):
    get_file(harness, fileNameDb, "./", "/sys/user/")
    path = "/sys/user/music/" + assetFileName
    con = sqlite3.connect(fileNameDb)
    cur = con.cursor()
    query = "SELECT path, title, artist, album FROM files"
    print(query)
    for row in cur.execute(query):
        print("%s vs %s\n%s vs %s\n%s vs %s\n" % (row[0], path, row[1], title, row[2], artist))
        if str(row[0]) == path and (str(row[1]) == title or str(row[1]) == assetFileName) and str(row[2]) == artist:
            return True
    return False

@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_ends_test_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_file_indexer(harness):
    # 1
    multimediadb = "multimedia.db"
    check_db(harness, multimediadb)
    # 2
    filename = "file1.mp3"
    put_file(harness, filename, "/sys/user/music", "test.mp3")
    ret = check_db_tags(harness, multimediadb, "test.mp3", "File 1 title", "File 1 artist", "File 1 album")
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass

    # 3
    filename = "file2.mp3"
    put_file(harness, filename, "/sys/user/music", "empty.mp3")
    ret = check_db_tags(harness, multimediadb, "empty.mp3", "", "", "")
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass

    # 4
    filename = "file3.mp3"
    put_file(harness, filename, "/sys/user/music", "test.mp3")
    ret = check_db_tags(harness, multimediadb, "test.mp3", "File 3 title", "File 3 artist", "File 3 album")
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass

    # 5
    filename = "file4.mp3"
    put_file(harness, filename, "/sys/user/music", "test.mp3")
    ret = check_db_tags(harness, multimediadb, "test.mp3", "", "", "")
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass


    # 6
    destfilename = "destfilename.mp3"
    rename_file(harness, "/sys/user/music/test.mp3", "/sys/user/music/" + destfilename)
    ret = check_db_tags(harness, multimediadb, destfilename, "", "", "")
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass    
    
    # 7
    filename = "file5.mp3"
    put_file(harness, filename, "/sys/user/music", "empty.mp3")
    ret = check_db_tags(harness, multimediadb, "empty.mp3", "File 5 title", "File 5 artist", "File 5 album")
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass

    # 8
    remove_file(harness, "/sys/user/music/destfilename.mp3")
    check_db(harness, multimediadb)

    # 9
    remove_file(harness, "/sys/user/music/empty.mp3")
    check_db(harness, multimediadb)
