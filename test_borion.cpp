#include "borion/BOrion.h"
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
BOrion borion(usehdd, 3000000);  
set<string> neg;

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
    string delimiters("|?@,:!\"> -./  \n");
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

        /* "Readdir" gets subsequent entries from "d". */
      entry = readdir (d);
      if (! entry) 
      {
            break;
      }
      d_name = entry->d_name;
           /* Print the name of the file and directory. */
          //printf ("%s/%s\n", dir_name, d_name);

           /* If you don't want to print the directories, use the
            *     following line: */

            if (! (entry->d_type & DT_DIR)) {
              printf ("%s/%s\n", dir_name, d_name);
	      string file = dir_name;
	      file = file.append("/");
	      file = file.append(d_name);
	      vector<string> kws1, kws;
	      string id = toS(fileid);
	      string cont = getFileContent(file);
    	      //cout << endl << "FILEID:" << fileid<< endl;
	      //cout << "["<<file <<"]"<< endl;
	      //cout << cont << endl << endl;
	      cout <<"=====================================" << endl;

	      boost::split(kws1, cont, boost::is_any_of(delimiters));
	      kws =  getUniquedWords(kws1, id);
	      int pos = 0;
	      for (auto it = kws.begin(); it != kws.end(); it++)
	      {
		      //cout <<"pos:"<<pos<<"-";
		      if(neg.find(*it)!=neg.end())
		      {
			 //cout << endl <<"Deleted:["<<*it<<"]"<<endl ;
			 kws.erase(it--);
		      }
		      //else
		      //{
			//cout <<"["<<*it<<"]" <<"     " ;
		      //}
		      //pos++;
	      }
	      cout << endl <<file<< " " << id <<endl << endl;
	      cout << "============================" << endl;
    		vector<string> blocks;
		blocks = divideString(file,BLOCK,id);
        	borion.insertWrapper(kws, blocks, id);
                fileid++;
               }



             if (entry->d_type & DT_DIR) {

                /* Check that the directory is not "d" or d's parent. */
                
                if (strcmp (d_name, "..") != 0 &&
                  strcmp (d_name, ".") != 0) {
			cout <<"HERE 1: " << d_name<< endl ;
                 int path_length;
                   char path[PATH_MAX];
                     
                     path_length = snprintf (path, PATH_MAX,
                         "%s/%s", dir_name, d_name);
                       //printf ("%s\n", path);
                         if (path_length >= PATH_MAX) {
				 cout << "Here 2" << endl ;
                             fprintf (stderr, "Path length has got too long.\n");
                               exit (EXIT_FAILURE);
                                 }
                           /* Recursively call "list_dir" with the new path. */
                           list_dir (path);
                            }
                }
             }
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
       fprintf (stderr, "Could not close '%s': %s\n",
          dir_name, strerror (errno));
        exit (EXIT_FAILURE);
        }
}


int main(int, char**) {
   
	/*string inputString("One!Two,Three:Four Five--Six ,  Seven,8.9");
	string delimiters("|,:! -.");
	vector<string> parts;
	boost::split(parts, inputString, boost::is_any_of(delimiters));
	for(auto v : parts)
		cout << v << endl;*/
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


    list_dir("enron");
    cout << endl << "FILEID:" << fileid<< endl;
    //insert a dummy file for fake entries at the end -- maybe not required
    // first searches ids 
    //map<string,string> allfiles = borion.searchWrapper("hell");
    /*
    for(auto itr= allfiles.begin(); itr!=allfiles.end();itr++)
    {
	   	 cout << " OUTPUT Blocks :[" << itr->first << "]\n";
		 cout << itr->second << endl << endl;
    }*/
    
    return 0;
}
