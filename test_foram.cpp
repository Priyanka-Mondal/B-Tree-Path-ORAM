#include "foram/Foram.h"
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
Foram foram(usehdd, 100, 2);  
set<string> neg;
string delimiters("|+#*?@,:!\"><; _-./  \n");


vector<string> getUniquedWords(vector<string> kws, string fileid)
{
    vector<string> kw;
    map<string, int> mp;
    string word;
    for(auto word : kws)
    {
        if (!mp.count(word))
            mp.insert(make_pair(word, 1));
        else
            mp[word]++;
    }
    mp.erase(fileid);
  
    for (map<string, int> :: iterator p = mp.begin();
         p != mp.end(); p++)
    {
        if (p->second >= 1)
            kw.push_back(p->first) ;
    }
return kw;
}

vector<string> divideString(string filename, int blk, string id)
    {
	 int sz = blk-FID_SIZE; // ID size is 4
	 fstream fs(filename); 
         string str((istreambuf_iterator<char>(fs)),
                       (istreambuf_iterator<char>()));
        int str_size = str.length();
	if (str_size% sz !=0)
	{
		int pad = ceil(str_size/sz)+1;
		pad = pad*sz-str_size;
	        str.insert(str.size(), pad, '#');
	}
  	int i;
        vector<string> result;
        string temp="";
         
	for (i = 0; i < str.length(); i++) {
            if (i % sz == 0) {
		  if(i!=0)
                  {
		     string ttemp =id;
		     ttemp.append(temp); 
                     result.push_back(ttemp);    
                  }
                  temp="";
            }
	    temp +=str[i];
	}
        string ttemp = id;
	ttemp.append(temp);	
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
	  if(file.good())
	  {
	  string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	      return content;
	  }
	  else
	      return "";
}


static void insert_dir (const char * dir_name)
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
	      int pos = 0;
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
		blocks = divideString(file,BLOCK,id);
        	foram.insert(kws, blocks, id);
		cout << "number of keywords :" << kws.size() <<endl;
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
                           insert_dir (path);
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


void deletefile(string id)
{
	string cont = foram.removefileblock(id);
	cout << cont << endl;
	if(cont != "")
	{
		cout <<endl;
		vector<string> kws ,kws1;
		boost::split(kws1, cont, boost::is_any_of(delimiters));
		kws =  getUniquedWords(kws1, id);
		      for (auto it = kws.begin(); it != kws.end(); it++)
		      {
			      if(neg.find(*it)!=neg.end())
			      {
				 kws.erase(it--);
			      }
		      }
		cout << "Deletion of keywords in :"<< id<< endl;
		foram.removekw(kws,id);
	}
	else
	{
	cout <<endl<<endl<<"FILE" << id <<" does NOT exist!!" << endl << endl;
	}
}

void kwfileblocks(string file)
{
	vector<string> kws, kws1, blocks;
	string cont = getFileContent(file);
	cout <<"==================================="<< endl;
	if(cont != "")
	{
	string id = toS(fileid);
	boost::split(kws1, cont, boost::is_any_of(delimiters));
	kws = getUniquedWords(kws1, id);
	for(auto it = kws.begin(); it != kws.end(); it++)
	{
		if(neg.find(*it)!=neg.end())
		{
			kws.erase(it--);
		}
	}
	cout << endl <<"Insertion of "<< file<<" "<< id << endl;
	blocks = divideString(file,BLOCK,id);
	fileid++;
	foram.insert(kws, blocks, id);
	cout <<"=================================="<<endl;
	}
	else 
		cout <<endl<<endl<<"NO SUCH FILE TO INSERT "<<file<<endl;
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
neg.insert("_");
neg.insert("*");
neg.insert("<");
neg.insert(">");
neg.insert("#");
neg.insert("+");
neg.insert("");

//INSERT keywords and file blocks of Enron
    //insert_dir("enron");
    insert_dir("tiny2");

//***NOW TEST search and delete

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
		map <string,string> files = foram.search(keyword);
		for (map<string, string> :: iterator p = files.begin();
		         p != files.end(); p++)
		{
			cout << "FILE[" << p->first << "]";
		}
		cout << endl;
		cout << "RESULT size: " << files.size() << endl;
	}
	else if(c=='i' || c=='I')
	{
		cout << "Enter file name to be inserted:";
		string file;
		cin>> file;
		kwfileblocks(file);
		cout <<endl;
	}
	else if(c=='d'|| c=='D')
	{
		cout <<"Enter file id to be deleted: ";
		string fid;
		cin>>fid;
		deletefile(fid);
	}
	else if(c=='p' || c=='P')
	{
		foram.print();
	}
	else //if(c=='q'||c=='Q')
	{
		cout <<"QUITTING..."<<endl<<endl;
		break;
	}
}
/*
cout << "DELETE STARTS***" << endl;
deletefile("0001");
files.clear();
files = foram.search("3");
cout << "RESULT size2: " << files.size() << endl;
for (map<string, string> :: iterator p = files.begin();
		         p != files.end(); p++)
{
	cout << "FILE[" << p->first << "]";
}
cout << endl;
deletefile("0002");
files.clear();
files = foram.search("3");
cout << "RESULT size2: " << files.size() << endl;
for (map<string, string> :: iterator p = files.begin();
		         p != files.end(); p++)
{
	cout << "FILE[" << p->first << "]";
}
cout << endl;
deletefile("0003");
files.clear();
files = foram.search("3");
cout << "RESULT size2: " << files.size() << endl;
for (map<string, string> :: iterator p = files.begin();
		         p != files.end(); p++)
{
	cout << "FILE[" << p->first << "]";
}
cout << endl;
//gives error
deletefile("0004");
files.clear();
files = foram.search("3");
cout << "RESULT size2: " << files.size() << endl;
for (map<string, string> :: iterator p = files.begin();
		         p != files.end(); p++)
{
	cout << "FILE[" << p->first << "]";
}
cout << endl;
*/
    
    return 0;
}
