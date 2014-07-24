/*
 * bitmap.h
 *
 *  Created on: 21.07.2014
 *      Author: kovalevfm
 */

#pragma once

#include <vector>
#include <ostream>
#include <cstdint>
#include <msgpack.hpp>



class BitMap {
    friend std::ostream& operator<<(std::ostream &os, const BitMap &bitmap);
    friend BitMap operator ~(const BitMap& a);
    friend BitMap operator &(const BitMap& a, const BitMap& b);
    friend BitMap operator |(const BitMap& a, const BitMap& b);
    friend BitMap operator ^(const BitMap& a, const BitMap& b);

public:

    BitMap();
    BitMap(size_t bitset_size_, bool value = false);
    BitMap(const BitMap& other);
    BitMap(const std::vector<uint32_t>& other_array, size_t other_bitset_size);
    virtual ~BitMap();

    void set(size_t pos, bool value = true);
    void reset(size_t pos);
    bool get(size_t pos) const;
    void set_all();
    void reset_all();

    size_t count() const;
    bool is_any() const;
    size_t size() const;

    void resize(size_t num_bits, bool value = false);
    void clear();
    void push_back(bool bit);

    BitMap& reverse();
    BitMap& bit_and(const BitMap& b);
    BitMap& bit_or(const BitMap& b);
    BitMap& bit_xor(const BitMap& b);

    BitMap& operator &=(const BitMap& b);
    BitMap& operator |=(const BitMap& b);
    BitMap& operator ^=(const BitMap& b);
    BitMap& operator =(const BitMap& b);

    BitMap& compact(size_t new_size);
    BitMap get_compact(size_t new_size);
    BitMap get_part(size_t begin, size_t end) const;

    std::vector<uint32_t>& data(){
        return array;
    }

private:
    void reset_tail();
    void set_tail(bool value = true);

private:
    static const size_t block_size;

    size_t bitset_size;
    std::vector<uint32_t> array;
public:
    MSGPACK_DEFINE(bitset_size, array)
};


BitMap operator ~(const BitMap& a);
BitMap operator &(const BitMap& a, const BitMap& b);
BitMap operator |(const BitMap& a, const BitMap& b);
BitMap operator ^(const BitMap& a, const BitMap& b);

std::ostream& operator<<(std::ostream &os, const BitMap &bitmap);

