#include<iostream>
#include<vector>
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
	for(auto it = abc.begin(); it<= abc.end(); it++)
		cout << *it << " ";
	cout << endl;
	abc.emplace_back(44);
	cout<<abc.back();

	//resize
	//swap
	//clear
	//assign
	return 0;
}
