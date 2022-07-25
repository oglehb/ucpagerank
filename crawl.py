from collections import deque
import requests
from bs4 import BeautifulSoup
from urllib.parse import urlparse
import signal
import os
import sys

VERTEXSET_FILENAME = 'vertices.txt'
EDGESET_FILENAME = 'edges.csv'
BFS_QUEUE_FILENAME = 'queue.txt'

def main():

    # Process command line arguments, if any
    if len(sys.argv) > 1:
        cli_option = sys.argv[1]
        if cli_option == '--restart' or cli_option == '-r':
            init_files()
    
    # Initialize files upon first run
    if not os.path.exists(VERTEXSET_FILENAME):
        init_files()

    # Read vertices, edges, and queue from file
    V, index = read_vertices_from_file()
    E = set(read_edges_from_file())
    q = read_queue_from_file()

    # Push the last vertex onto the queue if empty
    if len(q) == 0:
        q.append(V[-1])

    # Edge additions to write to file upon termination
    new_edges = deque()

    try:
        crawl(V, E, q, index, new_edges)
    except Error as err:
        print('Unexpected error while crawling:')
        print(err)
    finally:
        # Update files
        update_vertices_file(V)
        update_edges_file(new_edges)
        update_queue_file(q)

def crawl(V, E, q, index, new_edges):
    """
    Crawl by performing a bread-first traversal using hyperlinks.
    Update the vertex set, edge set, and BFS queue files when
    the BFS loop either ends naturally or is terminated manually 
    by the user.
    The user may choose to terminate the crawling process manually
    by entering the key combination CTRL+C at the command line.
    """

    # Introduce a flag to indicate when the user
    # wants to stop the crawling process manually.
    user_terminated = False
    def interrupt_signal_handler(sig, frame):
        nonlocal user_terminated
        print('You pressed Ctrl+C! Shutting down crawler...')
        user_terminated = True
    signal.signal(signal.SIGINT, interrupt_signal_handler)

    # For quick access (faster membership checks)
    qset = set(q)

    print(f'Resuming BFS. (Press CTRL+C at any time to end early)')
    
    # Bread-first traversal on hyperlinks of UC's website
    while len(q) > 0 and not user_terminated:

        url = q[0]
        src = index[url]

        print(f'{len(q)} vertices queued. Visiting {url}')
        
        outlinks = get_outlinks(url)
        for outlink in outlinks:

            # Update vertex set if necessary
            if outlink not in index:
                index[outlink] = len(V)
                V.append(outlink)

                # Push to BFS queue if not done already
                if outlink not in qset:
                    q.append(outlink)
                    qset.add(outlink)

            # Represent the edge as a comma-separated
            # pair of vertices, as in the edges.csv file
            dest = index[outlink]
            edge = str(src) + ',' + str(dest)

            # Update edge set if necessary
            if edge not in E:
                E.add(edge)
                new_edges.append(edge)  # indicate we need
                                        # to write to file

        q.popleft()
        qset.discard(url)

    print(f'BFS terminated with {len(q)} vertices left in the queue.')

def read_vertices_from_file():
    line_count = sum(1 for line in open(VERTEXSET_FILENAME, 'r'))
    vertices = ['']*line_count  # initialize list
    index = {}
    with open(VERTEXSET_FILENAME, 'r') as f:
        line_num = 0
        for line in f:
            url = line.strip()  # remove beginning/trailing whitespace
            vertices[line_num] = url
            index[url] = line_num
            line_num += 1
    return vertices, index

def read_edges_from_file():
    line_count = sum(1 for line in open(EDGESET_FILENAME, 'r'))
    edges = ['']*line_count  # initialize list
    with open(EDGESET_FILENAME, 'r') as f:
        line_num = 0
        for line in f:
            edges[line_num] = line.strip()
    return edges

def read_queue_from_file():
    q = deque()
    with open(BFS_QUEUE_FILENAME, 'r') as f:
        line = f.readline()
        while line:
            q.append(line.strip())
            line = f.readline()
    return q

def update_vertices_file(vertices):
    line_count = sum(1 for line in open(VERTEXSET_FILENAME, 'r'))
    with open(VERTEXSET_FILENAME, 'a') as f:
        for i in range(line_count, len(vertices)):
            f.write(vertices[i] + '\n')

def update_edges_file(new_edges):
    with open(EDGESET_FILENAME, 'a') as f:
        for edge in new_edges:
            f.write(edge + '\n')

def update_queue_file(q):
    """Write BFS queue to file and destroy the queue."""
    with open(BFS_QUEUE_FILENAME, 'w') as f:
        while (len(q) > 0):
            f.write(str(q[0]) + '\n')
            q.popleft()

def get_outlinks(url):

    outlinks = set()

    # Make a GET request to retrieve the HTML file at the `url`
    response = requests.get(url)

    # Parse the response into a python object
    soup = BeautifulSoup(response.content, 'html5lib')

    # Search the response HTML for links (links have the 'a' tag)
    page_links = soup.find_all('a')

    # Filter out invalid links or those which link outside of www.uc.edu
    for link in page_links:
        try:
            if link.has_attr('href'):
                parsed_url = urlparse(link['href'])
                if parsed_url.hostname == 'www.uc.edu' and parsed_url.scheme == 'https':
                    url = '{o.scheme}://{o.hostname}{o.path}'.format(o=parsed_url)
                    url = url.rstrip('/')
                    outlinks.add(url)
        except:
            print(f'Error processing link: {link}')
    
    return outlinks

def init_files():
    """
    Reinitialize cached files.
    """

    filenames = [
        VERTEXSET_FILENAME,
        EDGESET_FILENAME,
        BFS_QUEUE_FILENAME
    ]
    
    for filename in filenames:
        # Delete if exists
        if os.path.exists(filename):
            os.remove(filename)
        # Create empty file
        with open(filename, 'w') as f:
            pass

    # Initialize the vertex set and queue with the root 
    # of our breadth-first traversal, UC's homepage.
    root = 'https://www.uc.edu'
    with open(VERTEXSET_FILENAME, 'w') as f:
        f.write(root + '\n')
    with open(BFS_QUEUE_FILENAME, 'w') as f:
        f.write(root + '\n')

if __name__ == '__main__':
    main()