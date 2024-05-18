#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <random>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Global variables to store the number of components, nets, and grid dimensions
int num_of_components, num_of_nets, rows, columns;

// Function to parse the input file and create the netlist
vector<vector<int>> parser_function()
{
    vector<vector<int>> netlist; // 2D vector to store the netlist
    ifstream file("file1.txt");  // Open the input file
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

// Function to calculate the total wire length (WL) for the current placement
int calculate_WL(vector<vector<int>> netlist, vector<pair<int, int>> components_locations) {
    int WL = 0; // Initialize total wire length
    // Iterate over all nets
    for (int i = 1; i < num_of_nets; i++)
    {
        // Initialize min and max coordinates
        int min_x = 1000;
        int min_y = 1000;
        int max_x = -1;
        int max_y = -1;
        // Iterate over all components in the net
        for (int j = 1; j <= netlist[i][0]; j++)
        {
            int x = components_locations[netlist[i][j]].first;
            int y = components_locations[netlist[i][j]].second;
            if (min_x > x) min_x = x;
            if (min_y > y) min_y = y;
            if (max_x < x) max_x = x;
            if (max_y < y) max_y = y;
        }
        // Calculate wire length for the net and add to total
        WL += (max_x - min_x) + (max_y - min_y);
    }
    return WL;
}

int main() {
    vector<vector<int>> netlist; // 2D vector to store the netlist
    vector<pair<int, int>> components_locations; // Vector to store component locations
    
    // Parse the netlist from the file
    netlist = parser_function();
    // Read global parameters from the first line of the netlist
    num_of_components = netlist[0][0];
    num_of_nets = netlist[0][1];
    rows = netlist[0][2];
    columns = netlist[0][3];

    int array[rows][columns]; // 2D array to represent the grid
    // Initialize the grid with -1 (empty)
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            array[i][j] = -1;
        }
    }
    
    // Seed the random number generator
    srand(time(NULL));
    // Randomly place components in the grid
    for (int i = 0; i < num_of_components; i++) {
        int x, y;
        do {
            x = rand() % rows;
            y = rand() % columns;
        } while (array[x][y] != -1);
        // Place the component
        components_locations.push_back({x, y});
        array[x][y] = i;
    }
    
    // Display the initial random placement
    cout << "Initial random placement\n";
    int current_wire_length = calculate_WL(netlist, components_locations);
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
    
    // Simulated Annealing Parameters
    double initial_temperature = 500 * current_wire_length;
    double current_temperature = initial_temperature;
    double final_temperature = 5 * pow(10, -6) * (current_wire_length / num_of_nets);
    double cooling_factor = 0.95;

    // Start timing the simulated annealing process
    auto start_time = high_resolution_clock::now();

    // Simulated Annealing Algorithm
    while (current_temperature > final_temperature) {
        for (int i = 0; i < 20 * num_of_components; i++) {
            int x1, y1, x2, y2, id;
            // Select a random component and a new random position
            do {
                id = rand() % num_of_components;
                x1 = components_locations[id].first;
                y1 = components_locations[id].second;
                x2 = rand() % rows;
                y2 = rand() % columns;
            } while ((x1 == x2) && (y1 == y2));
            
            // Create a temporary copy of component locations
            vector<pair<int, int>> temp_components_locations = components_locations;
            if (array[x2][y2] != -1) {
                // Swap components if the new position is occupied
                swap(temp_components_locations[id], temp_components_locations[array[x2][y2]]);
            } else {
                // Move component to the new position
                temp_components_locations[id] = {x2, y2};
            }

            // Calculate new wire length and delta
            int new_wire_length = calculate_WL(netlist, temp_components_locations);
            int delta_wire_length = new_wire_length - current_wire_length;

            // Decide whether to accept the new placement
            if (delta_wire_length < 0) {
                current_wire_length = new_wire_length;
                components_locations = temp_components_locations;
                swap(array[x1][y1], array[x2][y2]);
            } else {
                int rejection_probability = (1 - exp(-delta_wire_length / current_temperature)) * 100;
                int random_number = rand() % 101;
                if (random_number > rejection_probability) {
                    current_wire_length = new_wire_length;
                    components_locations = temp_components_locations;
                    swap(array[x1][y1], array[x2][y2]);
                }
            }
        }
        // Cool down the temperature
        current_temperature = current_temperature * cooling_factor;
    }

    // Stop timing the simulated annealing process
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end_time - start_time);
    
    // Display the final placement
    cout << "Final random placement\n";
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
    cout << "Time taken to find the optimal solution: " << duration.count() << " seconds" << endl;
    
    return 0;
}
