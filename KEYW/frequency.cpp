#include <bits/stdc++.h>
#include<iostream>
#include<string.h>
#include<utility>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include<limits.h>
#include<set>
using namespace std;

map<string,int> kwfreq;
string delimiters("|+#(){}[]0123456789*?&@=,:!\"><; _-./'  \n");

std::set<std::string> stopwords = {
"\n",
"\0", 
" ", 
"-",
"?",
"from",
"to", 
"in",
"on",
"so",
"how"
"me",
"you",
"this",
"that",
"ok",
"no",
"yes",
"him",
"her",
"they",
"them",
"not",
"none",
"an",
"under",
"below",
"behind",
"he",
"she",
"their",
};

bool cmp(pair<string, int>& a, pair<string, int>& b)
{
     return a.second < b.second;
}

string getFileContent(string path)
{
	  ifstream file(path);
	  string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	      return content;
}
void sort(map<string, int>& M)
{
	vector<pair<string, int> > A;
	for (auto& it : M) 
	{
		A.push_back(it);
	}
	sort(A.begin(), A.end(), cmp);
	for (pair<string,int>& it : A) 
	{
		cout << it.first << ' '<< it.second << endl;
	}
}

vector<string> getUniquedWords(vector<string> kws)
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
	      kws =  getUniquedWords(kws1);
	      for (auto it = kws.begin(); it != kws.end(); it++)
	      {
		      if(stopwords.find(*it)!=stopwords.end())
		      {
			 kws.erase(it--);
		      }
	      }
	    }
	    for(auto word: kws)
	    {
       		 if (!kwfreq.count(word))
       		     kwfreq.insert(make_pair(word, 1));
       		 else
       		     kwfreq[word]++;
	    }
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
                           list_dir (path);
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

int main(int argc, char** argv)
{
	list_dir(argv[1]);
	sort(kwfreq);
        ofstream kw,freq;
	kw.open("keywords.txt");//,ios::app);	
	freq.open("frequency.txt");//,ios::app);	
	for(pair<string,int> word:kwfreq)
	{
		kw << word.first;
		freq << word.second;
	}
	return 0;
}
