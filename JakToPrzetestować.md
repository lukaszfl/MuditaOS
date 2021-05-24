# setup 5 min:

- [ ] wgrać soft na pure przez ./ruh.sh
- [ ] włączyć logowanie
- [ ] puścić test jak będzie już karta sim z odblokowanym tel: `python -m pytest ./pytest/test_send_message.py::test_send_2k_message --timeout=10  --phone_number=730591171 --sms_text="sms text" -s`

# po 3 h
- [ ] powinno sie wywalić na timeout po ~1k message
- [ ] kazda kolejna komenda będzie sie wywalać na timeout
    - [ ] sprawdźić fail przez: `python -m pytest ./pytest/test_harness.py --timeout=10`
        - RTT: `12545504 ms ERROR [ServiceCellular] ATCommon.cpp:cmdLog:34: [AT]: >AT<, timeout 1000 - please check the value with Quectel_EC25&EC21_AT_Commands_Manual_V1.3.pdf`
        - LOGI pytest `FAILED pytest/test_harness.py::test_send_AT - AssertionError: assert 'OK' in []`
    - [ ] sprawdzić na analizatorze czy telefon wysyła dane na RX i czy nie dostaje odpowiedzi na 100% jeszcze raz puszczająć tą samą komendę (test_harness.py)
- [ ] zgrać logi z modemu
