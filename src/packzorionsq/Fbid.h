#pragma once
#ifndef BFID_H
#define BFID_H
#include "Types.hpp"
#include <iostream>
#include <array>
using namespace std;

class Fbid {
public:
    std::array< byte_t, FID_SIZE> id;
    Fbid();
    Fbid(int value);
    Fbid(std::array< byte_t, FID_SIZE> value);
    Fbid(string value);
    virtual ~Fbid();
    Fbid operator++ ();
    Fbid& operator=(int other);
    bool operator!=(const int rhs) const ;
    bool operator!=(const Fbid rhs) const ;
    bool operator==(const int rhs)const ;
    bool operator==(const Fbid rhs)const ;
    Fbid& operator=(std::vector<byte_t> other);
    bool operator<(const Fbid& b) const ;
    bool operator>(const Fbid& b) const ;
    bool operator<=(const Fbid& b) const ;
    bool operator>=(const Fbid& b) const ;    
    friend ostream& operator<<(ostream &o, Fbid& id);
};



#endif /* BFID_H */

