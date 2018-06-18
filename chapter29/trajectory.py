#!/usr/bin/python

def step_trajectory():
    t_raw, p_raw = getInput()
    return step_trajectory_gen(t_raw, p_raw)


def cubic_trajectory():
    t_raw, p_raw = getInput()
    return cubic_trajectory_gen(t_raw, p_raw)


def getInput():
   
    while(1):
        try:
            data = raw_input("Enter a list of times and angles (intgers only! Format: time SPACE angle SPACE <repeat>): ").split()
            data = [int(x) for x in data]
        except ValueError:
            print "invalid input, try again"
            continue

        if len(data) % 2:
            print "Incorrect Format. Each time and angle paired together must have 2 values."
            continue

        L = len(data)/2
        t_vals = []#[0] # first value must be zero <-- this is better way to do it and remove the "whatever_raw.insert(0, 0)" lines in each of the generator functions, but no time right now
        p_vals = []#[0] # first value must be zero
        for i, val in enumerate(data):
            if i % 2:
                p_vals.append(val)
            else:
                t_vals.append(val)

        last_time = 0
        time_check = False
        for time in t_vals: # make sure no negative delta times occur
            if time <= last_time:
                time_check = True
                break
            last_time = time

        if(time_check == True):
            print "Time vals must monotonically inc with a first value greater than 0. Try again."
            continue
        else:
            return t_vals, p_vals


def step_trajectory_gen(t_raw, p_raw):


    FREQ = 200 # position update frequency
    dt = 1./FREQ

    L = len(t_raw)
    t_raw.insert(0, 0) # first data point needs to be zero
    p_raw.insert(0, 0) # first data point needs to be zero

    p_out = []
    for j in range(1, L+1):
        nsteps = (t_raw[j] - t_raw[j-1]) * FREQ
        vals = [p_raw[j]] * nsteps
        p_out = p_out + vals
        # p_out.append(vals) // this should be slightly faster, but isn't working at the moment

    L = len(p_out)
    t_out = range(L)
    return L, t_out, p_out


def cubic_trajectory_gen(t_raw, p_raw):
   

    FREQ = 200 # position update frequency
    dt = 1./FREQ

    L = max(t_raw) * FREQ
    t_raw.insert(0, 0) # first data point needs to be zero
    p_raw.insert(0, 0) # first data point needs to be zero

    v_raw = [0] # initial velocity = 0
    for i in range(1, len(t_raw)-1):
        v_raw.append((p_raw[i+1] - p_raw[i-1]) / (t_raw[i+1] - t_raw[i-1]))
    v_raw.append(0) # final velocity = 0 also

    n = 0
    p_out = []
    for j in range(L):
        if j * dt >= t_raw[n+1]: # compare times and increment to the next segment if appropriate
            n += 1
        t0 = t_raw[n]
        tf = t_raw[n+1]
        p0 = p_raw[n]
        pf = p_raw[n+1]
        v0 = v_raw[n]
        vf = v_raw[n+1]

        a0 = p0
        a1 = v0
        a2 = (3*pf - 3*p0 - 2*v0*(tf - t0) - vf*(tf - t0)) / ((tf - t0)**2)
        a3 = (2*p0 + (v0 + vf)*(tf - t0) - 2*pf) / ((tf - t0)**3)

        dt_seg = j*dt - t_raw[n]
        p_out.append(a0 + a1*dt_seg + a2*dt_seg**2 + a3*dt_seg**3)

    t_out = range(len(p_out)) # just to keep consistent w/ other function
    return L, t_out, p_out



if __name__ == '__main__':
    pass
