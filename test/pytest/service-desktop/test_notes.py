# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
import time
import string
import pytest
import random
from harness.interface.defs import status, key_codes


def open_notes_app(harness):
    if harness.get_application_name() != "ApplicationNotes":
        harness.connection.send_key_code(key_codes["enter"])
        harness.open_application("notes")
        if harness.get_application_name() != "ApplicationNotes":
            time.sleep(2)
            assert harness.get_application_name() == "ApplicationNotes"


def generate_random_string(length):
    return ''.join(random.choices(string.digits, k=length))


def add_note_ui(note_body, harness):
    assert harness.get_application_name() == "ApplicationNotes"
    harness.connection.send_key_code(key_codes["left"])
    harness.send_text(note_body)
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(1)
    harness.connection.send_key_code(key_codes["fnRight"])


def add_note(note_body, harness):
    add_msg_body = {"notesCommand": "addNote", "messageBody": note_body}
    ret = harness.endpoint_request("developerMode", "put", add_msg_body)
    return ret


def get_note(note_body, harness):
    get_msg_body = {"notesCommand": "getNoteByText", "messageBody": note_body}
    ret = harness.endpoint_request("developerMode", "get", get_msg_body)
    return ret


def get_all_notes(harness):
    return get_note("", harness)


def remove_note(note, harness):
    remove_msg_body = {"id": note["id"], "notesCommand": "removeNote"}
    ret = harness.endpoint_request("developerMode", "put", remove_msg_body)
    assert ret["status"] == status["OK"]


def append_note_ui(note_body, harness):
    assert harness.get_application_name() == "ApplicationNotes"
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(3)
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(3)
    harness.send_text(note_body)
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(1)
    harness.connection.send_key_code(key_codes["fnRight"])


def remove_characters_in_note(number_of_chars, harness):
    assert harness.get_application_name() == "ApplicationNotes"
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(3)
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(3)
    for i in range(number_of_chars):
        harness.connection.send_key_code(key_codes["#"])
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(1)
    harness.connection.send_key_code(key_codes["fnRight"])


def paste_clipboard(harness):
    assert harness.get_application_name() == "ApplicationNotes"
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(3)
    harness.connection.send_key_code(key_codes["enter"])
    time.sleep(3)
    harness.connection.send_key_code(key_codes["fnLeft"])
    harness.connection.send_key_code(key_codes["down"])
    harness.connection.send_key_code(key_codes["enter"])
    harness.connection.send_key_code(key_codes["fnRight"])


def remove_note_if_exists(note_body, harness, exact_match=True):
    result = get_note(note_body, harness)
    if result["body"] is not None:
        if result["status"] == status["OK"]:
            for note in result["body"]:
                if exact_match:
                    if note["messageBody"] == note_body:
                        remove_note(note, harness)
                else:
                    remove_note(note, harness)


def remove_all_notes(harness):
    result = get_note("", harness)
    if result["body"] is not None:
        if result["status"] == status["OK"]:
            for note in result["body"]:
                remove_note(note, harness)


def set_clipboard_cache(clipboard_string, harness):
    clipboard_copy_msg = {"clipboardCommand": "copy", "messageBody": clipboard_string}
    ret = harness.endpoint_request("developerMode", "put", clipboard_copy_msg)
    return ret


def back_to_home(harness):
    for i in range(5):
        harness.connection.send_key_code(key_codes["fnRight"])


# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_add_note(harness):
#     back_to_home(harness)
#     # test note body
#     note_body = generate_random_string(3)
#
#     remove_note_if_exists(note_body, harness, exact_match=False)
#
#     open_notes_app(harness)
#
#     # adds new note by ui
#     add_note_ui(note_body, harness)
#
#     # gets added note
#     result = get_note(note_body, harness)
#
#     back_to_home(harness)
#
#     assert (len(result["body"]) == 1)
#     assert (result["body"][0]["messageBody"] == note_body)
#
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_add_empty_note(harness):
#     # test note body
#     note_body = ""
#
#     remove_note_if_exists(note_body, harness)
#
#     open_notes_app(harness)
#
#     # adds new note by ui
#     add_note_ui(note_body, harness)
#
#     # gets added note
#     result = get_note(note_body, harness)
#     empty_notes = []
#
#     for note in result["body"]:
#         if note["messageBody"] == note_body:
#             empty_notes.append(note)
#
#     back_to_home(harness)
#     assert (len(empty_notes) == 1)
#     assert (empty_notes[0]["messageBody"] == note_body)
#
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_add_4k_note(harness):
#     # test note body
#     note_body = generate_random_string(4000)
#
#     remove_note_if_exists(note_body, harness)
#
#     open_notes_app(harness)
#
#     # adds new note by ui
#     add_note(note_body, harness)
#
#     # gets added note
#     result = get_note(note_body, harness)
#
#     back_to_home(harness)
#     assert (len(result["body"]) == 1)
#     assert (result["body"][0]["messageBody"] == note_body)
#
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_add_too_big_note(harness):
#     # test note body
#     note_body = generate_random_string(4001)
#     remove_note_if_exists(note_body, harness)
#     open_notes_app(harness)
#
#     # adds new note
#     add_note(note_body, harness)
#
#     # gets added note
#     result = get_note(note_body, harness)
#
#     back_to_home(harness)
#     assert (len(result["body"]) == 1)
#     assert (result["body"][0]["messageBody"] == note_body)
#
#
# # @pytest.mark.rt1051
# # @pytest.mark.usefixtures("phone_unlocked")
# # def test_add_too_big_note_by_ui(harness):
# #     # test note body
# #     base_note_body = generate_random_string(3999)
# #     append_note_body = generate_random_string(2)
# #
# #     remove_all_notes(harness)
# #
# #     open_notes_app(harness)
# #     add_note(base_note_body, harness)
# #
# #     append_note_ui(append_note_body, harness)
# #
# #     # gets added note
# #     result = get_note(base_note_body, harness)
# #
# #     print(len(result["body"][0]["messageBody"]))
# #
# #     assert (len(result["body"]) == 0)
# #     assert (result["body"][0]["messageBody"] != base_note_body)
# #
# #     back_to_home(harness)
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_get_notes(harness):
#     notes_count = 20
#     remove_all_notes(harness)
#
#     # adding notes
#     for i in range(3):
#         add_note("", harness)
#     for i in range(notes_count - 6):
#         add_note(generate_random_string(random.randint(1, 3999)), harness)
#     for i in range(3):
#         add_note(generate_random_string(4000), harness)
#
#     result = get_all_notes(harness)
#
#     back_to_home(harness)
#     assert (len(result["body"]) == notes_count)
#
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_edit_note(harness):
#     base_note_body = "before append"
#     append_note_body = "new"
#
#     remove_all_notes(harness)
#     add_note(base_note_body, harness)
#     open_notes_app(harness)
#     append_note_ui(append_note_body, harness)
#
#     result = get_all_notes(harness)
#
#     back_to_home(harness)
#     assert (len(result["body"]) == 1)
#     assert (result["body"][0]["messageBody"] == (base_note_body + append_note_body))
#
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_edit_4k_note_remove_chars(harness):
#     base_note_body_length = 4000
#     num_of_chars_removed = 10
#
#     remove_all_notes(harness)
#     add_note(generate_random_string(base_note_body_length), harness)
#     open_notes_app(harness)
#     remove_characters_in_note(num_of_chars_removed, harness)
#
#     result = get_all_notes(harness)
#
#     back_to_home(harness)
#     assert (len(result["body"]) == 1)
#     assert (len(result["body"][0]["messageBody"]) == (base_note_body_length - num_of_chars_removed))
#
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_edit_4k_note_add_chars(harness):
#     base_note_body_length = 4000
#     num_of_chars_added = 5
#     num_of_chars_removed = 1
#     last_char_before_edit = "0"
#     last_char_after_edit = "1"
#
#     # base note body, filled to 4k chars by 0 to be 100% certain it gets modified
#     base_note_body = generate_random_string(base_note_body_length - 1) + last_char_before_edit
#     append_note_body = last_char_after_edit + generate_random_string(num_of_chars_added - 1)
#
#     remove_all_notes(harness)
#     add_note(base_note_body, harness)
#     open_notes_app(harness)
#
#     # remove chars before appending to check if note got modified
#     remove_characters_in_note(num_of_chars_removed, harness)
#     append_note_ui(append_note_body, harness)
#
#     result = get_all_notes(harness)
#
#     back_to_home(harness)
#     assert (len(result["body"]) == 1)
#     assert (len(result["body"][0]["messageBody"]) == base_note_body_length)
#     assert (result["body"][0]["messageBody"][base_note_body_length - num_of_chars_removed] == last_char_after_edit)
#
#
# @pytest.mark.rt1051
# @pytest.mark.usefixtures("phone_unlocked")
# def test_edit_remove_all_chars(harness):
#     base_note_body_length = 5
#
#     remove_all_notes(harness)
#     add_note(generate_random_string(base_note_body_length), harness)
#     open_notes_app(harness)
#
#     # remove chars before appending to check if note got modified
#     remove_characters_in_note(base_note_body_length, harness)
#
#     result = get_all_notes(harness)
#
#     back_to_home(harness)
#     assert (len(result["body"]) == 1)
#     assert (len(result["body"][0]["messageBody"]) == 0)


@pytest.mark.rt1051
@pytest.mark.usefixtures("phone_unlocked")
def test_edit_paste(harness):
    base_note_body_length = 5
    base_note_body = generate_random_string(base_note_body_length)
    append_note_body = generate_random_string(base_note_body_length)

    remove_all_notes(harness)
    add_note(base_note_body, harness)

    set_clipboard_cache(append_note_body, harness)
    open_notes_app(harness)

    paste_clipboard(harness)

    result = get_all_notes(harness)

    back_to_home(harness)
    assert (len(result["body"]) == 1)
    assert (result["body"][0]["messageBody"] == base_note_body + append_note_body)
