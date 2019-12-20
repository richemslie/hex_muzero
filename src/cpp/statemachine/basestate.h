#pragma once

#include <cstdint>
#include <cstring>

#include <unordered_map>
#include <unordered_set>

namespace MuZero::SM {

struct BaseState {
  public:
    // Historical, attempted to different sizes of ArrayType.  In the end it was simplest and
    // most efficient to use bytes.  Leaving typedef as it doesn't make things less clear.
    typedef uint8_t ArrayType;
    static int const ARRAYTYPE_BYTES = 1;
    static int const ARRAYTYPE_BITS = 8;

    static int mallocSize(int num_bases) {
        int bytes_for_bases = num_bases / BaseState::ARRAYTYPE_BITS;
        if (num_bases % ARRAYTYPE_BITS != 0) {
            bytes_for_bases++;
        }

        return sizeof(BaseState) + bytes_for_bases;
    }

  public:
    // This should override new/delete where the 'data; is part of the allocation
    void init(const int size) {
        this->size = size;
        this->byte_count = size / ARRAYTYPE_BITS;
        if (size % ARRAYTYPE_BITS != 0) {
            this->byte_count++;
        }

        std::memset(this->data, 0, this->byte_count * ARRAYTYPE_BYTES);
    }

    const bool get(const int index) const {
        const ArrayType* ptdata = this->data + (index / ARRAYTYPE_BITS);
        int index2 = index % ARRAYTYPE_BITS;
        return *ptdata & (ArrayType(1) << index2);
    }

    const void set(const int index, const bool value) {
        ArrayType* ptdata = this->data + (index / ARRAYTYPE_BITS);
        int index2 = index % ARRAYTYPE_BITS;
        if (value) {
            *ptdata |= (ArrayType(1) << index2);
        } else {
            *ptdata &= ~(ArrayType(1) << index2);
        }
    }

    size_t hashCode() const {
        Hasher hasher;
        return hasher(this);
    }

    bool equals(const BaseState* other) const {
        for (int ii = 0; ii < this->byte_count; ii++) {
            if (this->data[ii] != other->data[ii]) {
                return false;
            }
        }

        return true;
    }

    void assign(const BaseState* other) {
        std::memcpy(this->data, other->data, this->byte_count * ARRAYTYPE_BYTES);
    }

  public:
    void calculateHashCode();

    struct Hasher {
        std::size_t operator()(const BaseState* key) const;
    };

    struct Equals {
        bool operator()(const BaseState* a, const BaseState* b) const {
            return a->equals(b);
        }
    };


    struct HasherMasked {
        HasherMasked(const ArrayType* mask_buf) : mask_buf(mask_buf) {
        }

        std::size_t operator()(const BaseState* key) const;

      private:
        const ArrayType* mask_buf;
    };

    struct EqualsMasked {
        EqualsMasked(const ArrayType* mask_buf) : mask_buf(mask_buf) {
        }

        bool operator()(const BaseState* a, const BaseState* b) const {
            for (int ii = 0; ii < a->byte_count; ii++) {
                ArrayType aa = a->data[ii] & this->mask_buf[ii];
                ArrayType bb = b->data[ii] & this->mask_buf[ii];

                if (aa != bb) {
                    return false;
                }
            }

            return true;
        }

      private:
        const ArrayType* mask_buf;
    };


  public:
    short size;
    short byte_count;
    ArrayType data[0];

  public:
    template <typename V>
    using HashMap = std::unordered_map<const BaseState*, V, Hasher, Equals>;

    template <typename V>
    using HashMapMasked = std::unordered_map<const BaseState*, V, HasherMasked, EqualsMasked>;

    template <typename V>
    static HashMapMasked<V>* makeMaskedMap(const ArrayType* mask_buf,
                                           typename HashMapMasked<V>::size_type bucket_size = 0) {
        return new std::unordered_map<const BaseState*, V, HasherMasked, EqualsMasked>(
            bucket_size, HasherMasked(mask_buf), EqualsMasked(mask_buf));
    }

    typedef std::unordered_set<const BaseState*, Hasher, Equals> HashSet;
};

}  // namespace MuZero::SM
