def challenge():
    from ctypes import cdll, c_char_p
    from hashlib import md5
    import json

    id_input = input('User id? ')
    pw_input = input('User pw? ')
    pw_input = md5(pw_input.encode()).hexdigest()

    dll = cdll.LoadLibrary('./mysql_test.so')

    dll.DBSampleQuery.restype = c_char_p
    res = json.loads(dll.DBSampleQuery(c_char_p(id_input.encode()), 
                    c_char_p(pw_input.encode())).decode())

    if list(res.keys()).count('error_code') != 0:
        if res['error_code'] == 1:  # ID가 틀렸다.
            print("Wrong ID")
        else:  # 알 수 없는 오류
            print("Unexpected Error")
            print(res['error_code'])
    elif len(list(res.keys())) == 1: # ID는 맞지만 PW가 틀렸다.
        print("Wrong Password")
    else:
        print(f"UID : {list(res.keys())[0]}\nUSERNAME : {res[list(res.keys())[0]]}")

if __name__ == "__main__":
    while True:
        challenge()
        print()
        print()