import math

def rawToRealDataV(data):
    return (data >> 3) * 0.008

def rawToRealDataVShunt(data):
    return (data >> 3) * 0.00004

def MathtTemp(V, vShunt):
    A = 0.8781625571e-3
    B = 2.531972392e-4
    C = 1.840753501e-7

    R_termistor1 = V / (vShunt / 270)
    print("R", R_termistor1)
    temp = 1 / ( A + B * math.log(R_termistor1) + C * math.log(R_termistor1) * math.log(R_termistor1) * math.log(R_termistor1))
    temp -=273.15

    print(temp)

def main():
    V = int(input("V: "))
    VShunt = int(input("V_Shunt: "))


    V = rawToRealDataV(V)
    VShunt = rawToRealDataVShunt(VShunt)

    print("V real:", V)
    print("VShunt real:", VShunt)


    MathtTemp(V, VShunt)


main()