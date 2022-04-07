#include <algorithm>

#include "Fbid.h"

Fbid::Fbid() {
    std::fill(id.begin(),id.end(),0);
}

Fbid::Fbid(string value) {
    std::fill(id.begin(),id.end(),0);
    std::copy(value.begin(), value.end(), id.begin());
}

Fbid::~Fbid() {
}

Fbid::Fbid(int value) {
    std::fill(id.begin(),id.end(),0);
    auto arr = to_bytes(value);
    std::copy(arr.begin(), arr.end(), id.begin());
}

Fbid::Fbid(std::array<byte_t, FID_SIZE> value) {
    std::copy(value.begin(), value.end(), id.begin());
}

Fbid Fbid::operator++() {
    id[FID_SIZE - 1]++;
    for (int i = FID_SIZE - 1; i > 0; i--) {
        if (id[i] == 0) {
            id[i - 1]++;
        } else {
            break;
        }
    }
}

Fbid& Fbid::operator=(int other) {
    for (int i = 0; i < 4; i++) {
        id[3 - i] = (other >> (i * 8));
    }
    std::fill(id.begin()+4,id.end(),0);
}

bool Fbid::operator!=(const int rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[3 - i] != (rhs >> (i * 8))) {
            return true;
        }
    }
    return false;
}

bool Fbid::operator!=(const Fbid rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[i] != rhs.id[i]) {
            return true;
        }
    }
    return false;
}

bool Fbid::operator<(const Fbid& b)const {
    for (int i = 0; i < FID_SIZE; i++) {
        if (id[i] < b.id[i]) {
            return true;
        } else if (id[i] > b.id[i]) {
            return false;
        }
    }
    return false;
}

bool Fbid::operator<=(const Fbid& b)const {
    for (int i = 0; i < FID_SIZE; i++) {
        if (id[i] < b.id[i]) {
            return true;
        } else if (id[i] > b.id[i]) {
            return false;
        }
    }
    return true;
}

bool Fbid::operator>(const Fbid& b)const {
    for (int i = 0; i < FID_SIZE; i++) {
        if (id[i] > b.id[i]) {
            return true;
        } else if (id[i] < b.id[i]) {
            return false;
        }
    }
    return false;
}

bool Fbid::operator>=(const Fbid& b)const {
    for (int i = 0; i < FID_SIZE; i++) {
        if (id[i] > b.id[i]) {
            return true;
        } else if (id[i] < b.id[i]) {
            return false;
        }
    }
    return true;
}

bool Fbid::operator==(const int rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[3 - i] != (rhs >> (i * 8))) {
            return false;
        }
    }
    return true;
}

bool Fbid::operator==(const Fbid rhs) const {
    for (int i = 0; i < FID_SIZE; i++) {
        if (id[i] != rhs.id[i]) {
            return false;
        }
    }
    return true;
}

Fbid& Fbid::operator=(std::vector<byte_t> other) {
    for (int i = 0; i < FID_SIZE; i++) {
        id[i] = other[i];
    }
}

ostream& operator<<(ostream &o, Fbid& bid) {
    o <<"[";
    for(int i=0;i<FID_SIZE;i++){
    o << (char)bid.id[i];
    }
    o<<"] ";
    return o;
}
