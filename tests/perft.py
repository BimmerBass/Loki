'''

This python script will run through a list of FEN's with perft count given for each depth.

'''
import argparse
import subprocess

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
                    stderr=subprocess.STDOUT,
                    stdout=subprocess.PIPE,
                    stdin=subprocess.PIPE,
                    universal_newlines=True)

    # Step 3. Read the epd file
    Positions = []

    with open(args.epd, "r") as perft_file:
        Lines = perft_file.readlines()

        # Step 3A. Read through the EPD line-by-line
        for line in Lines:
            elements = line.split(";")

            FEN = elements[0]
            nodesByDepth = []

            for i in elements[1:]:
                nodesByDepth.append(int(i[2:]))
            
            # Step 3A.1. Append the FEN and depths
            Positions.append(PerftPosition(FEN, len(nodesByDepth), nodesByDepth))



    # Step 4. Run through the positions and call perft for each depth
    n = 0

    for i in Positions:
        Fen = i.fen
        n += 1
        print("[*] Now perft-testing position nr. " + str(n))
        
        # Step 4A. Send the position to the engine.
        prc.stdin.write('position fen %s \n' % (Fen))

        for d in range(i.depths):
            if (d >= args.depth):
                break

            prc.stdin.write('perft depth %d \n' % (d + 1))
            prc.stdin.flush()

            found = int(prc.stdout.readline().strip())

            if found == i.nodeCounts[d]:
                print("Depth " + str(d + 1) + ": PASSED")
            else:
                print("Depth " + str(d + 1) + ": FAILED ----> count: " + str(found))
 
    # Step 5. Close Loki
    prc.stdin.write('quit')
    prc.stdin.flush()




if __name__ == "__main__":
    main()