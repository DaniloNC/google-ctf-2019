#!/usr/bin/python2
import z3

def solve_z3(target,steps=64):
    s = z3.Solver()
    bits = [((target >> x) & 1) for x in range(steps-1,-1,-1)]

    z3_target_bits = [z3.BoolVal(bool(bit)) for bit in bits] 
    z3_source_bits = [z3.Bool('b-%02d' % x) for x in xrange(steps) ]

    for k,v in enumerate(z3_target_bits):
        s.add(v == z3.Not(z3.Or(
            z3.And(z3_source_bits[(k-1)%steps],z3_source_bits[k], z3_source_bits[(k+1)%steps]),
            z3.And(z3.Not(z3_source_bits[(k-1)%steps]),z3.Not(z3_source_bits[k]), z3.Not(z3_source_bits[(k+1)%steps]))
            ))
          )

    possible_solutions = [] 
    while s.check() == z3.sat:
        m = s.model()
        temp_solution = [ z3.is_true(m[x]) for x in z3_source_bits]
        s.add(z3.Not(z3.And([ x == z3.is_true(m[x]) for x in z3_source_bits ])))

        bits_s = map(int,temp_solution)
        n = 0
        for bit in bits_s:
            n = (n << 1) | bit

        # possible_solutions.append(n)
        print(hex(n))

    return possible_solutions
   
def rule126(ini_state, steps=64):
    bits = [((ini_state >> x) & 1) for x in range(steps-1,-1,-1)]
    bits_copy = [0] * steps

    for x in range(steps):
        if x > 0 and x < steps-1:
            current_set = bits[x-1:x+2]
        elif x == 0:
            current_set = bits[steps-2:] + bits[x:x+2]
        elif x == steps-1:
            current_set  = bits[x-1:x+1] + bits[:1]
        
        n = (current_set[0] << 2) + (current_set[1] << 1) + (current_set[2])
        if n == 0 or n == 7:
            bits_copy[x] = 0
        else:
            bits_copy[x] = 1

        out = 0
        for bit in bits_copy:
            out = (out << 1) | bit

    return out

def main():

    # Example of working 
    # v = 0xdeadbeef
    # a = rule126(v,32)
    # b = solve_z3(a,32)
    # print (v in b)

    v = 0x66de3c1bf87fdfcf
    solutions = solve_z3(v,64)
    print map(hex,solutions)
    assert all(map(lambda x: rule126(x, 64) == v, solutions))

if __name__ == "__main__":
    main()