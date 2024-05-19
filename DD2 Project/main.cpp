#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <random>
#include <chrono>
#include <set>
#include <iterator>
#include <cmath>
#include <unordered_set>

using namespace std;
using namespace std::chrono;

int num_of_components, num_of_nets, rows, columns, current_wire_length;
vector<vector<int>> netlist;
unordered_set<int> affected_nets;
vector<pair<int, int>> components_locations;
vector<int> temp_netlist_lengths;

vector<vector<int>> parser_function()
{
//    string file_name;
//    cout << "Please inpuy file name: ";
//    cin >> file_name;
//    ifstream file(file_name);  // Open the input file
    
    vector<vector<int>> netlist; // 2D vector to store the netlist
    ifstream file("file3.txt");  // Open the input file
    if (!file.is_open())
    { // Check if file opened successfully
        cout << "Failed to open the file." << endl;
        exit(0);
    }
    
    string line;
    // Read the file line by line
    while (getline(file, line))
    {
        istringstream iss(line);
        int number;
        vector<int> lines; // Vector to store numbers from each line
        // Extract numbers from the line
        while (iss >> number)
        {
            lines.push_back(number);
        }
        // Add the line vector to the netlist
        netlist.push_back(lines);
        lines.clear(); // Clear the line vector for the next line
    }
    file.close(); // Close the file
    return netlist;
}

int calculate_WL(const vector<pair<int, int>>& components_locations, vector<vector<int>>& components_netlists, vector<int>& netlist_lengths) {
    int WL = 0;
    for (int i = 1; i <= num_of_nets; i++) {
        int min_x = INT_MAX;
        int min_y = INT_MAX;
        int max_x = INT_MIN;
        int max_y = INT_MIN;
        for (int j = 1; j <= netlist[i][0]; j++)
        {
            int x = components_locations[netlist[i][j]].first;
            int y = components_locations[netlist[i][j]].second;
            min_x = min(min_x, x);
            min_y = min(min_y, y);
            max_x = max(max_x, x);
            max_y = max(max_y, y);
            components_netlists[netlist[i][j]].push_back(i-1);
        }
        int net_length = (max_x - min_x) + (max_y - min_y);
        netlist_lengths[i-1] = net_length;
        WL += net_length;
    }
    return WL;
}

int calculate_new_WL() {
    int net_length, x, y, min_x, min_y, max_x, max_y, j;
    int temp_wire_length = current_wire_length;
    for (const int& net : affected_nets) {
        temp_wire_length -= temp_netlist_lengths[net];
        min_x = INT_MAX;
        min_y = INT_MAX;
        max_x = INT_MIN;
        max_y = INT_MIN;
        for (j = 1; j <= netlist[net+1][0]; j++) {
            x = components_locations[netlist[net+1][j]].first;
            y = components_locations[netlist[net+1][j]].second;
            min_x = min(min_x, x);
            min_y = min(min_y, y);
            max_x = max(max_x, x);
            max_y = max(max_y, y);
        }
        net_length = (max_x - min_x) + (max_y - min_y);
        temp_netlist_lengths[net] = net_length;
        temp_wire_length += net_length;
    }
    affected_nets.clear();
    return temp_wire_length;
}

int main() {
    netlist = parser_function();
    num_of_components = netlist[0][0];
    num_of_nets = netlist[0][1];
    rows = netlist[0][2];
    columns = netlist[0][3];
    
    vector<vector<int>> components_netlists(num_of_components);
    vector<int> netlist_lengths(num_of_nets);
    vector<vector<int>> array(rows, vector<int>(columns, -1));
    
    srand(time(NULL));
    for (int i = 0; i < num_of_components; i++) {
        int x, y;
        do {

            x = rand() % rows;
            y = rand() % columns;
        } while (array[x][y] != -1);
        components_locations.emplace_back(x, y);
        array[x][y] = i;
    }

    current_wire_length = calculate_WL(components_locations, components_netlists, netlist_lengths);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if (array[i][j] == -1) {
                cout << "--- ";
            } else if ((array[i][j] > -1) && (array[i][j] < 10)) {
                cout << "00" << array[i][j] << " ";
            } else if ((array[i][j] > 9) && (array[i][j] < 100)) {
                cout << "0" << array[i][j] << " ";
            } else {
                cout << array[i][j] << " ";
            }
        }
        cout << endl;
    }
    cout << "Initial total wire length = " << current_wire_length << endl;
    
    double initial_temperature = 500 * current_wire_length;
    double current_temperature = initial_temperature;
    double final_temperature = 5e-6 * (current_wire_length / num_of_nets);
    double cooling_factor = 0.95;
    int x1, y1, x2, y2, id, id2, new_wire_length, delta_wire_length, i;
    auto start_time = high_resolution_clock::now();
    while (current_temperature > final_temperature) {
        for (i = 0; i < 20 * num_of_components; i++) {
            id = rand() % num_of_components;
            x1 = components_locations[id].first;
            y1 = components_locations[id].second;
            x2 = rand() % rows;
            y2 = rand() % columns;
            id2 = array[x2][y2];
            if (id2 != -1) {
                affected_nets.insert(components_netlists[id].begin(), components_netlists[id].end());
                affected_nets.insert(components_netlists[id2].begin(), components_netlists[id2].end());
                swap(components_locations[id], components_locations[id2]);

            } else {
                affected_nets.insert(components_netlists[id].begin(), components_netlists[id].end());
                components_locations[id] = {x2, y2};

            }
            temp_netlist_lengths = netlist_lengths;
            new_wire_length = calculate_new_WL();
            delta_wire_length = new_wire_length - current_wire_length;
            if ((delta_wire_length < 0) || (rand() / (RAND_MAX + 1.0)) < exp(-delta_wire_length/current_temperature)){
                current_wire_length = new_wire_length;
                netlist_lengths = temp_netlist_lengths;
                swap(array[x1][y1], array[x2][y2]);
            } else if (id2 != -1) {
                swap(components_locations[id], components_locations[id2]);
            } else {
                components_locations[id] = {x1, y1};
            }
        }
        current_temperature *= cooling_factor;
    }
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            if (array[i][j] == -1) {
                cout << "--- ";
            } else if ((array[i][j] > -1) && (array[i][j] < 10)) {
                cout << "00" << array[i][j] << " ";
            } else if ((array[i][j] > 9) && (array[i][j] < 100)) {
                cout << "0" << array[i][j] << " ";
            } else {
                cout << array[i][j] << " ";
            }
        }
        cout << endl;
    }
    cout << "Final total wire length = " << current_wire_length << endl;
    cout << "Time taken to find the optimal solution: " << duration.count() << " milliseconds" << endl;

    return 0;
}
