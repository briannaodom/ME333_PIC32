#!/usr/bin/python
import time
import serial
import matplotlib.pyplot as plt
#from trajectory import step_trajectory, cubic_trajectory

L = 0
t_vals = []
ref_vals = []

def help():
    print "a: Read current sensor (ADC counts)    b: Read current sensor (mA)"
    print "c: Read encoder (counts)               d: Read encoder (deg)"
    print "e: Reset encoder                       f: Set PWM (-100 to 100)"
    print "g: Set current gains                   h: Get current gains"
    print "i: Set position gains                  j: Get position gains"
    print "k: Test current control                l: Go to angle (deg)"
    print "m: Load step trajectory                n: Load cubic trajectory"
    print "o: Execute trajectory                  p: Unpower the motor"
    print "q: Quit client                         r: Get mode"
    print


if __name__ == '__main__':
    with serial.Serial('/dev/ttyUSB0', 230400, rtscts=1) as ser:
        print "Serial port communication established!"
        print
        print "MENU OPTIONS:"
        help()

        while(1):
            # grab user input from terminal:
            user_input = raw_input('please make a selection: ')
# A #
            # begin giant emulated switch statement, Python style
            if user_input == 'a': # read current sensor (ADC counts)
                ser.write(str(user_input + '\n').encode())
                response = int(ser.readline())
                print "ADC value =", response, "counts\n"
# B #
            elif user_input == 'b': # read current sensor (mA)
                ser.write(str(user_input + '\n').encode())
                response = float(ser.readline())
                print "ADC value =", response, "mA\n"
# C #
            elif user_input == 'c': # read encoder (counts)
                ser.write(str(user_input + '\n').encode())
                response = int(ser.readline())
                print "motor angle =", response, "counts\n"
# D #
            elif user_input == 'd': # read encoder (deg)
                ser.write(str(user_input + '\n').encode())
                response = float(ser.readline())
                print "motor angle =", response, "degrees\n"
# E #
            elif user_input == 'e': # reset encoder
                ser.write(str(user_input + '\n').encode())
                print "encoder counts reset to 32,768\n"
# F #
            elif user_input == 'f': # set PWM (-100 to 100)
                ser.write(str(user_input + '\n').encode())
                while(1):
                    try:
                        num = int(raw_input('enter a duty cycle (-100 to 100): '))
                        break
                    except ValueError:
                        print "invalid input, try again"
                ser.write((str(num) + '\n').encode())
# G #
            elif user_input == 'g': # set current gains
                ser.write(str(user_input + '\n').encode())
                nums = []
                for i, name in enumerate(["kp", "ki"]):
                    while(1):
                        try:
                            var = 'enter a value for ' + name + ': '
                            nums.append(int(raw_input(var)))
                            break
                        except ValueError:
                            print "invalid input, try again"
                    str_out = ""
                    for num in nums:
                        ser.write((str(num) + '\n').encode())
                print
# H #
            elif user_input == 'h': # get current gains
                ser.write(str(user_input + '\n').encode())
                names = ["kp", "ki"]
                for i, each_val in enumerate(names):
                    print names[i], "=", ser.readline()
                print
# I #
            elif user_input == 'i': # set position gains
                ser.write(str(user_input + '\n').encode())
                nums = []
                for i, name in enumerate(["kp", "ki", "kd"]):
                    while(1):
                        try:
                            var = 'enter a value for ' + name + ': '
                            nums.append(float(raw_input(var)))
                            break
                        except ValueError:
                            print "invalid input, try again"
                    str_out = ""
                    for num in nums:
                        ser.write((str(num) + '\n').encode())
                print
# J #
            elif user_input == 'j': # get position gains
                ser.write(str(user_input + '\n').encode())
                names = ["kp", "ki", "kd"]
                for i in enumerate(names):
                    print names[i], "=", ser.readline()
                print
# K #
            elif user_input == 'k': # test current control
                ser.write(str(user_input + '\n').encode())
                sampnum = 0 # index for number of samples read
                read_samples = 99;
                ref = []
                ADCval = []
                while read_samples > 1:
                    raw_data = ser.readline() # reading data from serial port
                    print "raw_data:", raw_data
                    data_read = raw_data.split()
                    read_samples = int(data_read[0])
                    ADCval.append(int(data_read[1]))
                    ref.append(int(data_read[2]))
                    sampnum += 1 # incrementing loop number
                t = range(0, sampnum)

                fig = plt.figure()
                ax = fig.add_subplot(111)

                plt.step(t, ref, label='Reference')
                plt.step(t, ADCval, 'r', label='ADC Value')
                plt.title("Actual vs Desired Current")
                plt.legend()
                ax.set_xlabel('Sample Number (at 100 Hz)')
                ax.set_ylabel('Brightness (ADC counts)')
                ax.set_ylim([0, 1000])
                figname = "/home/pabiney/Northwestern Classes/333 Introduction to Mechatronics/Final Project/test/figs/itest" + time.strftime("%Y%m%d") + '_' + time.strftime("%H%M%S") + ".svg"
                plt.savefig(figname, bbox_inches='tight')
                plt.show()
# L #
            elif user_input == 'l': # go to angle (deg)
                ser.write(str(user_input + '\n').encode())
                while(1):
                    try:
                        num = int(raw_input('enter the desired motor angle (in degrees): '))
                        break
                    except ValueError:
                        print "invalid input, try again"
                ser.write((str(num) + '\n').encode())
                print
# M #
            elif user_input == 'm': # load step trajectory
                ser.write(str(user_input + '\n').encode())
                L, t_vals, ref_vals = step_trajectory()
                ser.write((str(L) + '\n').encode())
                for val in ref_vals:
                    ser.write((str(val) + '\n').encode())

                print "step trajectory data sent to PIC"

                # print "received back from PIC:"
                for i in range(L):
                    response = ser.readline()
                print # note the comma after response in the line above
# N #
            elif user_input == 'n': # load cubic trajectory
                ser.write(str(user_input + '\n').encode())
                L, t_vals, ref_vals = cubic_trajectory()
                ser.write((str(L) + '\n').encode())
                for val in ref_vals:
                    ser.write((str(val) + '\n').encode())

                print "cubic trajectory data sent to PIC"

                # print "received back from PIC:"
                for i in range(L):
                    response = ser.readline()
                print # note the comma after response in the line above
# O #
            elif user_input == 'o': # execute trajectory
                ser.write(str(user_input + '\n').encode())

                print "executationtating the trajectorvector\n"
                ret = []
                for i in range(L):
                    response = ser.readline()
                    ret.append(int(response))
                plot_vals = [(x - 32768)/5 for x in ret]
                print # just giving myself some space

                fig = plt.figure()
                ax = fig.add_subplot(111)
                plt.step(t_vals, ref_vals, label='Reference')
                plt.step(t_vals, plot_vals, 'r', label='Encoder Value')
                plt.title("Actual vs Desired Position")
                plt.legend()
                ax.set_xlabel('Sample Number (at 200 Hz)')
                ax.set_ylabel('Motor Angle (deg)')
                figname = "/home/pabiney/Northwestern Classes/333 Introduction to Mechatronics/Final Project/test/figs/trajectory" + time.strftime("%Y%m%d") + '_' + time.strftime("%H%M%S") + ".svg"
                plt.savefig(figname, bbox_inches='tight')
                plt.show()

# P #
            elif user_input == 'p': # unpower the motor
                ser.write(str(user_input + '\n').encode())
                print "motor powered down\n"
# Q #
            elif user_input == 'q': # quit client
                ser.write(str(user_input + '\n').encode())
                print "quitting"
                exit()
# R #
            elif user_input == 'r': # get mode
                ser.write(str(user_input + '\n').encode())
                response = ser.readline() # reading data from serial port
                print "Mode =", response
# DEFAULT #
            else: # default case, invalid selection
                ser.write(user_input.encode())
                print "INVALID INPUT, OPTIONS ARE:"
                help()
