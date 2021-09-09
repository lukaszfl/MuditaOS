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
    get_file(harness, fileName, "./", "/sys/user/")
    fs_resp = list_files(harness, "/sys/user/music")
    fs_list = [item["path"] for item in fs_resp]
    con = sqlite3.connect(fileName)
    cur = con.cursor()
    db_tuple = cur.execute('SELECT path FROM files')
    db_list = [item[0] for item in db_tuple]
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
        if str(row[0]) == path and str(row[1]) == title and str(row[2]) == artist:
            return True
        
        if str(row[0]) == path and not str(row[1]) and not title and not str(row[2]) and not artist:
            return True

    return False
    # rows_count = len(cur.execute(query).fetchall())
    # try:
    #     os.remove(fileNameDb)
    # except OSError:
    #     pass
    # assert rows_count > 0


# def test_2(harness, multimediadb, filename, title, artist, album):
#     add_tags(filename, title, artist, album)
#     put_file(harness, filename, "/sys/user/music/")
#     test_3(harness, multimediadb, filename, title, artist, album)

# def test_3(harness, multimediadb, filename, title, artist, album):
#     time.sleep(2)
#     ret = check_db_tags(harness, multimediadb, filename, title, artist, album)
#     assert ret is True
#     try:
#         os.remove(multimediadb)
#     except OSError:
#         pass

@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_ends_test_in_desktop")
@pytest.mark.usefixtures("phone_unlocked")
def test_file_indexer(harness):
    # remove_file(harness, "/sys/user/music/test.mp3")
    # 1
    multimediadb = "multimedia.db"
    check_db(harness, multimediadb)
    time.sleep(2)
    # 2
    filename = "test.mp3"
    title = u"Harness test title"
    artist = u"Harness test artist"
    album = u"Harness test album"
    add_tags(filename, title, artist, album)
    put_file(harness, filename, "/sys/user/music")
    time.sleep(2)
    ret = check_db_tags(harness, multimediadb, filename, title, artist, album)
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass

    # 3
    emptyTagFileName = "emptyTag.mp3"
    shutil.copy(filename, emptyTagFileName)
    emptyTitle = ""
    emptyArtist = ""
    emptyAlbum = ""
    add_tags(emptyTagFileName, emptyTitle, emptyArtist, emptyAlbum)
    time.sleep(2)
    put_file(harness, emptyTagFileName, "/sys/user/music")
    time.sleep(2)
    ret = check_db_tags(harness, multimediadb, emptyTagFileName, emptyTitle, emptyArtist, emptyAlbum)
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass

    # # 4
    title = u"Abc"
    artist = u"Def"
    album = u"Asdf"
    add_tags(filename, title, artist, album)
    put_file(harness, filename, "/sys/user/music")
    time.sleep(2)
    ret = check_db_tags(harness, multimediadb, filename, title, artist, album)
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass

    # # 5
    add_tags(filename, emptyTitle, emptyArtist, emptyAlbum)
    put_file(harness, filename, "/sys/user/music")
    time.sleep(2)
    ret = check_db_tags(harness, multimediadb, filename, emptyTitle, emptyArtist, emptyAlbum)
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass


    # # 6
    destfilename = "destfilename.mp3"
    rename_file(harness, "/sys/user/music/" + filename, "/sys/user/music/" + destfilename)
    ret = check_db_tags(harness, multimediadb, destfilename, emptyTitle, emptyArtist, emptyAlbum)
    assert ret is True
    try:
        os.remove(multimediadb)
    except OSError:
        pass    
    
    # remove_file("sys/user/music/" + destfilename)

