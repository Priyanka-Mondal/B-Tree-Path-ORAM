#include "borion/BOrion.h"
//#include "utils/Utilities.h"
#include<string.h>
#include<utility>

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
	//	cout << "Its a SMALL file:" << str_size << " " << SMALL - str.size() << "\n" ;
        str.insert(str.size(), SMALL - str.size(), '#');
	}
	else if (str_size <= MEDIUM)
	{
		cout << "Its a MEDIUM file:" << MEDIUM;
        str.insert(str.size(), MEDIUM - str.size(), '#');
	}
	else 
	{
		cout << "Its a LARGE file:" << LARGE;
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
    pair<int , std::array< byte_t, 64>> val;
    val.first = 100;
    string sec = "byte me";
    std::fill(val.second.begin(), val.second.end(), 0);
    std::copy(sec.begin(), sec.end(), val.second.begin());
    string res ="";
   res.assign(val.second.begin(), val.second.end());
   res = res.c_str(); 
   cout << "pair :[" << val.first <<"]["<< res << "]" <<endl;


    bool usehdd = false;
    BOrion borion(usehdd, 1024);  // This 4*max-size does not have effect, was able to insert a lot more elements
    /*
    borion.insert("test1", "1");
    borion.insert("test1", "2");
    borion.insert("test1", "3");
    borion.insert("test1", "4");
    borion.insert("test1", "5");
    borion.insert("test1", "6");
    borion.insert("test1", "7");
    borion.insert("test1", "8");
    borion.insert("test1", "9");
    */
    //cout << borion.search("test1").size() << endl;
    //borion.remove("test1", 1);
    //for(int i=0;i<=8;i++)
    //	cout << borion.search("test1")[i] << endl;
    vector<string> kw;
    vector<string> blocks;
    
    kw = getUniquedWords("test2.txt");
    //for (auto i: kw) 
    //    cout << "kws: " << i << "\n";
    blocks = divideString("test2.txt",BLOCK);
    int bcnt = 0;
     //for (auto i: blocks)
     //{
     //   bcnt++;
     //   cout <<bcnt << "input blocks: [" << i << "]\n";
     //}
    //The file ids are always 4bytes.
    borion.insertWrapper(kw, blocks, "0002");
    
    kw = getUniquedWords("test1.txt");
    //for (auto i: kw) 
    //    cout << "kws: " << i << "\n";
    blocks = divideString("test1.txt",BLOCK);
    borion.insertWrapper(kw, blocks, "0001");

    kw = getUniquedWords("test5.txt");
    blocks = divideString("test5.txt",BLOCK);
    borion.insertWrapper(kw, blocks, "0005");
    
    kw = getUniquedWords("test6.txt");
    blocks = divideString("test6.txt",BLOCK);
    borion.insertWrapper(kw, blocks, "0006");

    kw = getUniquedWords("test7.txt");
    blocks = divideString("test7.txt",BLOCK);
    borion.insertWrapper(kw, blocks, "0007");

    kw = getUniquedWords("test8.txt");
    blocks = divideString("test8.txt",BLOCK);
    borion.insertWrapper(kw, blocks, "0008");

     kw = getUniquedWords("test9.txt");
     blocks = divideString("test9.txt",BLOCK);
     borion.insertWrapper(kw, blocks, "0009");

    for (int k = 10; k<=19; k++)
    {
	string ks=to_string(k);
	string fl = "test";
        fl.append(ks);
        fl.append(".txt");

        kw = getUniquedWords(fl);
    blocks = divideString(fl,BLOCK);
    string id = "00";
	    id.append(ks);
    borion.insertWrapper(kw, blocks, id);    
    }
	 //
    // first searches ids 
    vector<pair<int,string>> allid = borion.searchWrapper("friend");
    /*
    for(auto id : allid)
    {
	    bcnt=0;
	    // then searches for blocks for all files one by one
	    vector<string> blocks = borion.search(id);
	    for(auto blk: blocks)
	    {
		 bcnt++;
	   	 cout << bcnt <<" OUTPUT Blocks :[" << blk << "]\n";
	    }
    }
    */
    return 0;
}
