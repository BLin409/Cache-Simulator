/**
 * CS 320 - Computer System III
 * Project 2 - Cache Simulator
 *
 * @author Cheng Lin
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

#define OFFSET 6
#define BASE 64

struct Cache_Block {
    int validBit;
    int tag;
};

int main(int argc, char* argv[])
{
    void initialSets(Cache_Block** block, int sets, int ways);
    bool directCache(Cache_Block* block, int indexIn, int tagIn);
    bool assocCache(Cache_Block* block, int tagIn, int ways, bool load);
	bool fullyRandCache(Cache_Block* block, int tagIn);

    if (argc != 3) {
        cerr << "ERROR: Check your input AGAIN!\n";
        return 0;
    }

    ifstream fin(argv[1]);
    if (!fin.is_open()) {
        cerr << "ERROR: The input file does not exist!\n";
        return 1;
    }

    ofstream fout(argv[2]);
    if (!fout.is_open()) {
        cerr << "ERROR: The output file does not exist!\n";
        return 2;
    }

	srand(time(NULL));

    int dirSets[] = {64, 128, 256, 512, 1024};
    Cache_Block** direct = new Cache_Block*[5];
    int setAssocWays[] = {2, 4, 8, 16};
    Cache_Block*** setAssoc = new Cache_Block**[4];
    Cache_Block* fullyLRU = new Cache_Block[1024];
	Cache_Block* fullyRand = new Cache_Block[1024];
	Cache_Block*** setAssocLorS = new Cache_Block**[4];

    for (int i = 0; i < 4; i++) 
    {
        direct[i] = new Cache_Block[(1 << i) * BASE];
        
        int numSets = (8 >> i) * BASE;
        setAssoc[i] = new Cache_Block*[numSets];
		setAssocLorS[i] = new Cache_Block*[numSets];
        for (int j = 0; j < numSets; j++) {
            setAssoc[i][j] = new Cache_Block[setAssocWays[i]];
			setAssocLorS[i][j] = new Cache_Block[setAssocWays[i]];
        }
        initialSets(setAssoc[i], numSets, setAssocWays[i]);
		initialSets(setAssocLorS[i], numSets, setAssocWays[i]);
    }
    direct[4] = new Cache_Block[16 * BASE];   

	//Set all the validBits to invalid(0)
    for (int i = 0; i < 64; i++) {
        direct[0][i].validBit = direct[1][i].validBit = direct[2][i].validBit =
        direct[3][i].validBit = direct[4][i].validBit = 0;
        fullyLRU[i].validBit = fullyRand[i].validBit = 0;
    }
    for (int i = 64; i < 128; i++) {
        direct[1][i].validBit = direct[2][i].validBit = direct[3][i].validBit =
        direct[4][i].validBit = 0;
        fullyLRU[i].validBit = fullyRand[i].validBit = 0;
    }
    for (int i = 128; i < 256; i++) {
        direct[2][i].validBit = direct[3][i].validBit = direct[4][i].validBit = 0;
        fullyLRU[i].validBit = fullyRand[i].validBit = 0;
    }
    for (int i = 256; i < 512; i++) {
        direct[3][i].validBit = direct[4][i].validBit = 0;
        fullyLRU[i].validBit = fullyRand[i].validBit = 0;
    }
    for (int i = 512; i < 1024; i++) {
        direct[4][i].validBit = 0;
        fullyLRU[i].validBit = fullyRand[i].validBit = 0;
    }

	//Counters
    int hitDir[5] = {0};
    int hitSetAssoc[4] = {0};
	int hitFullyLRU = 0;
	int hitFullyRand = 0;
	int hitSetAssocLorS[4] = {0};
    int iCount = 0;

    long long address;
    char lorS;
    while (fin >> lorS >> hex >> address)
    {
		// Reading Trace File
		// format: L/S(whether its a load or store instruction)\ address(hex)
        int index;
        int tag;
        address >>= OFFSET;
		bool load = (lorS == 'L');

        for (int i = 0; i < 5; i++) {
            index = address % dirSets[i];
            tag = address / dirSets[i];
            hitDir[i] += directCache(direct[i], index, tag);
        }

        for (int i = 0; i < 4; i++) {
            int factor = (8 >> i) * BASE;
            index = address % factor;
            tag = address / factor;
            hitSetAssoc[i] += assocCache(setAssoc[i][index], tag, setAssocWays[i], 1);
			hitSetAssocLorS[i] += assocCache(setAssocLorS[i][index], tag, setAssocWays[i], load);
        }

		hitFullyLRU += assocCache(fullyLRU, address, 1024, 1);
		hitFullyRand += fullyRandCache(fullyRand, address);
		
        iCount++;
    }

    int hitRateDir[5] = {0};
    for (int i = 0; i < 4; i++) {
        hitRateDir[i] = (int)((float)hitDir[i] * 100 / iCount + 0.5f);
        fout << hitRateDir[i] << " ";
    }
    hitRateDir[4] = (int)((float)hitDir[4] * 100 / iCount + 0.5f);
    fout << hitRateDir[4] << endl;

    int hitRateSetAssoc[4] = {0};
    for (int i = 0; i < 3; i++) {
        hitRateSetAssoc[i] = (int)((float)hitSetAssoc[i] * 100 / iCount + 0.5f);
        fout << hitRateSetAssoc[i] << " ";
    }
    hitRateSetAssoc[3] = (int)((float)hitSetAssoc[3] * 100 / iCount + 0.5f);
    fout << hitRateSetAssoc[3] << endl;
	
	int hitRateFullyLRU = (int)((float)hitFullyLRU * 100 / iCount + 0.5f);
	int hitRateFullyRand = (int)((float)hitFullyRand * 100 / iCount + 0.5f);
    fout << hitRateFullyLRU << " " << hitRateFullyRand << endl; 
	
	int hitRateSetAssocLorS[4] = {0};
    for (int i = 0; i < 3; i++) {
        hitRateSetAssocLorS[i] = (int)((float)hitSetAssocLorS[i] * 100 / iCount + 0.5f);
        fout << hitRateSetAssocLorS[i] << " ";
    }
    hitRateSetAssocLorS[3] = (int)((float)hitSetAssocLorS[3] * 100 / iCount + 0.5f);
    fout << hitRateSetAssocLorS[3] << endl;
	
	fin.close();
	fout.close();
	
	for (int i = 0; i < 5; i++) {
		delete [] direct[i];
	}
	delete [] direct;
	
	for (int i = 0; i < 4; i++) {
		int numSets = (8 >> i) * BASE;
		for (int j = 0; j < numSets; j++) {
			delete [] setAssoc[i][j];
			delete [] setAssocLorS[i][j];
		}
		delete [] setAssoc[i];
		delete [] setAssocLorS[i];
	}
	delete [] setAssoc;
	delete [] setAssocLorS;
	
    delete [] fullyLRU;
	delete [] fullyRand;
}

/**
 * Direct-Mapped Cache Simulation
 *
 * @param block direct-mapped cache
 * @param index locate the correspond set
 * @param tagIn use to check if the same tag is in the set
 * @return whether there's a cache hit
 */
bool directCache(Cache_Block* block, int indexIn, int tagIn)
{
    if (block[indexIn].validBit == 0) {
        block[indexIn].validBit = 1;
        block[indexIn].tag = tagIn;
        return false;
    } else {
        if (block[indexIn].tag == tagIn) {
            return true;
        } else {
            block[indexIn].tag = tagIn;
            return false;
        }
    }
}

/**
 * Set-Associative Cache & Fully-Associative Cache Simulation
 * Also use to simulate Set-Associative Cache with no allocation on a write Miss
 * All of them uses Least Recently Used (LRU) replacement policy
 *
 * @param block correspond set
 * @param tagIn use to check if the same tag is in the set
 * @param ways number of ways the set has
 * @param load where the instruction is load or store
 * @return whether there's a cache hit
 */
bool assocCache(Cache_Block* block, int tagIn, int ways, bool load)
{
    bool hit = false;
    int indexMax = 0;
    for (int i = 0; i < ways; i++) {
        if (block[i].validBit == 0) {
            if (!hit) {
				if (load) {
					block[i].validBit = 1;
					block[i].tag = tagIn;
				}
                return false;
            } else {
                return true;
            }
        } else {
            if (block[i].tag == tagIn) {
                block[i].validBit = 1;
                hit = true;
            } else {
                block[i].validBit++;
                if (block[indexMax].validBit < block[i].validBit) {
                    indexMax = i;
                } 
            }
        }
    }
    
    if (!hit && load) {
        block[indexMax].tag = tagIn;
        block[indexMax].validBit = 1;
    }
    return hit;
}

/**
 * Fully-Associative Cache Simulation
 * Uses Randonmized replacement policy
 *
 * @param block correspond set
 * @param tagIn use to check if the same tag is in the set
 * @return whether there's a cache hit
 */
bool fullyRandCache(Cache_Block* block, int tagIn)
{
    for (int i = 0; i < 1024; i++) {
        if (block[i].validBit != 0 && block[i].tag == tagIn) {
            return true;
        }
    }
    
	int index = rand() % 1024;
    block[index].tag = tagIn;
    block[index].validBit = 1;

    return false;
}

/**
 * Initialize all valid bits to invalid(0)
 *
 * @param block cache
 * @param sets number of sets in the cache
 * @param ways number of ways of the cache
 */
void initialSets(Cache_Block** block, int sets, int ways) 
{
    for (int i = 0; i < sets; i++) {
        for (int j = 0; j < ways; j++) {
            block[i][j].validBit = 0;
        }
    }
}
