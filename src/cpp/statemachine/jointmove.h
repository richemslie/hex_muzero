#pragma once

namespace MuZero::SM {

struct JointMove {
  public:
    typedef short IndexType;

    static int mallocSize(int input_count) {
        return sizeof(JointMove) + input_count * sizeof(IndexType);
    }

  public:
    void setSize(int input_count) {
        this->input_count = input_count;
    }

    int get(int role_index) const {
        return *(this->input_indices + role_index);
    }

    void set(int role_index, int value) {
        *(this->input_indices + role_index) = value;
    }

    // effectively : this = other;
    void assign(const JointMove* other) {
        for (int ii = 0; ii < this->input_count; ii++) {
            this->input_indices[ii] = other->input_indices[ii];
        }
    }

    bool equals(const JointMove* other) const {
        for (int ii = 0; ii < this->input_count; ii++) {
            if (this->input_indices[ii] != other->input_indices[ii]) {
                return false;
            }
        }

        return true;
    }

  private:
    short input_count;
    IndexType input_indices[0];
};

}  // namespace MuZero::SM
