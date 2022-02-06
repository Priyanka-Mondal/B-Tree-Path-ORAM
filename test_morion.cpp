#include "morion/Orion.h"
#include<string.h>
#include<utility>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include<limits.h>
#include<set>

using namespace std;


int fileid = 1;
bool usehdd = false;
Orion orion(usehdd, 600000);  


bool BothAreSpaces(char lhs, char rhs) 
{ 
	return (lhs == rhs) && (lhs == ' '); 
}
/*
vector<string> getUniquedWords(vector<string> kws)
{
    vector<string> kw;
    map<string, int> mp;
    string word;
    for(auto word : kws)
    {
	    if(word.size()<=12 && neg.find(word)!=neg.end())
	    {
    		    if ((!mp.count(word)) && (word.size()<=12))
    		    {
    		        mp.insert(make_pair(word, 1));
    		    }
    		    else 
    		        mp[word]++;
	    }
    }
    //mp.erase(fileid);
    
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
	cout <<"++++++++++++++++++++++++++++++++"<<endl;
        int str_size = str.length();
	
	if (str_size% sz !=0)
	{
		int pad = ceil(str_size/sz)+1;
		pad = pad*sz-str_size;
	        str.insert(str.size(), pad, '#');
	}
         
	str_size = str.length();
  	int i;
        vector<string> result;
        string temp="";
        
	for (i = 0; i < str_size; i++) {
            if (i % sz == 0) {
		  if(i!=0)
                  {
		     string ttemp =""; //id
		     ttemp.append(temp); 
                     result.push_back(ttemp);    
                  }
                  temp="";
            }
	    temp +=str[i];
	}
        string ttemp = "";//id;
	ttemp.append(temp);	
	result.push_back(ttemp); 
	return result;
}
*/

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
              //printf ("%s/%s\n", dir_name, d_name);
	      string file = dir_name;
	      file = file.append("/");
	      file = file.append(d_name);
	      vector<string> kws1, kws;
	      //string id = toS(fileid);
	      string cont = getFileContent(file);
	      cout <<"=====================================" << endl;
              cout << endl <<file<< " " << fileid <<endl << endl;
	      /*boost::split(kws1, cont, boost::is_any_of(delimiters));
	      kws =  getUniquedWords(kws1);
	      for (auto it = kws.begin(); it != kws.end(); it++)
	      {
		      if(neg.find(*it)!=neg.end())
		      {
			 kws.erase(it--);
		      }
	      }
	      cout << endl <<file<< " " << fileid <<endl << endl;
    		vector<string> blocks;
		blocks = divideString(file,BLOCK);
		orion.insert(kws, blocks, fileid);*/
	        orion.insertWrap(cont,fileid);
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


int main(int, char**) 
{

list_dir("allen-p/deleted_items");
//list_dir("tiny");
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
    		map<int,string> files = orion.search(keyword);
    		for(auto file:files)
		{
    			cout << "["<<file.first<<"] ";
			cout << file.second<< endl<<endl;
		}
    		cout <<endl<<endl;
		cout << "RESULT size: " << files.size() << endl<<endl;
	}
	else if(c=='d'|| c=='D')
	{
		cout <<"Enter file id to be deleted: ";
		int fid;
		cin>>fid;
		//deletefile(fid);
		orion.remove(fid);
	}
	/*else if(c=='i' || c=='I')
	{
		cout << "Enter file name to be inserted:";
		string file;
		cin>> file;
		kwfileblocks(file);
		cout <<endl;
	}
	else if(c=='p' || c=='P')
	{
		foram.print();
	}
	*/
	else //if(c=='q'||c=='Q')
	{
		cout <<"QUITTING..."<<endl<<endl;
		break;
	}
}    
    return 0;
}
