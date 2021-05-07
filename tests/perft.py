'''

This python script will run through a list of FEN's with perft count given for each depth.

'''
import argparse
import subprocess
import sys

# Class for holding a position and its perft results
class PerftPosition:
    def __init__(self, _fen = "", _depths = 0, _ncs = []):
        self.fen = _fen
        self.depths = 0
        self.nodeCounts = []

        for i in range(_depths):
            self.depths += 1
            self.nodeCounts.append(_ncs[i])
        

def main():
    print("-"*100)
    print("[*] Starting perft test")
    print("-"*100)

    # Step 1. Parse the inputs
    parser = argparse.ArgumentParser(description='Test the move generator of Loki')

    parser.add_argument("exe", type=str, help='The path to the Loki executable. Please use an ABSOLUTE path (eg. "C:\\Users\\...\\Loki.exe")')
    parser.add_argument("epd", type=str, help='The path to the EPD file. Please use an ABSOLUTE path (eg. "C:\\Users\\...\\perft.epd")')
    parser.add_argument("--depth", type=int, help='The maximum perft-depth for all positions. Default is the highest possible.', default=100)

    args = parser.parse_args()

    # Step 2. Run the engine
    prc = subprocess.Popen(args.exe, 
                    stderr=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    stdin=subprocess.PIPE,
                    universal_newlines=True)

    # Step 3. Read the epd file
    Positions = []
    EPDS = []

    with open(args.epd, "r") as perft_file:
        Lines = perft_file.readlines()

        # Step 3A. Read through the EPD line-by-line
        for line in Lines:
            EPDS.append(line)

            elements = line.split(";")

            FEN = elements[0]
            nodesByDepth = []

            for i in elements[1:]:
                nodesByDepth.append(int(i[2:]))

            # Step 3A.1. Find the smallest depth that has a node-count for the position
            smallest_depth = int(elements[1].split(" ")[0][-1])

            # Step 3A.2. Append the FEN and depths
            PositionData = (FEN, len(nodesByDepth), nodesByDepth, smallest_depth)
            Positions.append(PositionData)



    # Step 4. Run through the positions and call perft for each depth
    n = 0

    # Step 4A. When starting up, Loki outputs some info on the transposition table,
    found = prc.stdout.readline().strip()
    prc.stdout.flush()
    found = prc.stdout.readline().strip()
    prc.stdout.flush()

    for i in Positions:
        if (i[3] >= args.depth):
            continue

        Fen = i[0]
        n += 1
        print("[*] Now perft-testing position nr. " + str(n))
        
        # Step 4A. Send the position to the engine.
        prc.stdin.write('position fen %s \n' % (Fen))


        for d in range(0, i[1]):
            if d == 0:
                continue
            if d >= args.depth or d < i[3]:
                break

            prc.stdin.write('perft depth %d \n' % (d + 1))
            prc.stdin.flush()

            #found = prc.stdout.readline().strip()
            #print(found)
            found = []

            while True:
                found.append(prc.stdout.readline().strip())
                prc.stdout.flush()
                if "Nodes visited" in found[-1]:
                    break
            
            nodecount = int(found[-1].split(": ")[-1])

            if nodecount == i[2][d]:
                print("Depth " + str(d) + ": PASSED")
            else:
                print("Depth " + str(d) + ": FAILED ----> count: " + str(nodecount))

 
    # Step 5. Close Loki
    prc.stdin.write('quit')
    prc.stdin.flush()




if __name__ == "__main__":
    main()