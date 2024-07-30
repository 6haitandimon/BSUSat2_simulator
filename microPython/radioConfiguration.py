def outdivSelect(input_value):
    if 142 <= input_value < 175:
        return 24
    elif 284 <= input_value < 350:
        return 12
    elif 420 <= input_value < 525:
        return 8
    elif 850 <= input_value < 1050:
        return 4
    else:
        return 0

def validFreqXo(freqXo):
    if 25 < freqXo < 32:
        return True
    else:
        return False


def freqSetup():
    arrayDataConfig = [0x11, 0x40, 0x08, 0x00] 

    freq_xo = 26 * 10**6

    RF_freq_input = int(input("Введите желаемую частоту в Hz: "))

    outdiv = outdivSelect(RF_freq_input / (10**6))
    if outdiv != 0:
        print("outdiv для выбраной частоты:", outdiv)
    else:
        print("ERROR: No valid frequence")
        return
    print("Частота freq_xo: ", freq_xo)
    answer = int(input("Желаете изменить частоту freq_xo? (1 - Да, 0 - нет): "))
    if answer == 1:
        freq_xo = int(input("Введите желаемую частоту в Hz (25*10^6 - 32*10^6 Hz): "))
        while validFreqXo(freq_xo / 10**6) != True:
            print("частота не валидна!")
            freq_xo = int(input("Введите желаемую частоту в Hz (25*10^6 - 32*10^6 Hz): "))
    fc_inte = 55
    fc_fraq = 2**19 + 1
    # print(RF_freq_input)
    # time.sleep(1)
    while fc_inte <= 255:
        fc_inte += 1
        fc_fraq = 2**19 + 1
        while 2**19 < fc_fraq < 2*2**19:
            fc_fraq += 10
            RF_freq = int((fc_inte + (fc_fraq / 2**19)) * (2 * freq_xo) / outdiv)
            if (RF_freq_input / 100 - 1) < RF_freq / 100 < (RF_freq_input / 100 + 1):
                arrayDataConfig.append(fc_inte)
                print("значение fc_inte:", fc_inte, "HEX", f"0x{fc_inte:02X}")
                print("значение fc_fraq:", fc_fraq, "HEX:", f"0x{fc_fraq:02X}")
                # print(fc_fraq.to_bytes(3, byteorder='big'))
                fc_fraqByteValue = fc_fraq.to_bytes(3, byteorder='big')

                for item in fc_fraqByteValue:
                    arrayDataConfig.append(item)

                arrayDataConfig.append(0x07)
                arrayDataConfig.append(0xE0)
                arrayDataConfig.append(0x20)
                arrayDataConfig.append(0xFE)

                hex_values = [f"0x{item:02X}" for item in arrayDataConfig]

                result = f"[{', '.join(hex_values)}]"

                print("реальная частота дя полученных значений:", RF_freq)

                print("массив байт RF_FREQ_CONTROL_INTE_8: (from copy)", result)

                return
            
    



freqSetup()

# hex_values = [f"0x{item:02x}" for item in arrayDataConfig]

# result = f"[{', '.join(hex_values)}]"

# print(result)

