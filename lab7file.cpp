//Palmer Swanson
//CWID 11726494
//ECE 487 Lab 7 Memory Simulator
//Eat your heart out Max
//But also thank you

#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <fstream>    //for reading files
#include <utility>
#include <cstring>

//defining a struct for user input
struct user_struct {
    int bytesOfMainMemory = 0;
    int bytesOfCacheMemory = 0;
    int blocksize = 0;
    int degreesetAssociativity = 0;
    char replacementpolicy = 'k';
    std::string fileName = "";
};

//defining a struct for the Cache
struct CacheBlocks {
    int age;
    bool dirtybit = 0;
    bool validbit = 0;
    std::string tag = "?";
    std::string data = "?";
};

//queries the user to enter an int.  This int will be the size of the main memory in bytes
int main_memory_size_query() {
    int size;                                                                  //init integer
    std::cout << "Enter the size of main memory in bytes: ";                    //std::cout to get user input
    std::cin >> size;                                                          //read in inputted value
    return size;                                                               //return integer
}

//queries the user to enter an int.  This int will be the size of the cache for this iteration of the Simulator
int cache_memory_query() {
    int size;                                                                  //init integer
    std::cout << "Enter the size of the cache in bytes: ";                     //std::cout to get user input
    std::cin >> size;                                                         //read in inputted value
    return size;                                                              //return integer
}

//queries the user to enter an int.  This int will be the size of the cache block for this iteration of the Simulator
int block_size_query() {
    int size;                                                                //init integer
    std::cout << "Enter the cache block/line size: ";                         //std::cout to get user input
    std::cin >> size;                                                        //read in inputted value
    return size;                                                             //return integer
}

//queries the user to enter an int.  This int will be the n-wway set associative mapping for this iteration of the Simulator
int set_associativity_query() {
    int degree;                                                               //init integer
    std::cout << "\nEnter the degree of set-associativity (input n for an n-way set-associative mapping): ";            //std::cout to get user input
    std::cin >> degree;                                                      //read in inputted value
    return degree;                                                          //return integer
}

//queries the user to enter an char.  This char will be thereplacement policy for this iteration of the Simulator
char replacementPolicy_query() {
    char LorF;                                                                        //init char
    std::cout << "Enter the replacement policy (L = LRU, F = FIFO): ";                //std::cout to get user input
    std::cin >> LorF;                                                                //read in inputted value
    return LorF;                                                                    //return integer
}

//queries the user to enter an string.  This string will be the name of the input file for this iteration of the Simulator
std::string inputFileNameQuery() {
    std::string input;                                                                                                                      //init string
    std::cout << "Enter the name of the input file containing the list of memory references generated by the CPU:";                         //query user for input in "filename.txt" format
    std::cin >> input;                                                                                                                     //read in string name
    return input;                                                                                                                          //return string
}

//returns an int that is the number of address lines needed for this simulation.  uses the log2 function from the math.h library
int addresslines_calc(int MainMem) {
    int temp = (int)log2(MainMem);
    return temp;
}

//Calculated the number of bits required for offset.  Input is an int and output is also an int
int bitsforoffset_calc(int blockSize) {
    return (int)log2(blockSize);
}

//Calculates number of bits for index.  Input is the int size of the cache, int block size, and int nway associativity
int bitsforindex_calc(int cacheSize, int blockSize, int nwayassociativity) {
    int blocksInCache = (cacheSize / blockSize);
    int temp = (blocksInCache / nwayassociativity);
    int numOfBits = (int)log2(temp);
    return numOfBits;
}

//Calculates the number of bits for the tag
int bitsforTag_calc(int addressbits, int offsetBits, int indexBits) {
    return (addressbits - offsetBits - indexBits);
}

//returns the number of blocks in Cache Memory from user inputs of Cache size and block size
int numberofCMblks(int sizeofCache, int sizeofBlock) {
    int temp = sizeofCache / sizeofBlock;                                         //create temp variable
    return temp;                                                                //return this temp variable
}

//gets the two columns from the data file
void meat_of_input_file(std::string input_file, std::vector<char>& read_write, std::vector<int>& memorylocations, std::vector<int>& mmblock, std::vector<int>& cmset, int cachesize, int blocksize, int nwayassociativity) {
    int temp = (cachesize/ blocksize);
    int temp1 = (temp/nwayassociativity);
    int number = 0;                                       //declare a variable to keep track of the elements in the vectors
    std::string line1;                                    //string for the line to be read from file
    std::string word;
    std::string value;
    std::ifstream stream(input_file.c_str());
    stream.ignore(1, '\n');                           //to ignore the first line
    while (getline(stream, line1)) {
        stream >> word >> value;                      //scan word and value from the line
        read_write.push_back(word[0]);                //add word to read_write vector
        number = stoi(value);                         //string to integer the memory value
        memorylocations.push_back(number);            //pushback the vector of memory locations
        mmblock.push_back(number / blocksize);
        cmset.push_back((number/blocksize) % temp1);
    }
    //These next four lines were added because without them, the last element of the vectors was repeated.
    read_write.pop_back();
    memorylocations.pop_back();
    mmblock.pop_back();
    cmset.pop_back();
}

//Get the range of the cache memory blocks
void range_cmblk(std::vector<int> cmset, std::vector<std::pair<int, int>>& p, int associativity){
  int temp;
  int temp1;
  for(int i = 0; i < cmset.size(); i++){
    temp = cmset[i]*associativity;                                            //calculating bottom range of possible block numbers
    temp1 = (cmset[i]*associativity) + (associativity - 1);                   //calculating top range of possible block numbers
    p.push_back(std::make_pair(temp, temp1));                                 //add the two values to the vector
  }
}

//calculates final size of Cache.  INput is the original size of cache, number of CM blocks, and number of tag bits.
int finalsizeofCache(int origianlCMsize, int numOfBlocks, int numberOfTagBits) {
    int temp = (1 + 1 + numberOfTagBits);                       //int for value of the sum of compiler directives and tag bits
    int temp1 = temp * (numOfBlocks);                                           //int for temp*numOfBlocks
    int temp3 = temp1 / 8;                                                      //divide temp1 by 8 bits per byte
    int temp4 = origianlCMsize + temp3;                                       //add this to the original cache memory size
    return temp4;                                                             //return this value
}

//function to convert a decimal tag to binary.  Input is the decimal number n and the number of tag bits
//Was surprised that there wasn't any built in or included function to convert to binary
std::string toBinary(int n, int numofTagbits) {
    std::string r;
    for (int i = 0; i < numofTagbits; i++) {            //calculate the binary number consisting of numofTagBits number of bits
        r += (n % 2 == 0 ? "0" : "1");                  //spit out string of 1 or 0
        n /= 2;
    }
    return r;                                         //return the string of the bianry number
}

//calculate tag from the memory location
std::string tag(int memorylocation, int numOffset, int indexbits, int numtagbits){
  int temp = numOffset + indexbits;
  int temp1 = pow(2, temp);
  int temp2 = memorylocation/temp1;;
  std::string temp3 = toBinary(temp2, numtagbits);
  return temp3;
}

void xs(int numtag){
  for (int i = 0; i < numtag; i++){
    std::cout << "X";
  }
}

//formats the data field for the second table to include mm blk #
std::string Data(int block){
  std::string temp1;
  std::string temp2;
  std::string temp3;
  temp1 = "mm blk # ";
  temp2 = std::to_string(block);
  temp3 = temp1 + temp2;
  return temp3;
}

//Calculate the number of hits and total memory locations.  Also determine the hit rate as a percentage
void optimal_hit_rate(std::vector<int> mainmemblock, int& hits, int& total, float& percentage) {
    int i, j, count = 0;
    for (i = 0; i < mainmemblock.size(); i++) {
        for (j = i + 1; j < mainmemblock.size(); j++) {                         //loop through the main memory block vector and count the number of repeated instances
            if (mainmemblock[i] == mainmemblock[j]) {
                count++;                                                        //increment for eeach hit
                break;
            }
        }
    }
    hits = count;
    total = mainmemblock.size();
    percentage = (((float)count / mainmemblock.size()) * 100);                  //calculate percentage from hits and total values
}

//calculate the actual hit rate for the given memory configuration
float actual_hit_rate(int count, int total){
  float temp = (float) count/total;
  return temp*100;
}

/*
void ages(CacheBlocks CacheMemory[], int numcmblks, std::vector<std::pair<int, int>> cmpair, int associativity){
  int temp = numcmblks/associativity;
  for(int i = 0; i < temp; i++){
    for (int j = 0; j < associativity; j++){
      CacheMemory[(i*associativity)+j].age = j;
    }
  }
  for(int k = 0; k < numcmblks; k++){
    std::cout << k << "\t" << CacheMemory[k].age << "\t";
  }
}
*/

int pop_cache(CacheBlocks CacheMemory[], std::vector<char> rw, std::vector<int> mmadd, std::vector<int> mmblk, std::vector<int> cmsetnum, std::vector<std::pair<int, int>> cmpair, int offset, int index, int tagbits, int numcmblks, char policy, int associativity){
  int ageofblks[numcmblks] = {0};
  int counter = 0;
  for (int i = 0; i < cmsetnum.size(); i++){
    int index_age = 0;
    int index_hit = 0;
    bool hit = false;
    for(int j = 0; j < numcmblks; j++){
      if (j >= cmpair[i].first && j <= cmpair[i].second){
        std::cout << "checking block " << j << " age = " << CacheMemory[j].age << std::endl;
        if(CacheMemory[j].age == 0){
          std::cout << "Found index of 0\t" << j << std:: endl;
          index_age = j;
        }
        if(CacheMemory[j].validbit == 1 && CacheMemory[j].tag == tag(mmadd[i], offset, index, tagbits)){
          std::cout << "HIT block " << j << std::endl;
          index_hit = j;
          counter++;
          hit = true;
          if(rw[i] == 'W'){
            CacheMemory[j].dirtybit = 1;
          }
          break;
        }
        else if(j == cmpair[i].second){
          std::cout << index_age << "\t" << "adding to oldest index" << std::endl;
          CacheMemory[index_age].validbit = 1;
          CacheMemory[index_age].tag = tag(mmadd[i], offset, index, tagbits);
          CacheMemory[index_age].data = Data(mmblk[i]);
          if(rw[i] == 'W'){
            CacheMemory[index_age].dirtybit = 1;
          }
          else if (rw[i] == 'R'){
            CacheMemory[index_age].dirtybit = 0;
          }
          break;
        }
      }
    }
      //replacement policy stuff
      if (policy == 'L'){
        if (hit == true){                                                   //the behavior for LRU for hits and not hits are almost identiocal
          int previous_age = CacheMemory[index_hit].age;
          CacheMemory[index_hit].age = (associativity-1);
          for (int k = 0; k < numcmblks; k++){
            if (k >= cmpair[i].first && k <= cmpair[i].second){
              std::cout << "Updating age of k " << k << " to ";
              if (CacheMemory[k].age > previous_age){
                CacheMemory[k].age--;
              }
            }
          }
        }
        else if (hit == false){
          int previous_age = CacheMemory[index_age].age;
          CacheMemory[index_age].age = (associativity-1);
          for (int k = 0; k < numcmblks; k++){
            if (k >= cmpair[i].first && k <= cmpair[i].second){
              std::cout << "Updating age of k " << k << " to ";
              if (CacheMemory[k].age > previous_age){
                CacheMemory[k].age--;
              }
            }
          }
        }
      }
      //FIFO replacement policy
      else if(policy == 'F'){
        if (hit == false){                                                      //since we only care about the first in, we don't care if theres a hit
          for (int k = 0; k < numcmblks; k++){
            if (k >= cmpair[i].first && k <= cmpair[i].second){
              std::cout << "Updating age of k " << k << " to ";
              CacheMemory[k].age++;
              if (CacheMemory[k].age >= associativity){
                CacheMemory[k].age = 0;
              }
              std::cout << CacheMemory[k].age << std::endl;
            }
          }
        }
      }
    }
  return counter;
}

// formats simulation output required in lab sheet.
void simulator_output(int requiredAddressLines, int offsetbits, int indexbits, int tagbits, int sizeCM) {
    std::cout << "\nSimulator Output:" << std::endl;
    std::cout << "Total address lines required = " << requiredAddressLines << std::endl;
    std::cout << "Number of bits for offset = " << offsetbits << std::endl;
    std::cout << "Number of bits for index = " << indexbits << std::endl;
    std::cout << "Number of bits for tag = " << tagbits << std::endl;
    std::cout << "Total cache size required = " << sizeCM << " bytes" << std::endl;
}

//formats the first table of the simulation output
void firsttable(std::vector<int> address, std::vector<int> block, std::vector<int> cmset, std::vector<std::pair<int, int>> range, float optimal, int hits, int total, int actual_hits, float actual_percent){
  std::cout << "\nMain Memory Address\t MM Blk#\t Cm Set#\t Cm Blk#\t hit/miss" << std::endl;
  std::cout << "----------------------------------------------------------------------------------" << std::endl;
  for (int i =0; i < address.size(); i++){
    std::cout << "\t  " <<address[i] << "\t\t    " << block[i] << "\t\t    " << cmset[i] << "\t\t   " << range[i].first << "-" << range[i].second << std::endl;
}
std::cout << "\nHighest possible hit rate = " << hits << "/" << total << " = "<< optimal << "%" << std::endl;
std::cout << "Actual hit rate = " << actual_hits << "/" << total << " = " << actual_percent << "%" << std::endl;
}

void secondtable(CacheBlocks CacheMemory[], int size){
  std::cout << "\n  Cache blk #\t dirty bit\t valid bit\t Tag\t\t Data" << std::endl;
  std::cout << "-----------------------------------------------------------------------------" << std::endl;
  for(int i = 0; i <size; i++){
    std::cout << "\t" << i << "\t      " << CacheMemory[i].dirtybit << " \t      " << CacheMemory[i].validbit;
    std::cout << "\t         " << CacheMemory[i].tag << "\t      " << CacheMemory[i].data<< std::endl;
  }
}

int main(){

    char user_loop_exit;
    bool exit_loop = false;

    while (exit_loop == false) {

        //defining the user input struct
        user_struct input;

        int totalbits;
        int offsetbits;
        int indexbits;
        int tagbits;
        int finalsize;
        int numberofCacheBlocks;

        //variables for best possible hit/miss ratio
        int hits = 0;
        int total = 0;
        float percentage = 0.0;
        int act_hits = 0;
        float actual_percentage = 0.0;

        //get vectors of memory info
        std::vector<char> readwrite;
        std::vector<int> mainMemAddress;
        std::vector<int> mainMemBlock;
        std::vector<int> cacheMemSet;
        std::vector<std::pair<int, int>> cmpair;

        //user input variables.  Input into the struct declcared inside main
        input.bytesOfMainMemory = main_memory_size_query();
        input.bytesOfCacheMemory = cache_memory_query();
        input.blocksize = block_size_query();
        input.degreesetAssociativity = set_associativity_query();
        input.replacementpolicy = replacementPolicy_query();
        input.fileName = inputFileNameQuery();

        //calculate the number of cache blocks
        numberofCacheBlocks = numberofCMblks(input.bytesOfCacheMemory, input.blocksize);

        //initialize an array of CacheVBlock structs called CacheMemory
        CacheBlocks CacheMemory[numberofCacheBlocks];

        //set variables based upon the output of various functions
        totalbits = addresslines_calc(input.bytesOfMainMemory);
        offsetbits = bitsforoffset_calc(input.blocksize);
        indexbits = bitsforindex_calc(input.bytesOfCacheMemory, input.blocksize, input.degreesetAssociativity);
        tagbits = bitsforTag_calc(totalbits, offsetbits, indexbits);
        finalsize = finalsizeofCache(input.bytesOfCacheMemory, numberofCacheBlocks, tagbits);

        meat_of_input_file(input.fileName, readwrite, mainMemAddress, mainMemBlock, cacheMemSet, input.bytesOfCacheMemory, input.blocksize, input.degreesetAssociativity);

        std::cout << mainMemAddress.size() << std::endl;

        simulator_output(totalbits, offsetbits, indexbits, tagbits, finalsize);
        optimal_hit_rate(mainMemBlock, hits, total, percentage);

        range_cmblk(cacheMemSet, cmpair, input.degreesetAssociativity);

        int temp = numberofCacheBlocks/input.degreesetAssociativity;
        for(int i = 0; i < temp; i++){
          for (int j = 0; j < input.degreesetAssociativity; j++){
            CacheMemory[(i*input.degreesetAssociativity)+j].age = j;
          }
        }
        for(int k = 0; k < numberofCacheBlocks; k++){
          std::cout << k << "\t" << CacheMemory[k].age << "\t";
        }

        //ages(CacheMemory, numberofCacheBlocks, cmpair, input.degreesetAssociativity);
        act_hits = pop_cache(CacheMemory, readwrite, mainMemAddress, mainMemBlock, cacheMemSet, cmpair, offsetbits, indexbits, tagbits, numberofCacheBlocks, input.replacementpolicy, input.degreesetAssociativity);
        actual_percentage = actual_hit_rate(act_hits, total);

        firsttable(mainMemAddress, mainMemBlock, cacheMemSet, cmpair, percentage, hits, total, act_hits, actual_percentage);
        secondtable(CacheMemory, numberofCacheBlocks);

        input = {};                     //reset user_struct

        //exit loop conditions
        std::cout << "\nContinue? (y = yes, n = no): ";
        std::cin >> user_loop_exit;
        if (user_loop_exit == 'n') {
            exit_loop = true;
            break;
        }
    }
}
