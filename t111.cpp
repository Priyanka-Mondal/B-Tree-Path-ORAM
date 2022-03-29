#include<iostream>
#include<vector>
#include<map>
#include<unordered_map>
using namespace std;
using pr = pair<int,int>;
int main(int argc, char** argv) 
{
	cout <<"CMPS 111,LAB01E - Day 1"<< endl;
	//create
	vector<int> abc;  
	//insert
	abc.push_back(56);
	abc.push_back(101);
	abc.push_back(99);
	abc.push_back(10);
	abc.reserve(50);
	cout <<"size:"<< abc.size()<<endl;
	cout <<"capacity:"<< abc.capacity()<<endl;
	abc.insert(abc.begin(), 20);
        
	for(int i : abc)
		cout << i<<" ";
	cout <<endl;
	//delete
        abc.pop_back();
	for(int i : abc)
		cout << i<<" ";
	cout <<endl;
	abc.erase(abc.begin()+2);
	//access
	
int x = abc.front();
cout <<"x:"<< x<<endl;
	
	for(int i : abc)
		cout << i<<" ";
	cout <<endl;
	
	
x = abc.back();
cout <<"x:"<< x<<endl;
	
	for(int i : abc)
		cout << i<<" ";
	cout <<endl;
x = abc.at(2);//3rd element
cout <<"x:"<< x<<endl;
	
	for(int i : abc)
		cout << i<<" ";
	cout <<endl;
cout <<"============================="<<endl;
	for(auto it = abc.begin(); it< abc.end(); it++)
		cout << *it << " ";
	cout << endl;
	abc.emplace_back(44);
	cout<<abc.back()<<endl;

	abc.resize(0);
	cout <<"NEW SIZE:"<< abc.size()<<endl;
	abc.push_back(56);
	abc.push_back(101);
	abc.push_back(99);
	abc.push_back(10);
	abc.reserve(50);
	vector<int> def ;
	def.push_back(16);
	def.push_back(17);
	abc.swap(def);

	for(auto it = abc.begin(); it< abc.end(); it++)
		cout << *it << " ";
	cout << endl;
	for(auto it = def.begin(); it< def.end(); it++)
		cout << *it << " ";
	cout << endl;
	def.clear();
	cout <<"NEW SIZE:"<< def.size()<<endl;
	//assign
	def.assign(7,100);
	for(auto it = def.begin(); it< def.end(); it++)
		cout << *it << " ";
	cout << endl;
	cout <<"====================================MAP====================================="<<endl;




	map<int,int> map1; //pairs

	for(int i =1;i<=10;i++)
		map1[i]=i*i;

	for(auto j: map1)
		cout <<j.first <<":"<< j.second<< endl;

map1.insert(make_pair(11,121));


cout <<"---------------------------------------------------"<<endl;
	for(auto j: map1)
		cout <<j.first <<":"<< j.second<< endl;

map<string,int> count;

count.insert(make_pair("orange",6));
count.insert(make_pair("kiwi",4));
count.insert(make_pair("apple",5));
count.insert(make_pair("banana",6));


cout <<"---------------------------------------------------"<<endl;
	for(auto j: count)
		cout <<j.first <<":"<< j.second<< endl;




unordered_map<string,int> ucount;

ucount.insert(make_pair("orange",6));
ucount.insert(make_pair("kiwi",4));
ucount.insert(make_pair("apple",5));
ucount.insert(make_pair("banana",6));


cout <<"---------------------------------------------------"<<endl;
	for(auto j: ucount)
		cout <<j.first <<":"<< j.second<< endl;

//erase swap clear end begin size find count
//

	if(count.count("orange")>0)
		count["orange"]=12;


cout <<"---------------------------------------------------"<<endl;
	for(auto j: count)
		cout <<j.first <<":"<< j.second<< endl;

for(auto it : count)
{
		if(it.first == "orange")
		{
			count.erase(it.first);
			break;
		}
}

cout <<"---------------------------------------------------"<<endl;
	for(auto j: count)
		cout <<j.first <<":"<< j.second<< endl;

count.clear();

cout <<"size of count:"<<count.size()<<endl;

count.insert(make_pair("orange",6));
count.insert(make_pair("kiwi",4));
count.insert(make_pair("apple",5));
count.insert(make_pair("banana",6));

	for(auto it = count.begin(); it != count.end(); it++)
		cout << it->first << ":"<<it->second<<endl;
	cout << endl;













	return 0;
}
