#include "borion/BOrion.h"
//#include "utils/Utilities.h"
#include<string.h>
#include<utility>

using namespace std;



vector<string> getUniquedWords(string filename, string fileid)
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
    mp.erase(fileid);
  
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

vector<string> divideString(string filename, int sz, string id)
    {
	 fstream fs(filename); 
         string str((istreambuf_iterator<char>(fs)),
                       (istreambuf_iterator<char>()));

        int str_size = str.length();

	
	if (str_size% sz !=0)
	{
		int pad = ceil(str_size/sz)+1;
		//cout << filename<< " pad:" << pad << endl;
		pad = pad*sz-str_size;
		//cout << "again pad:" << pad << endl; 
	        str.insert(str.size(), pad, '#');
	}/*
	else if (str_size <= MEDIUM)
	{
		cout << "Its a MEDIUM file:" << MEDIUM;
        str.insert(str.size(), MEDIUM - str.size(), '#');
	}
	else 
	{
		cout << "Its a LARGE file:" << LARGE;
        str.insert(str.size(), LARGE - str.size(), '#');
	} */
         
	str_size = str.length();
	//cout << "the new size of the file:" <<str_size << endl;
	//cout << str << endl << endl ;
  	int i;
        vector<string> result;
        string temp="";
        
	for (i = 0; i < str_size; i++) {
            if (i % sz == 0) {
		  if(i!=0)
                  {
		     string ttemp =id;
		     ttemp.append(temp); 
                     result.push_back(ttemp);    
		     //cout << "New Node:[" << ttemp << "]\n";
                  }
                  temp="";
            }
	    temp +=str[i];
	}
        string ttemp = id;
	ttemp.append(temp);	
	    //cout << "New Node::"<< ttemp << endl; 
	result.push_back(ttemp); 
	
	return result;
    }

int main(int, char**) {
    
    bool usehdd = false;
    BOrion borion(usehdd, 2048000);  
    // This 4*max-size does not have effect, was able to insert a lot more elements
    //cout << borion.search("test1").size() << endl;
    //borion.remove("test1", 1);
    //for(int i=0;i<=8;i++)
    //	cout << borion.search("test1")[i] << endl;
    vector<string> kw;
    vector<string> blocks;
    
    kw = getUniquedWords("test2.txt","0002");
    //for (auto i: kw) 
    //    cout << "kws: " << i << "\n";
    blocks = divideString("test2.txt",BLOCK-4,"0002");
    int bcnt = 0;
     //for (auto i: blocks)
     //{
     //   bcnt++;
     //   cout <<bcnt << "input blocks: [" << i << "]\n";
     //}
    //The file ids are always 4bytes.
    borion.insertWrapper(kw, blocks, "0002");
    
    kw = getUniquedWords("test1.txt","0001");
    //for (auto i: kw) 
    //    cout << "kws: " << i << "\n";
    blocks = divideString("test1.txt",BLOCK-4,"0001");
    borion.insertWrapper(kw, blocks, "0001");

    kw = getUniquedWords("test5.txt", "0005");
    blocks = divideString("test5.txt",BLOCK-4,"0005");
    borion.insertWrapper(kw, blocks, "0005");
    
    kw = getUniquedWords("test6.txt","0006");
    blocks = divideString("test6.txt",BLOCK-4,"0006");
    borion.insertWrapper(kw, blocks, "0006");

    kw = getUniquedWords("test7.txt","0007");
    blocks = divideString("test7.txt",BLOCK-4,"0007");
    borion.insertWrapper(kw, blocks, "0007");

    kw = getUniquedWords("test8.txt","0008");
    blocks = divideString("test8.txt",BLOCK-4,"0008");
    borion.insertWrapper(kw, blocks, "0008");

     kw = getUniquedWords("test9.txt","0009");
     blocks = divideString("test9.txt",BLOCK-4,"0009");
     borion.insertWrapper(kw, blocks, "0009");

     kw = getUniquedWords("test3.txt","0003");
     blocks = divideString("test3.txt",BLOCK-4,"0003");
     borion.insertWrapper(kw, blocks, "0003");

     //kw = getUniquedWords("test4.txt","0004");
     //blocks = divideString("test4.txt",BLOCK-4,"0004");
     //borion.insertWrapper(kw, blocks, "0004");

    for (int k = 10; k<=19; k++)
    {
	string ks=to_string(k);
	string fl = "test";
        fl.append(ks);
        fl.append(".txt");
 
        string id = "00";
        id.append(ks);
        kw = getUniquedWords(fl,id);
        blocks = divideString(fl,BLOCK,id);
        borion.insertWrapper(kw, blocks, id);    
    }
    //insert a dummy file for fake entries at the end -- maybe not required
    // first searches ids 
    map<string,string> allfiles = borion.searchWrapper("hell");
    
    for(auto itr= allfiles.begin(); itr!=allfiles.end();itr++)
    {
	   	 cout << " OUTPUT Blocks :[" << itr->first << "]\n";
		 cout << itr->second << endl << endl;
    }
    
    return 0;
}
