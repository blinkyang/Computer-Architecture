#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <fstream>
#include <cmath>

using namespace std;


void test(); ///FUNCTION JUST USED FOR TESTING REMOVE LATER

int intArg(char *arg[]);
int mkMap(int tag, int data);
int write(int tag, int data, int prevMap);
void insert(int tag, int data);
void lookup(int tag, string address);
long long addressNum(string address);
string htob(string addr);
long long get_tag(string address);
long long get_data(string addressBin);
long long LRU();

vector<int> MTag;
unordered_map<int,int> dataMap;
unordered_multimap<int,int> tagMap;
unordered_map<int,int> incMap;
long long maxCacheSize; //8 mB of l3 Cache assuming each address is 32 bit

long long missCnt;  //Count for miss data
long long dataCnt; //Count for amount of data compressed
long long totalCnt; //cnt for total instructions in general
long long totalDataCnt; //cnt for total data in general
int m;




int main(int argc, char * argv[])
{
  long long prevMap = 0;
  float missRate;
  float dataRate;
  string addrLine; //Variable to whole line from trace
  maxCacheSize = intArg(&argv[1])*100;
  m = intArg(&argv[2])*10;
  cout << "cacheSize (Bytes): " << maxCacheSize*32/8 << endl;

  ifstream infile;

  infile.open("trace.din");
  //infile.open("temp.trace");

      if(!infile) {
          cout << "Unable to open file";
          exit(1); // terminate with error
        }



while(getline(infile,addrLine))         //START OF WHILE LOOP FOR ALL LINES IN INSTRUCTION
{

  string address = addrLine.substr(2,8);
  string binAddr = htob(address);
  char choice = addrLine[0];
  long long tag = get_tag(binAddr);
  long long data = get_data(binAddr);

  if (choice == '1')
  {
    write(tag,data,prevMap);
  }
  else
  {
    lookup(tag,address);
  }

}

  //test();


  missRate = float(missCnt)/float(totalCnt)*100;
  dataRate = float(dataCnt)/float(totalDataCnt)*100;
  cout << "Miss rate: " << missRate  << "%" << endl;
  cout << "Percentage of Data reduced: " << dataRate << "%" << endl;


  return 0;
}


int write(int tag, int data, int prevMap) //FUNCTION TO WRITE DATA INTO CACHE
{
  long long map;
  map = mkMap(m, data);
  bool found = false;    //Implemented but kinda not needed honestly, might take out later
  totalDataCnt++;
  totalCnt++;

  if (map == prevMap){ return prevMap;} //Checks to see if prevMap is same, if same, do nothing
  else
  {

    for (auto dataIt = dataMap.begin(); dataIt != dataMap.end(); dataIt++) //Checks to see if map is in data cache, if map is in data cache, attaches tag to that data
    {

      if (dataIt->first == map)
      {
        found == true;
        dataCnt++;
        tagMap.emplace(dataIt->first,tag);
        prevMap = map;
        return prevMap;   //Exits out because no other map should be in data cache

      }
    }
    if (found == false) //IF map is not found, insert tag and data
    {
      insert(tag,data);
    }
  }
  prevMap = map;  //Setting current map as new previous map

return prevMap;
}

void insert(int tag, int data)
{
  missCnt++; //COUNTER FOR MISSES
  long long map;
  map = mkMap(m, data); //FINDING MAP OF ADDRESS
  bool found = false;

    for (auto dataIt = dataMap.begin(); dataIt != dataMap.end(); dataIt++) //For loop to check for Data is already in cache
    {
      if (dataIt->first == map)
      {
        found == true;
        dataCnt++;
        tagMap.insert({dataIt->first,tag});
        return;
      }
    }
    if (found == false) //IF data not found, do the following, again kinda not needed might take out later
    {

      if ((tagMap.bucket_count() + dataMap.bucket_count()) < maxCacheSize)  //If data part of cache is not full, insert data and tag
      {
          //cout << "1: " << tagMap.bucket_count() + dataMap.bucket_count() << endl;
          dataMap.emplace(map,data);
          tagMap.emplace(map,tag);
          incMap.emplace(map,0);

      }
      else if ((tagMap.bucket_count() + dataMap.bucket_count()) > maxCacheSize) //If data part of cache is full, replace
      {
        long long lruMap = LRU();  //LRU MAP (NEEDS TO BE IMPLEMENTED)
        tagMap.erase(lruMap); //Erase all related tags of Data to be removed
        dataMap.erase(lruMap); //Erase Data from part of cacheSize
        incMap.erase(lruMap);
        dataMap.emplace(map,data); //Input new address data and tag into cache
        tagMap.emplace(map,tag);
        incMap.emplace(map,0);
      }

    }
}

void lookup(int tag, string address)
{
  totalCnt++;
  for (auto tagIt = tagMap.begin(); tagIt != tagMap.end(); tagIt++) //For loop to check for tag in tag map
  {
    if (tagIt->second == tag)
    {

      for (auto dataIt = dataMap.begin(); dataIt != dataMap.end(); dataIt++) //For loop to look up data of tag
      {
        if (dataIt->first == tagIt->first)
        {
          int temp = dataIt->first;
          incMap[temp]++;
          //cout << "Look up Found: " << dataIt->second << endl;
          return;
        }
      }

    }
  }
  int data = get_data(htob(address)); //TEMPORARY DATA // NEED TO RETRIEVE DATA FROM ADDRESS // NEED CONVERSION FUNCTION
  insert(tag,data);

}


int mkMap(int m, int data)  //FINDING THE MAP OF CERTAIN ADDRESS (NEEDS TO BE WORKED ON)
{                             //BUT DOPPLEGANGER DOES THIS IN BLOCKS, DO WE WANT TO CREATE BLOCKS?
  float map;
  //map = stoll(to_string(data/2) + to_string(data%10));
  map = stoll(to_string(floor(float(data)/float(m))));

return map;
}

string htob(string addr)
 {
   string result;
   int j = 0;
  string finalString;
   for (int i = 0; i < addr.size(); i++)
   {
     if (addr[i] == '0') { result = "0000";}
     else if (addr[i] == '1') { result = "0001";}
     else if (addr[i] == '2') { result = "0010";}
     else if (addr[i] == '3') { result = "0011";}
     else if (addr[i] == '4') { result = "0100";}
     else if (addr[i] == '5') { result = "0101";}
     else if (addr[i] == '6') { result = "0110";}
     else if (addr[i] == '7') { result = "0111";}
     else if (addr[i] == '8') { result = "1000";}
     else if (addr[i] == '9') { result = "1001";}
     else if (addr[i] == 'a') { result = "1010";}
     else if (addr[i] == 'b') { result = "1011";}
     else if (addr[i] == 'c') { result = "1100";}
     else if (addr[i] == 'd') { result = "1101";}
     else if (addr[i] == 'e') { result = "1110";}
     else if (addr[i] == 'f') { result = "1111";}

    finalString = finalString + result; //adding all the string together to make one long binary

   }

   return finalString;
}


long long get_tag(string addressBin){
  string tag_bin;
  if(addressBin.size() == 24){
    tag_bin = addressBin.substr(0,12);
  }
  else{
    tag_bin = addressBin.substr(0,21);
  }
  long tagNum = stoll(tag_bin,nullptr,2);
  return tagNum;
}

long long get_data(string addressBin){
  string data_bin;

  if(addressBin.size() == 24){
      data_bin = addressBin.substr(13,20);
  }
  else{
      data_bin = addressBin.substr(22,27);
  }
  long long dataNum = stoll(data_bin,nullptr,2);
  return dataNum;
}

int intArg(char *arg[])
{
  int result;
  result = stoi(*arg);
  //cout << result << endl;

return result;
}

long long LRU()
{
  int lowCnt = 50;
  int lowMap;
  for(auto incIt = incMap.begin(); incIt != incMap.end(); incIt++)
  {
    if (incIt->second < lowCnt) {lowMap = incIt->first;}
  }
  return lowMap;
}




void test()   //Loops to see results
{
  cout << endl << "DATA MAP NOW!!" << endl;
  for (auto dataIt = dataMap.begin(); dataIt != dataMap.end(); dataIt++)
  {
    cout << dataIt->first << ": " << dataIt->second << endl;
  }

  cout << endl << "TAG MAP NOW!!" << endl;
  for (auto tagIt = tagMap.begin(); tagIt != tagMap.end(); tagIt++)
  {
    cout << tagIt->first << ": " << tagIt->second << endl;
  }
}
