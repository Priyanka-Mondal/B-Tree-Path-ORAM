#include "baseline/Client.h"
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
int ressize;
//string delimiters("_|+#(){}[]0123456789*?&@=,:!\"><; _-./ \t\n");
map<string,int> kwfreq;
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


string getFileContent(string path)
{
	  ifstream file(path);
	  string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	      return content;
}


static int list_dir (const char * dir_name, Client& client)
{
   DIR * d;
   d = opendir (dir_name);
   if (! d) 
   {
      fprintf (stderr, "Cannot open directory '%s': %s\n",dir_name, strerror (errno));
      exit (EXIT_FAILURE);
   }
   while (1) 
   {
      struct dirent * entry;
      const char * d_name;
      entry = readdir (d);
      if (! entry) 
            break;
      d_name = entry->d_name;
      vector<string> kws1, kws;
      if (! (entry->d_type & DT_DIR)) 
      {
        string file = dir_name;
        file = file.append("/");
        file = file.append(d_name);
        string cont = getFileContent(file);
	//cout <<"searching in file:"<<file<<" :"<< fileid<<endl;
        //boost::split(kws1, cont, boost::is_any_of(delimiters));
	//kws =  getUniquedWords(kws1, fileid);
	client.insertFile(fileid, cont);
	cout <<file << "inserted:"<< fileid<< endl;
  	//if(find(kws.begin(),kws.end(),line)!=kws.end())
	//{
		//cout <<"FOUND in: " << fileid <<endl;
	//	ressize++;
	//}
          fileid++;
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
                           list_dir (path, client);
                            }
                }
             }
    if (closedir (d)) {
       fprintf (stderr, "Could not close '%s': %s\n",
          dir_name, strerror (errno));
        exit (EXIT_FAILURE);
        }

    return ressize;
}

int main(int argc, char** argv) 
{
	if(argc <5)
	{
		cout <<"incorrect arguments! EXITING..."<<endl;
	cout <<"<execFile> <fileDir> <keywordFile> <outputFile> <speed>"<<endl;
		return 0;

	}
	string speed1 = argv[4];
	string unit = "";
	string speed2="";
	speed2.assign(speed1.begin(),speed1.end()-2);
	unit.assign(speed1.end()-2,speed1.end());
	if(unit != "MB")
	{
		cout <<"please give speed in MB"<<endl;
		return 0;
	}
	double speed = double(stoI(speed2));
	speed = speed*1024*1024;
    Server server(false);
    Client client(&server, false, 100,1001);
    ifstream skw;
    skw.open(argv[2]);
    string line;
    list_dir(argv[1], client);
    int l = 1;
    ofstream sres;
    sres.open(argv[3]);
////////////////////////////////////////////////////////////////////////////
 	string first=argv[0];
	int b =1 ;
	while(b<argc)
	{
		first.append(" ");
		first.append(argv[b]);
		b++;
	}
	sres<<first<<endl;
///////////////////////////////////////////////////////////////////////////
	  cout <<endl<<"# | SearchTime | TransferTime | TotalTime | resultSize"<<endl;
    while(getline(skw,line))
    {
        ressize = 0;
    	auto start = high_resolution_clock::now();
	auto s = client.getfreq(line, fileid-1);
    	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<milliseconds>(stop-start);
	double time = double(s.first)/speed;
	time = time*1000;
	int totTime = time+ duration.count();
	sres<<line<<" "<< duration.count()<<" "<<totTime<<" "<<s.second<<endl;
cout<<l<<" | "<<duration.count()<<" | "<<time<<" | "<<totTime<<" | "<<s.second<<endl;
    	l++;
    }
return 0;
}
