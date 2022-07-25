# Contents

Within this repo is a Python script which indexes every [www.uc.edu]() webpage reachable from the homepage as well as a C++ program which builds the hyperlink graph and computes the PageRank vector. Both the script and the program ultimately write their data to files, **which have been uploaded to this repo in advance**, so fulfilling this process yourself is entirely unnecessary to enjoy the PageRank visualization—just open the HTML file, which itself uses the aforementioned uploaded data files.

# (Optional) Building the hyperlink matrix and running PageRank yourself

If you are so inclined to crawl every UC-domain webpage reachable from the homepage, and then compute the PageRank vector, you may follow the below instructions.

## Running the Crawler

To start indexing UC's website, use the command:

    python crawl.py --restart

The `--restart` CLI option indicates to the script to delete cached data and start crawling from scratch.
The crawler uses a breadth-first traversal to walk the [www.uc.edu]() domain.
Upon first run, the crawler will start its traversal with the root vertex [www.uc.edu](), UC's homepage.
On my machine, it took about 2 hours total to crawl the ~6000 vertices (the script does not make requests in parallel).
The user may wish to terminate manually, which is supported using CTRL+C. The script will save the current
BFS queue to file and resume where it left off next time it is run. Next time, simply run `python crawl.py` to pick up where you left off.
The script `crawl.py` will create the cached files:
- `vertices.txt`, a list of vertices, one per line. Each vertex is uniquely identified by its line number in this file.
- `edges.csv`, a list of vertex ID pairs.
- `queue.txt`, a list of vertices in the BFS queue, if the crawler was terminated early by the user.

## Computing the PageRank vector

I personally opted to use C++ for this part as I may wish to use this code somewhere meaningful in the future. Thus, you must have a C++ compiler (or IDE) on your machine. Because it's a single file, the simplest way is to compile via CLI. Linux users may want to use the `g++` compiler for this. Personally, since I am running Windows, I used `g++` in an instance of bash running on the Linux kernel that they now ship with Windows (setup instructions [here](https://docs.microsoft.com/en-us/windows/wsl/install)).

Compiling and running using `g++` in bash:

    g++ PageRank.cpp -o PageRank && ./PageRank

The program writes the PageRank vector to a file named `pagerank.txt`, where each line is a `double` value such that the value on line `i` is the PageRank for the vertex with ID `i` whose corresponding URL is on line `i` of the file `vertices.txt`.