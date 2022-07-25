"""
Simple script to convert all data to
a single JSON object for web consumption.
"""

import json

VERTEXSET_FILENAME = 'vertices.txt'
EDGESET_FILENAME = 'edges.csv'
PAGERANK_FILENAME = 'pagerank.txt'
JSON_OUTPUT_FILENAME = 'pagerank.json'

def main():

    # Read vertices
    V = []
    with open(VERTEXSET_FILENAME, 'r') as f:
        line = f.readline()
        while line:
            V.append(line.strip())
            line = f.readline()
    print(f'Read {len(V)} vertices.')

    # Tests
    # Vset = set(V)
    # print('https://www.uc.edu' in Vset)
    # print('https://www.uc.edu/' in Vset)
    # ct = 0
    # for v in V:
    #     if v[-1] == '/':
    #         print(f'{v}  {v.rstrip("/") in Vset}')
    #         ct += 1
    # print(f'ct: {ct}')
    # ct = 0
    # for v in V:
    #     if v[:5] == 'http:':
    #         print(f'{v}  {(v[:4] + "s" + v[4:]) in Vset}')
    #         ct += 1
    # print(f'ct: {ct}')
    # return
    
    # Read edges
    E = []
    with open(EDGESET_FILENAME, 'r') as f:
        line = f.readline()
        while line:
            E.append([int(x) for x in line.strip().split(',')])
            line = f.readline()
    print(f'Read {len(E)} edges.')
        
    # Read PageRank vector
    pi = []
    with open(PAGERANK_FILENAME, 'r') as f:
        line = f.readline()
        while line:
            pi.append(float(line))
            line = f.readline()
    print(f'Read {len(pi)}-element PageRank vector.')
    
    # Form output as a Python object
    output = {}
    output['nodes'] = V
    output['edges'] = E
    output['pagerank'] = pi

    # Serialize Python object to JSON
    json_output = json.dumps(output, indent=4)

    # Write JSON to file
    print(f'Writing JSON to file "{JSON_OUTPUT_FILENAME}"...', end='')
    with open(JSON_OUTPUT_FILENAME, 'w') as f:
        f.write(json_output)
    print(' done.')

if __name__ == '__main__':
    main()