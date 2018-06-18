#!/usr/bin/python
import time
import serial
import matplotlib.pyplot as plt
import numpy
from trajectory import step_trajectory, cubic_trajectory

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
   


if __name__ == '__main__':
    with serial.Serial('/dev/ttyUSB0', 230400, rtscts=1) as ser:
        print "Serial port communication established!"
        print
        print "MENU OPTIONS:"
        help()

        while(1):
           
            usr_input = raw_input('Letter choice: ')

            if usr_input == 'a': # get ADC counts
                ser.write(str(usr_input + '\n').encode())
                output = int(ser.readline())
                print "ADC val: ", output, "counts\n"

            elif usr_input == 'b': # get current in mA
                ser.write(str(usr_input + '\n').encode())
                output = float(ser.readline())
                print "ADC val: ", output, "mA\n"

            elif usr_input == 'c': #motor angle counts
                ser.write(str(usr_input + '\n').encode())
                output = int(ser.readline())
                print "The motor angle is", output, "counts\n"

            elif usr_input == 'd': #motor angle degrees
                ser.write(str(usr_input + '\n').encode())
                output = float(ser.readline())
                print "The motor angle is", output, "degrees\n"

            elif usr_input == 'e': # encoder reset
                ser.write(str(usr_input + '\n').encode())
                print "encoder counts reset to 32,768\n"

            elif usr_input == 'f': # set PWM (-100 to 100)
                ser.write(str(usr_input + '\n').encode())
                while(1):
                    try:
                        num = int(raw_input('Choose a duty cycle (-100 to 100): '))
                        break
                    except ValueError:
                        print "invalid input, try again"
                ser.write((str(num) + '\n').encode())

            elif usr_input == 'g': # set current gains
                ser.write(str(usr_input + '\n').encode())
                nums = []
                for i, name in enumerate(["Kp", "Ki"]):
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

            elif usr_input == 'h': # get current gains
                ser.write(str(usr_input + '\n').encode())
                names = ["Kp", "Ki"]
                for j, each_val in enumerate(names):
                    print names[j], "=", ser.readline()
                print

            elif usr_input == 'i': # set pos gains
                ser.write(str(usr_input + '\n').encode())
                nums = []
                for i, name in enumerate(["Kp", "Ki", "Kd"]):
                    while(1):
                        try:
                            var = 'Enter a val for ' + name + ': '
                            nums.append(float(raw_input(var)))
                            break
                        except ValueError:
                            print "Invalid Input. Try Again!"
                    str_out = ""
                for num in nums:
                    ser.write((str(num) + '\n').encode())
                print

            elif usr_input == 'j': # get position gains
                ser.write(str(usr_input + '\n').encode())
                names = ["Kp", "Ki", "Kd"]
                for j, each_val in enumerate(names):
                    print names[j], "=", ser.readline()
                print
                
            elif usr_input == 'k': # test current control
                ser.write(str(usr_input + '\n').encode())
                sampnum = 0 
                read_samples = 99;
                ref = []
                ADCval = []
                while read_samples > 1:
                    raw_data = ser.readline() 
                    print "raw_data:", raw_data
                    data_read = raw_data.split()
                    read_samples = int(data_read[0])
                    ref.append(int(data_read[1]))
                    ADCval.append(int(data_read[2]))
                    sampnum += 1 
                t = range(0, sampnum)

                fig = plt.figure()
                ax = fig.add_subplot(111)

                plt.step(t, ref, label='Reference')
                plt.step(t, ADCval, 'r', label='ADC Value')
                plt.title("Actual Current vs Desired Current")
                plt.legend()
                ax.set_xlabel('Sample Number (at 100 Hz)')
                ax.set_ylabel('Brightness (ADC counts)')
                ax.set_ylim([-1000, 1000])
                figname = "/home/ms-robot/Downloads/" + time.strftime("%Y%m%d") + '_' + time.strftime("%H%M%S") + ".svg"
                plt.savefig(figname, bbox_inches='tight')
                plt.show()
               

            elif usr_input == 'l': # go to angle in degs
                ser.write(str(usr_input + '\n').encode())
                while(1):
                    try:
                        val = int(raw_input('Enter the desired motor angle (in degrees): '))
                        break
                    except ValueError:
                        print "invalid input, try again"
                ser.write((str(val) + '\n').encode())
                print

            elif usr_input == 'm': # load step trajectory
                ser.write(str(usr_input + '\n').encode())
                L, t_vals, ref_vals = step_trajectory()
                ser.write((str(L) + '\n').encode())
                for val in ref_vals:
                    ser.write((str(val) + '\n').encode())

                print "step trajectory data sent to PIC"

             
                for j in range(L):
                    response = ser.readline()
                print 

            elif usr_input == 'n': # load cubic trajectory
                ser.write(str(usr_input + '\n').encode())
                L, t_vals, ref_vals = cubic_trajectory()
                ser.write((str(L) + '\n').encode())
                for val in ref_vals:
                    ser.write((str(val) + '\n').encode())

                print "cubic trajectory data sent to PIC"

              
                for j in range(L):
                    response = ser.readline()
                print 

            elif usr_input == 'o': # execute trajectory
                ser.write(str(usr_input + '\n').encode())

                print "Executing Trajectory \n"
                ret = []
                for j in range(L):
                    response = ser.readline()
                    ret.append(int(response))
                plot_vals = [(x - 32768)/5 for x in ret]
                print 

                fig = plt.figure()
                ax = fig.add_subplot(111)
                plt.step(t_vals, ref_vals, label='Reference')
                plt.step(t_vals, plot_vals, 'r', label='Encoder Value')
                plt.title("Actual vs Desired Pos")
                plt.legend()
                ax.set_xlabel('Sample Number (at 200 Hz)')
                ax.set_ylabel('Motor Angle (deg)')
                figname = "/home/ms-robot/Downloads" + time.strftime("%Y%m%d") + '_' + time.strftime("%H%M%S") + ".svg"
                plt.savefig(figname, bbox_inches='tight')
                plt.show()

            elif usr_input == 'p': # unpower the motor
                ser.write(str(usr_input + '\n').encode())
                print "Motor is powering down.\n"

            elif usr_input == 'q': # quit commands
                ser.write(str(usr_input + '\n').encode())
                print "Program quitting."
                exit()

            elif usr_input == 'r': # get current mode
                ser.write(str(usr_input + '\n').encode())
                output = ser.readline() # reading data from serial port
                print "Mode =", output

            else: # default, invalid inout
                ser.write(usr_input.encode())
                print "Invalid input. Your options are: \n"
                help()
