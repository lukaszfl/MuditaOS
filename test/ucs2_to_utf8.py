from sys import argv, stderr

if len(argv) > 1:
    # assume
    ucs_in = argv[1]
else:
    print("podaj tekst w formacie UCS2 jako argument", file=stderr)
    exit(1)


def ucs_gsm(ucs):
    to_ret = ''
    char_len = 4
    
    partial_char = len(ucs) % char_len
    if partial_char:
        print(f'!! warning: given text has partial character trailing: >>{ucs[-partial_char:]}<<(UCS2)', file=stderr)
            
    for i in range(0, len(ucs), char_len ):
        ucs_char = ucs[i:i+char_len]
        ucs_char.replace('0', '')
        ucs_char_int = int(ucs_char, 16)
        to_ret += chr(ucs_char_int)
    
    to_ret = '>> The text comes after a new line:' + \
        '\n' + \
        to_ret + \
        '\n' + '>> The text ends in the previous line'
    return to_ret

def main():
    print(ucs_gsm(ucs_in))

        
if __name__ == '__main__':
    main()
