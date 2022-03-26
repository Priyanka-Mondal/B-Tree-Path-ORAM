#include "mitra/Client.h"
#include<iostream>
#include<string.h>
#include<utility>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include<limits.h>
#include<set>
#include<chrono>
using namespace std;
using namespace std::chrono;

#define REAL true
#define FAKE false

int fileid = 1;
int uniquekw = 0;
int totentry=0;
string delimiters("_|+#(){}[]0123456789*?&@=,:!\"><; _-./ \t\n");
map<string,int> kwfreq;
std::set<std::string> neg = {"_","a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z","\n","\0", " ", "-","?","from","to", "in","on","so","how""me","you","this","that","ok","no","yes","him","her","they","them","not","none","an","under","below","behind","he","she","their","has","our","would","am","may","know","all","can","any","me","or","as","your","it","we","please","at","if","will","are","by","with","be","com","have","is","of","for","and","the","date","cc","up","but","do","what","which","been","where","could","who","would","did","put","done","too","get","got","yet","co","if"};

bool cmp(pair<string, int>& a, pair<string, int>& b)
{
     return a.second < b.second;
}
void sort(map<string, int>& M)
{
	vector<pair<string, int> > A;
	for (auto& it : M) 
	{
		A.push_back(it);
	}
	sort(A.begin(), A.end(), cmp);
        ofstream kw,freq, both;
	kw.open("keywords");//,ios::app);	
	freq.open("frequency.txt");//,ios::app);	
	both.open("kw-freq.txt");//,ios::app);	
	for (pair<string,int>& it : A) 
	{
		//cout << it.first << ' '<< it.second << endl;
		kw << it.first<<endl;
		freq << it.second<<endl;
		totentry = totentry+it.second;
		both << it.first <<" "<<it.second<<endl;
	}
	cout<< "totentry:"<<totentry<<endl;
}
int stoI(string updt_cnt)
{
        int updc;
        stringstream convstoi(updt_cnt);
        convstoi >> updc;
        return updc;
}
vector<string> getUniquedWords(vector<string> kws, int fileid)
{
    vector<string> kw;
    map<string, int> mp;
    string word;
    for(auto word : kws)
    {
	    if(word.length()<=12)
	    {
       		 if (!mp.count(word))
       		     mp.insert(make_pair(word, 1));
       		 else
       		     mp[word]++;
	    }
    }
    for (map<string, int> :: iterator p = mp.begin();p != mp.end(); p++)
    {
        if (p->second >= 1)
            kw.push_back(p->first) ;
    }
    return kw;
}

vector<string> divideString(string str, int sz)
{
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
	for (i = 0; i < str_size; i++) 
	{
	    if (i % sz == 0) 
	    {
		  if(i!=0)
	          {
		     string ttemp ="";//id;
		     ttemp.append(temp); 
	             result.push_back(ttemp);    
	          }
	          temp="";
	    }
	    temp +=str[i];
	}
	string ttemp = "";
	ttemp.append(temp);	
	result.push_back(ttemp); 
	return result;
}

string getFileContent(string path)
{
	  ifstream file(path);
	  string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	      return content;
}


static void list_dir (const char * dir_name, Client& client, bool real)
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
	      vector<string> kws1, kws;
            if (! (entry->d_type & DT_DIR)) 
	    {
	      string file = dir_name;
	      file = file.append("/");
	      file = file.append(d_name);
	      string cont = getFileContent(file);
	      cout <<"===========================================" << endl;
	      boost::split(kws1, cont, boost::is_any_of(delimiters));
	      kws =  getUniquedWords(kws1, fileid);
	      for (auto it = kws.begin(); it != kws.end(); it++)
	      {
		      if(neg.find(*it)!=neg.end())
		      {
			 kws.erase(it--);
		      }
	      }
	      cout <<file<< " " << fileid <<endl;
			client.insert(kws, fileid,true);
			client.insertFile(fileid,cont,true);
			uniquekw = uniquekw+kws.size();
			cout << "inserted "<< uniquekw <<" unique keywords"<<endl;
                fileid++;
              }
	    /*for(auto word: kws)
	    {
       		 if (!kwfreq.count(word))
       		     kwfreq.insert(make_pair(word, 1));
       		 else
       		     kwfreq[word]++;
	    }*/
             if (entry->d_type & DT_DIR) {
                if (strcmp (d_name, "..") != 0 &&
                  strcmp (d_name, ".") != 0) {
                 int path_length;
                   char path[PATH_MAX];
                     
                     path_length = snprintf (path, PATH_MAX,
                         "%s/%s", dir_name, d_name);
                         if (path_length >= PATH_MAX) {
                             fprintf (stderr, "Path length has got too long.\n");
                               exit (EXIT_FAILURE);
                                 }
                           list_dir (path, client, real);
                            }
                }
             }
    if (closedir (d)) {
       fprintf (stderr, "Could not close '%s': %s\n",
          dir_name, strerror (errno));
        exit (EXIT_FAILURE);
        }
    //return client;
}

void insertSingleFile(Client &client,string file)
{

      string cont = getFileContent(file);
      vector<string> kws1, kws;
      boost::split(kws1, cont, boost::is_any_of(delimiters));
      kws =  getUniquedWords(kws1, fileid);
      for (auto it = kws.begin(); it != kws.end(); it++)
      {
	      if(neg.find(*it)!=neg.end())
	      {
		 kws.erase(it--);
	      }
      }
      cout << endl <<file<< " " << fileid <<endl << endl;
      client.insert(kws, fileid,false);
      client.insertFile(fileid, cont, false);
      fileid++;
}
/*
void deleteSinglefile(Client &client, int fileid)
{
	vector <int> singlefile;
	singlefile.push_back(fileid);
	map<int,string>file = client.searchfile(singlefile);
	
		cout <<"size of files:"<< file.size()<<endl;
	for(auto con : file)// there is one file though
	{
		string cont = con.second;
		cout << cont<< endl;
      		vector<string> kws1, kws;
      		boost::split(kws1, cont, boost::is_any_of(delimiters));
      		kws =  getUniquedWords(kws1, fileid);
     		for (auto it = kws.begin(); it != kws.end(); it++)
      		{
	      		if(neg.find(*it)!=neg.end())
	      		{
		 		kws.erase(it--);
	      		}
      		}
		cout <<"size of keywords:"<< kws.size();
        	 for(auto k: kws)
        	 {
			 cout <<"AT client del:"<< k<< endl;
      			int lenn = k.length();
      			if(lenn <= 12)
      			{
      				client.remove(k, fileid,false);
      			}
        	 }
	 }
}
*/
int main(int argc, char** argv) 
{

    bool usehdd = true, deletFiles = true;
    Server server(usehdd, deletFiles);
    int kwcnt = stoI(argv[1]);
    int filecnt = stoI(argv[2]);
    Client client(&server, deletFiles, kwcnt, filecnt);
    list_dir(argv[3],client, REAL);
    //sort(kwfreq);
    client.endSetup();
        ofstream sres;
	sres.open("mitra.txt");
        ifstream kw;
	kw.open("KEYW/smallkw");
	string line;
	
	while(getline(kw,line))
	{
        	auto start = high_resolution_clock::now();
		auto s = client.search(line);
	    	//	for(auto file:s)
		//	{
	    	//		cout << "["<<file.first<<"] ";
	    			//cout << file.second<<endl;
		//	}
		//		cout <<endl;
        	auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop-start);
		sres <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
		cout <<line<<" "<< duration.count()<<" "<< s.size()<<endl;
	}
	return 0;
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
   			map<int,string> files = client.search(keyword);
			cout <<"--------Search result---------"<<endl;
	    		for(auto file:files)
			{
	    			cout << "["<<file.first<<"] ";
	    			//cout << file.second<<endl;
				cout <<endl;
			}
			cout <<endl<<"RESULT SIZE:"<< files.size()<<endl;
			/*
			  vector<int> forfile;
	    		cout <<endl<<endl;
			cout << "RESULT size: " << files.size() << endl<<endl;
			map<int,string> content = client.searchfile(forfile);
			*/
			//for(auto m: content)
			//{
			//	cout <<"["<<m.first<<"]{"<<m.second<<"}"<<endl;
			//}
		}
		/*else if(c=='d'|| c=='D')
		{
			cout <<"Enter file id to be deleted: ";
			string idstr;
			cin>>idstr;
			bool flag = true;
			int len = idstr.length();
			for(int i=0;i<len;i++)
			{
				if(isdigit(idstr[i])== true)
				{
					continue;
				}
				else 
				{
					flag = false;
					break;
				}
			}
			if(flag == true)
			{
				int id = stoI(idstr);
				deleteSinglefile(client,id);
			}
			else
				cout <<"***invalid id (enter integer)***"<<endl;
		}*/
		else if(c=='i' || c=='I')
		{
			string file;
			cout << "Enter file path to be inserted:";
			cin>> file;
			insertSingleFile(client,file);
			cout <<endl;
		}
		else if(c=='p' || c=='P')
		{
			//mitra.print();
		}
		else if(c=='q'||c=='Q')
		{
			cout <<"QUITTING..."<<endl<<endl;
			break;
		}
		else
			cout <<"INVALID CHOICE!"<<endl;
	}    
    return 0;

}
