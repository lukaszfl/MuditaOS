ETAP 1: 
- [ ] zbuduj master i:
    - podłącz logi
    - podłącz analizator logiczny
- [ ] puść i zostaw niech chodzi `python -m pytest ./pytest/test_send_message.py::test_send_2k_message --timeout=10  --phone_number=730591171 --sms_text="sms text" -s`
    - powinno chodzić jakoś 3h
    - wysłać ~1000 wiadomości
    - skończy sie na errorze z timeout na komendzie at


ETAP 2:
- [ ] włącz logi na analizatorze logicznym
- [ ] puść `python -m pytest ./pytest/test_harness.py --timeout=10`
    - powinien wyjść error z komendy AT
    - powinien wyjść timeout
- [ ] zobacz RX - czy dane sie wysłaly 
- [ ] zobacz TX - czy dane doszły

