#include <bits/stdc++.h>
using namespace std;

const string VERTEX_SET_FILENAME = "vertices.txt";
const string EDGE_SET_FILENAME = "edges.csv";
const string PAGERANK_FILENAME = "pagerank.txt";
const double ALPHA = 0.85;  // alpha value used in Google matrix
const int ITERATIONS = 20;  // number of PageRank iterations (likely overkill here)

int count_lines(const string&);
void read_vertices_from_file(const string&, vector<string>&);
void read_edges_from_file(const string&, list<pair<int,int>>&);
void write_pagerank_vector_to_file(const string&, const vector<double> &pi);
void build_google_graph(int, list<pair<int,int>>&, vector<unordered_map<int,double>>&, double);
void PageRank(vector<unordered_map<int,double>>&, vector<double>&, int);

int main()
{

    // Store start time for benchmarking purposes
    auto start = std::chrono::system_clock::now();

    // Create and populate the vertex set from file
    vector<string> V;
    read_vertices_from_file(VERTEX_SET_FILENAME, V);
    cout << "Read " << V.size() << " vertices from file." << endl;

    int n = V.size();

    // Create and populate the edge set from file
    list<pair<int,int>> E;
    read_edges_from_file(EDGE_SET_FILENAME, E);
    cout << "Read " << E.size() << " edges from file." << endl;

    // Create and populate the Google graph
    vector<unordered_map<int,double>> G;
    build_google_graph(n, E, G, ALPHA);

    // Initialize the PageRank vector
    vector<double> pi(n, 1.0/n);
    PageRank(G, pi, ITERATIONS);

    write_pagerank_vector_to_file(PAGERANK_FILENAME, pi);
    // for (int i = 0; i < pi.size(); ++i)
    //     cout << pi[i] << " ";
    // cout << endl;

    // Benchmarking below
    auto end = std::chrono::system_clock::now();
 
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
 
    std::cout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s"
              << std::endl;

    return 0;
}

/*
    We build the Google hyperlink graph with two
    important facts in mind:
    1. The graph is very sparse
       (low average degree relative to vertex count),
       so we don't want to store an entire n×n matrix.
    2. We later want to simulate multiplication of a 1×n row
       by an n×n matrix with normalized (stochastic) rows.

    Because the graph is so sparse, we will be using 
    an **adjacency list** to store G.

    In the hyperlink **matrix** H, the value H_{ij}
    is 1/|P_i| if page P_i links to page P_j, else zero,
    where |P_i| is the number of outlinks in page P_i.

    If we left-multiply an n×n matrix using a 1×n row,
    we are using the **columns** of the matrix to calculate
    the respective indices of the resultant 1×n product.

    Therefore, during the vector multiplication in each
    iteration of the PageRank procedure, we need to quickly
    access all H_{ij} values for any given value of j.

    Thus, we store an adjacency list of the transpose of 
    the hyperlink matrix, as it is defined in the PageRank
    algorithm normally. Then, we make the proper adjustments
    (stochasticity adjustment and primitivity adjustment)
    to convert it into the transpose of the Google matrix.
*/
void build_google_graph(
    int n,  // vertex count
    list<pair<int,int>> &E,
    vector<unordered_map<int,double>> &G,
    double alpha
) {

    G = vector<unordered_map<int,double>>(n);

    // Count the number of outlinks
    // for each webpage.
    // Will be used for virtual
    // row-normalization.
    vector<int> outlinks(n, 0);

    int src, dest;

    // Read edge-by-edge and update G, destroying
    // the edge list in the process.
    // We don't need it for anything besides
    // building the hyperlink matrix, so it's
    // best to free up the memory.
    while (E.size() > 0)
    {
        // Set src and dest using the
        // top pair in the edge list
        tie(src, dest) = E.front();

        // Pop the edge from the list
        E.pop_front();

        // Update the inlinks of `dest`
        G[dest][src] = -1.0;  // placeholder until
                              // |P_{src}| is calculated

        // Update |P_{src}|, the outlink count of `src`
        outlinks[src]++;
    }

    // Ensure stochasticity in non-dangling rows
    for (dest = 0; dest < n; ++dest)
    {
        auto it = G[dest].begin();
        for (it; it != G[dest].end(); ++it)
        {
            src = it->first;
            G[dest][src] = 1.0/outlinks[src];
        }
    }

    // **Stochasticity adjustment**
    // Ensure stochasticity in dangling node rows
    for (int src = 0; src < n; ++src)
    {
        if (outlinks[src] == 0)  // page P_{src} is a dangling node
        {
            for (int dest = 0; dest < n; ++dest)
            {
                G[dest][src] = 1.0/n;
            }
        }
    }

    // Value of any cell in the matrix 
    // (1-alpha)(1/n)ee^T
    double jump_transition_prob = (1.0-alpha)*(1.0/n);

    // **Primitivity adjustment**
    for (int dest = 0; dest < n; ++dest)
    {
        auto it = G[dest].begin();
        for (it; it != G[dest].end(); ++it)
        {
            src = it->first;
            double G_ij = it->second;
            // it->second = alpha*(it->second) + jump_transition_prob;
            G[dest][src] = alpha*(G_ij) + jump_transition_prob;
        }
    }

}

/*
    PageRank procedure using an adjacency list of the
    transpose of the Google matrix.
*/
void PageRank(
    vector<unordered_map<int,double>> &G,
    vector<double> &pi,
    int iterations
) {

    int n = G.size();

    for (int k = 0; k < iterations; ++k)
    {

        vector<double> next_pi(n);

        for (int j = 0; j < n; ++j)
        {
            double pi_j = 0.0;
            for (auto const &key_value_pair : G[j])
            {
                int i = key_value_pair.first;
                double G_ij = key_value_pair.second;
                pi_j += (pi[i]*G_ij);
            }
            next_pi[j] = pi_j;
        }

        pi = next_pi;

        // Row normalize pi with respect to the L1 norm
        double L1_pi = 0.0;
        for (int i = 0; i < n; ++i)
            L1_pi += pi[i];
        for (int i = 0; i < n; ++i)
            pi[i] /= L1_pi;

    }

}


//////////// Boring helper functions and IO below ////////////


void read_vertices_from_file(
    const string &filename,
    vector<string>& V
) {

    string line;
    ifstream infile(filename);
    if (infile.is_open())
    {
        while (getline(infile, line))
        {
            V.push_back(line);  // Benchmarking this specific scenario indicates
                                // `push_back` is faster than using a pre-emptive 
                                // line count in order to initialize the vector
                                // with the final size.
        }
        infile.close();
    }
    else throw runtime_error("Unable to open vertex set file.");

}

void read_edges_from_file(const string &filename, list<pair<int,int>> &E)
{
    string line;
    ifstream infile(filename);
    if (infile.is_open())
    {
        int u, v;
        while (getline(infile, line))
        {
            // Each line is a pair of integers separated by a comma.
            // Parse the line into a pair of integers and push
            // that pair to the edge list.
            string::size_type loc = line.find(",", 0);
            if (loc != string::npos)
            {
                u = stoi(line.substr(0, loc));
                v = stoi(line.substr(loc+1, string::npos));
                E.push_back(make_pair(u, v));
            }
            else throw runtime_error("Malformed line.");
        }
        infile.close();
    }
    else throw runtime_error("Unable to open edge set file.");
}

void write_pagerank_vector_to_file(const string &filename, const vector<double> &pi)
{

    int n = pi.size();

    // Open file for output and overwrite file if exists
    ofstream outfile(filename, ios::out | ios::trunc);

    if (outfile.is_open())
    {
        for (int i = 0; i < n; ++i)
        {
            outfile << pi[i] << '\n';
        }
        outfile.close();
    } else throw runtime_error("Unable to open output file.");

}

/*
    Count the number of lines in the file
    with filename `filename`.
*/
int count_lines(const string &filename)
{
    int n;
    ifstream infile(filename);
    
    if (infile.is_open())
    {
        // count number of lines in file
        n = count(istreambuf_iterator<char>(infile),
                istreambuf_iterator<char>(), '\n');
        infile.close();
    }
    else throw runtime_error("Unable to open file.");

    return n;
}