#include <iostream>
#include <vector>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <random>
#include <fstream>
#include <unordered_set>

using namespace std;

const int HASH_LENGTH = 128;
const uint64_t PRIME1 = 11400714785074694791ULL;
const uint64_t PRIME2 = 14029467366897019727ULL;
const uint64_t PRIME3 = 10092003300140014003ULL;

long int keys[10] = {0};
string possiblSolutions[10] = {""};
uint64_t problem = 0;
const string KEY_FILE = "keys.csv";

void loadKeys()
{
    ifstream file(KEY_FILE);
    if (!file)
    {
        cout << "No existing keys found. Starting fresh.\n";
        return;
    }

    int i = 0;
    while (file >> keys[i] && i < 10)
    {
        i++;
    }
    file.close();
    cout << "Keys loaded successfully.\n";
}

void saveKeys()
{
    ofstream file(KEY_FILE);
    for (int i = 0; i < 10; i++)
    {
        if (keys[i] != 0)
        {
            file << keys[i] << endl;
        }
    }
    file.close();
}

void registerKey()
{
    cout << "Enter key: (integer)\n";
    long int newKey;
    cin >> newKey;

    for (int i = 0; i < 10; i++)
    {
        if (keys[i] == newKey)
        {
            cout << "Key already registered!\n";
            return;
        }
    }

    for (int i = 0; i < 10; i++)
    {
        if (keys[i] == 0)
        {
            keys[i] = newKey;
            cout << "Key Registered \n";
            saveKeys();
            return;
        }
    }
    cout << "Key list full!\n";
}

uint64_t Salt(const string &problem, const string &keyId)
{
    uint64_t salt = PRIME1;
    for (int c : (problem + keyId))
    {
        salt ^= static_cast<uint64_t>(c) * PRIME2;
        salt = (salt << 7) | (salt >> (64 - 7));
        salt *= PRIME1;
        salt ^= (salt >> 17) | (salt << (64 - 17));
    }
    return salt;
}

vector<uint64_t> Lattice(const string &problem, uint64_t salt)
{
    vector<uint64_t> poly(HASH_LENGTH / 64);
    for (size_t i = 0; i < problem.size(); i++)
    {
        poly[i % poly.size()] ^= (static_cast<uint64_t>(problem[i]) + salt) * PRIME1;
    }
    return poly;
}

void bitwiseMixing(vector<uint64_t> &poly)
{
    for (size_t i = 0; i < poly.size(); i++)
    {
        poly[i] ^= (poly[i] << 31) | (poly[i] >> (64 - 31));
        poly[i] *= PRIME1;
        poly[i] ^= (poly[i] << 17) | (poly[i] >> (64 - 17));
    }
}

void neighborBasedTransformation(vector<uint64_t> &poly)
{
    size_t n = poly.size();
    for (size_t i = 0; i < n; i++)
    {
        uint64_t left = poly[(i + n - 1) % n];
        uint64_t right = poly[(i + 1) % n];
        poly[i] ^= (left >> 3) ^ (right << 5);
        poly[i] *= PRIME2;
        poly[i] ^= (poly[i] << 11) | (poly[i] >> (64 - 11));
    }
}

string generateHash(const vector<uint64_t> &poly)
{
    ostringstream oss;
    for (size_t i = 0; i < poly.size(); i++)
    {
        oss << poly[i];
    }
    return oss.str();
}

string Hash(const string &password, const string &username)
{
    uint64_t salt = Salt(password, username);
    vector<uint64_t> poly = Lattice(password, salt);
    neighborBasedTransformation(poly);
    bitwiseMixing(poly);
    return generateHash(poly);
}

uint64_t generateProblem()
{
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dis(1000000000000000000ULL, 9999999999999999999ULL);
    return dis(gen);
}

void generateSolutions()
{
    for (int i = 0; i < 10; i++)
    {
        if (keys[i] != 0)
        {
            possiblSolutions[i] = Hash(to_string(keys[i]), to_string(problem));
        }
    }
    cout << "Generated solutions:\n";
    for (int i = 0; i < 10; i++)
    {
        if (keys[i] != 0)
        {
            cout << possiblSolutions[i] << "\n";
        }
    }
}

int main()
{
    loadKeys();

    char buffer1 = 't';
    while (buffer1 != 'q')
    {
        int choice = 0;
        cout << "MENU MODE\n1. Register Key\n2. Main Menu\n";
        cin >> choice;
        switch (choice)
        {
        case 1:
            registerKey();
            break;
        case 2:
            buffer1 = 'q';
            break;
        default:
            cout << "Invalid Choice\n";
        }
    }

    char buffer = 't';
    while (buffer != 'q')
    {
        long int keyId;
        cout << "Enter keyId: ";
        cin >> keyId;

        cout << "Generating Problem\n";
        problem = generateProblem();
        cout << "Problem: " << problem << endl;
        cout << "Passing Problem to key\n";
        string hash1 = Hash(to_string(keyId), to_string(problem));
        cout << "Solution of key:\n"
             << hash1 << "\n";
        cout << "Key returning solution\n";
        generateSolutions();
        cout << "Verifying soln\n";
        bool accessGranted = false;
        for (int i = 0; i < 10; i++)
        {
            if (possiblSolutions[i].empty())
            {
                break;
            }
            if (possiblSolutions[i] == hash1)
            {
                cout << "Key Verified\n";
                cout << "Unlocked\n";
                accessGranted = true;
                break;
            }
        }
        if (!accessGranted)
        {
            cout << "Key Verification Failed\n";
            cout << "Unlock request failed\n";
        }

        cout << "Press 'q' to quit or any other key to continue: ";
        cin >> buffer;
    }

    return 0;
}
