#include "orion/Orion.h"
//#include "utils/Utilities.h"
#include<string.h>
#include<utility>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include<limits.h>
#include<set>

using namespace std;


int fileid = 1;
bool usehdd = false;
Orion orion(usehdd, 1000000);  
set<string> neg;
int totk = 0;
vector<string> getUniquedWords(vector<string> kws, string fileid)
{
    // Open a file stream
    vector<string> kw;
    // Create a map to store count of all words
    map<string, int> mp;
  
    // Keep reading words while there are words to read
    string word;
    for(auto word : kws)
    {
        // If this is first occurrence of word
        if (!mp.count(word))
            mp.insert(make_pair(word, 1));
        else
            mp[word]++;
    }
    mp.erase(fileid);
  
  
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


string toS(int id)
{
	string s = to_string(id);
	string front ="";
	if (id < 10)
		front = "000";
	else if(id < 100)
		front = "00";
	else if(id < 1000)
		front = "0";
	s=front.append(s);

	return s;
}

string getFileContent(string path)
{
	  ifstream file(path);
	  string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	      return content;
}


static void list_dir (const char * dir_name)
{
    string delimiters("|?@,:!\">; -./  \n");
    DIR * d;
    d = opendir (dir_name);

   if (! d) 
   {
      fprintf (stderr, "Cannot open directory '%s': %s\n",
         dir_name, strerror (errno));
       exit (EXIT_FAILURE);
   }
   while (1) 
   {
      struct dirent * entry;
      const char * d_name;
      entry = readdir (d);
      if (! entry) 
      {
            break;
      }
      d_name = entry->d_name;
            if (! (entry->d_type & DT_DIR)) {
              printf ("%s/%s\n", dir_name, d_name);
	      string file = dir_name;
	      file = file.append("/");
	      file = file.append(d_name);
	      vector<string> kws1, kws;
	      string id = toS(fileid);
	      string cont = getFileContent(file);
	      cout <<"=====================================" << endl;

	      boost::split(kws1, cont, boost::is_any_of(delimiters));
	      kws =  getUniquedWords(kws1, id);
	      for (auto it = kws.begin(); it != kws.end(); it++)
	      {
		      if(neg.find(*it)!=neg.end())
		      {
			 kws.erase(it--);
		      }
	      }
	      cout << endl <<file<< " " << id <<endl << endl;
	      cout << "============================" << endl;
    		vector<string> blocks;
				orion.beginSetup();
		for(auto k: kws)
		{
			int lenn = k.length();
			if(lenn <= 12)
			{
				orion.setupInsert(k, fileid);
	        		totk++;
			}
			else
			{
				cout << "GREATER THAN 12"<< endl;
			}
		}
		orion.endSetup();
                fileid++;
               }
             if (entry->d_type & DT_DIR) {
                if (strcmp (d_name, "..") != 0 &&
                  strcmp (d_name, ".") != 0) {
			cout <<"HERE 1: " << d_name<< endl ;
                 int path_length;
                   char path[PATH_MAX];
                     
                     path_length = snprintf (path, PATH_MAX,
                         "%s/%s", dir_name, d_name);
                         if (path_length >= PATH_MAX) {
                             fprintf (stderr, "Path length has got too long.\n");
                               exit (EXIT_FAILURE);
                                 }
                           list_dir (path);
                            }
                }
             }
    if (closedir (d)) {
       fprintf (stderr, "Could not close '%s': %s\n",
          dir_name, strerror (errno));
        exit (EXIT_FAILURE);
        }
}


int main(int, char**) {
neg.insert("and");
neg.insert("the");
neg.insert("The");
neg.insert("a");
neg.insert("A");
neg.insert("an");
neg.insert("An");
neg.insert("to");
neg.insert("To");
neg.insert("in");
neg.insert("of");
neg.insert("or");
neg.insert("as");
neg.insert("for");
neg.insert("on");
neg.insert(",");
neg.insert(" ");
neg.insert("\n");
neg.insert("\0");
neg.insert("?");
neg.insert("by");
neg.insert("\t");
neg.insert("from");


	//list_dir("allen-p/deleted_items");
	//list_dir("allen-p");
	list_dir("tiny");
	cout << endl<<" SETUP INSERT DONE!"<< endl;
	cout <<"=================================="<< endl;
	cout <<"READY TO PERFORM QUERIES!" << endl;
	
	
	while(1)
	{
		char c;
		cout <<endl<<endl<<endl;
		cout <<"Enter your choice (s/i/d/p/q): "<<endl;
		cout <<"s/S: Search"<<endl;
		cout <<"i/I: Insert"<<endl;
		cout <<"d/D: Delete"<<endl;
		cout <<"p/P: Print"<<endl;
		cout <<"q/Q: Quit"<<endl;
		cout <<"------------------"<<endl;
		cout <<"_";
	
		cin >> c;
	
		if(c=='s' || c=='S')
		{
			cout << "Enter the keyword to be searched: ";
			string keyword;
			cin>> keyword;
	    		vector<int> files = orion.searchsimple(keyword);
			cout <<"--------Search result---------"<<endl;
	    		for(auto file:files)
			{
	    			cout << "["<<file<<"] ";
				//cout << file.second<< endl<<endl;
			}
	    		cout <<endl<<endl;
			cout << "RESULT size: " << files.size() << endl<<endl;
		}
		else if(c=='d'|| c=='D')
		{
			cout <<"Enter file id to be deleted: ";
			int fid;
			cin>>fid;
			//orion.remove(fid);
		}
		else if(c=='i' || c=='I')
		{
			cout << "Enter keyword to be inserted:";
			string kw;
			cin>> kw;
			cout << "Enter id to be inserted:";
			int id;
			cin>> id;
			orion.beginSetup();
			orion.setupInsert(kw,id);
			orion.endSetup();
			cout <<endl;
		}
		else if(c=='p' || c=='P')
		{
			//orion.print();
		}
		else //if(c=='q'||c=='Q')
		{
			cout <<"QUITTING..."<<endl<<endl;
			break;
		}
	}    
        return 0;
}
