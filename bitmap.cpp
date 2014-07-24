/*
 * BitMap.cpp
 *
 *  Created on: 21.07.2014
 *      Author: kovalevfm
 */

#include "bitmap.h"
#include <iostream>
#include <cmath>
#include <cassert>

static const unsigned char BitsSetTable256[256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

size_t count_bits(uint32_t word){
    unsigned char * p = (unsigned char *) &word;
    return BitsSetTable256[p[0]] +
           BitsSetTable256[p[1]] +
           BitsSetTable256[p[2]] +
           BitsSetTable256[p[3]];
}

const size_t BitMap::block_size = sizeof(uint32_t) * 8;

std::ostream& operator<<(std::ostream &os, const BitMap &bitmap){
    for (size_t pos = 0; pos < bitmap.size(); ++pos) {
        os << (bitmap.get(pos) ? 1 : 0);
    }
    return os;
}

BitMap::BitMap() : bitset_size(0){}

BitMap::BitMap(const BitMap& other) :
        bitset_size(other.bitset_size), array(other.array) {}

BitMap::BitMap(const std::vector<uint32_t>& other_array, size_t other_bitset_size) :
        bitset_size(other_bitset_size), array(other_array) {}


BitMap::BitMap(size_t bitset_size_, bool value) :
        bitset_size(bitset_size_), array((bitset_size_ == 0) ? 0 : (bitset_size_ - 1) / block_size + 1, value ? ~0u : 0){}

BitMap::~BitMap() {}

void BitMap::reset_tail(){
    uint32_t tail_mask = (1u << bitset_size % block_size) - 1;
    if (tail_mask){
        *array.rbegin() = *array.rbegin() & tail_mask;
    }
}

void BitMap::set_tail(bool value){
    uint32_t tail_mask = (1u << bitset_size % block_size) - 1;
    if (tail_mask){
        *array.rbegin() = value ? (*array.rbegin() | ~tail_mask) : *array.rbegin() & tail_mask;
    }
}

void BitMap::set(size_t pos, bool value){
    if (value){
        array[pos/block_size] |= 1u << pos % block_size;
    } else {
        reset(pos);
    }
}

void BitMap::reset(size_t pos){
    array[pos/block_size] &= ~(uint32_t)(1u << pos % block_size);
}

bool BitMap::get(size_t pos) const{
    return (array[pos/block_size] & (1 << pos % block_size)) > 0;
}
void BitMap::set_all(){
    for(auto i = array.begin() ; i != array.end() ; ++i){
        *i = ~0u;
    }
}
void BitMap::reset_all(){
    for(auto i = array.begin() ; i != array.end() ; ++i){
        *i = 0u;
    }
}

size_t BitMap::size() const{
    return bitset_size;
}

size_t BitMap::count() const{
    size_t cnt = 0;
    for(auto i = array.rbegin() ; i != array.rend() ; ++i){
        if (i == array.rbegin() && bitset_size % block_size){
            cnt += count_bits(*i & ((1u << bitset_size % block_size) - 1));
        } else {
            cnt += count_bits(*i);
        }
    }
    return cnt;
}

bool BitMap::is_any() const{
    for(auto i = array.rbegin() ; i != array.rend() ; ++i){
        if (i == array.rbegin() && bitset_size % block_size){
            if (*i & ((1u << bitset_size % block_size) - 1)) return true;
        } else {
            if (*i) return true;
        }
    }
    return false;
}
BitMap& BitMap::reverse(){
    for(auto i = array.begin() ; i != array.end() ; ++i){
        *i = ~*i;
    }
    return *this;
}

void BitMap::resize(size_t bitset_size_, bool value){
    set_tail(value);
    bitset_size = bitset_size_;
    array.resize((bitset_size_ == 0) ? 0 : (bitset_size_ - 1) / block_size + 1, value ? ~0u : 0);
}

void BitMap::clear(){
    bitset_size = 0;
    array.clear();
}

void BitMap::push_back(bool bit){
    resize(bitset_size, bit);
}


BitMap& BitMap::operator =(const BitMap& b){
    bitset_size = b.bitset_size;
    array.resize(b.array.size());
    array.assign(b.array.begin(),b.array.end());
    return *this;
}


BitMap& BitMap::bit_and(const BitMap& b){
    assert(bitset_size == b.bitset_size);
    for (size_t i = 0 ; i < array.size() ; ++ i){
        array[i] &= b.array[i];
    }
    return *this;
}

BitMap& BitMap::bit_or(const BitMap& b){
    assert(bitset_size == b.bitset_size);
    for (size_t i = 0 ; i < array.size() ; ++ i){
        array[i] |= b.array[i];
    }
    return *this;
}
BitMap& BitMap::bit_xor(const BitMap& b){
    assert(bitset_size == b.bitset_size);
    for (size_t i = 0 ; i < array.size() ; ++ i){
        array[i] ^= b.array[i];
    }
    return *this;
}

BitMap& BitMap::operator &=(const BitMap& b){
    return bit_and(b);
}
BitMap& BitMap::operator |=(const BitMap& b){
    return bit_or(b);
}
BitMap& BitMap::operator ^=(const BitMap& b){
    return bit_xor(b);
}

BitMap operator ~(const BitMap& a){
    BitMap result(a);
    return result.reverse();
}
BitMap operator &(const BitMap& a, const BitMap& b){
    BitMap result(a);
    result &= b;
    return result;
}
BitMap operator |(const BitMap& a, const BitMap& b){
    BitMap result(a);
    result |= b;
    return result;
}
BitMap operator ^(const BitMap& a, const BitMap& b){
    BitMap result(a);
    result ^= b;
    return result;
}

BitMap& BitMap::compact(size_t new_size){
    if (new_size == bitset_size){
        return *this;
    }
    BitMap result = this->get_part(0, new_size);
    for (size_t i = new_size ; i < bitset_size ;  i += new_size){
        result |= this->get_part(i, std::min(i+new_size, bitset_size));
    }
    *this = result;
    return *this;
}

BitMap BitMap::get_compact(size_t new_size){
    if (new_size == bitset_size){
        return BitMap(*this);
    }
    BitMap result = this->get_part(0, new_size);
    for (size_t i = new_size ; i < bitset_size ;  i += new_size){
        result |= this->get_part(i, std::min(i+new_size, bitset_size));
    }
    return result;
}

BitMap BitMap::get_part(size_t begin, size_t end) const{
    if (begin >= end){
        return BitMap();    
    }
    std::vector<uint32_t> new_array((end - begin - 1) / block_size + 1, 0);
    size_t block_shift = begin % block_size;
    size_t vector_shift = begin / block_size;
    for (size_t i = 0 ; i < new_array.size() ; ++ i){
        new_array[i] |= ( array[i + vector_shift] >> block_shift);
        if (i + vector_shift + 1 < array.size() && block_shift){
            new_array[i] |= ( array[i + vector_shift + 1] << (block_size - block_shift));
        }
    }
    return BitMap(new_array, end - begin);
}