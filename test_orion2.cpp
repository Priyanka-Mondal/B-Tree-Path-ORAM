#include "orion2/Orion2.h"
//#include "utils/Utilities.h"

using namespace std;



vector<string> getUniquedWords(string filename)
{
    // Open a file stream
    fstream fs(filename);
    vector<string> kw;
    // Create a map to store count of all words
    map<string, int> mp;
  
    // Keep reading words while there are words to read
    string word;
    while (fs >> word)
    {
        // If this is first occurrence of word
        if (!mp.count(word))
            mp.insert(make_pair(word, 1));
        else
            mp[word]++;
    }
  
    fs.close();
  
    // Traverse map and print all words whose count
    //is 1
    for (map<string, int> :: iterator p = mp.begin();
         p != mp.end(); p++)
    {
        if (p->second >= 1)
            kw.push_back(p->first) ;
    }
return kw;
}

vector<string> divideString(string filename, int sz)
    {
	 fstream fs(filename); 
         string str((istreambuf_iterator<char>(fs)),
                       (istreambuf_iterator<char>()));

        int str_size = str.length();
	if (str_size<= SMALL)
	{
		cout << "Its a SMALL file:" << SMALL << "\n" ;
        str.insert(str.size(), SMALL - str.size(), '#');
	}
	else if (str_size <= MEDIUM)
	{
		cout << "Its a MEDIUM file:" << MEDIUM <<"\n";
        str.insert(str.size(), MEDIUM - str.size(), '#');
	}
	else 
	{
		cout << "Its a LARGE file:" << LARGE << "\n";
        str.insert(str.size(), LARGE - str.size(), '#');
	}
        
	str_size = str.length();
  	int i;
        vector<string> result;
        string temp="";
        
	for (i = 0; i < str_size; i++) {
            if (i % sz == 0) {
		  if(i!=0)
                  { 
                     result.push_back(temp);    
		     //cout << "PUSHING:[" << temp << "]\n";
                  }
                  temp="";
            }
	    temp +=str[i];
	}
	result.push_back(temp); 
	
	return result;
    }

int main(int, char**) {
    bool usehdd = false;
    Orion2 orion2(usehdd, 500000, 1024000);  // This 4*max-size does not have effect, was able to insert a lot more elements
    vector<string> kw;
    vector<string> blocks;
    
    kw = getUniquedWords("test1.txt");
    for (auto i: kw) 
    {
    //    cout << "kws: " << i << "\n";
        orion2.insert(i,1);
    }
    blocks = divideString("test1.txt",BLOCK);
    int bcnt = 0;
    //for (auto i: blocks)
    //{
    //   bcnt++;
    //   cout <<bcnt << "blocks to insert:[" << i << "]\n";
    //}
     	orion2.insertFile(1,blocks);
    
    kw = getUniquedWords("test2.txt");
    for (auto i: kw) 
    {
        //cout << "kws: " << i << "\n";
	orion2.insert(i,2);
    }
    blocks = divideString("test2.txt",BLOCK);
    //for(auto i : blocks)
    //{
	    orion2.insertFile(2,blocks);
    //}
    

    kw = getUniquedWords("test3.txt");
    for (auto i: kw) 
    {
        //cout << "kws: " << i << "\n";
	orion2.insert(i,3);
    }
    blocks = divideString("test3.txt",BLOCK);
    //for(auto i : blocks)
    //{
	    orion2.insertFile(3,blocks);
    //}
    kw = getUniquedWords("test4.txt");
    for (auto i: kw) 
    {
        //cout << "kws: " << i << "\n";
	orion2.insert(i,4);
    }
    blocks = divideString("test4.txt",BLOCK);
    //for(auto i : blocks)
    //{
	    orion2.insertFile(4,blocks);
    //}
    // first searches ids 
    vector<int> allid = orion2.search("key");
    for(auto id : allid)
    {
	    bcnt=0;
	    // then searches for blocks for all files one by one
	    vector<string> blocks = orion2.searchFile(id);
	    for(auto blk: blocks)
	    {
		 bcnt++;
	   	 cout <<"File:"<< id <<"/" <<bcnt <<" Blocks:[" << blk << "]\n";
	    }
	    cout <<"\n\n\n\n";
    }
    return 0;
}
