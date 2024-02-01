#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <functional>

using std::string;

template<typename T>
class SearchSpace {
public:
    size_t pindex;
    std::vector<int> current;
    std::vector<int> max;
    std::function<T(SearchSpace &g)> f;
    bool reject_flag;

public:
    SearchSpace(std::function<T(SearchSpace &g)> f) : f(f) {
        pindex = 0;
        reject_flag = false;
    }
    int new_parameter() {
        if(pindex >= current.size())
            current.push_back(0);
        auto result = current[pindex++];
        return result;
    }
    void reject(int max) {
        current[pindex-1] = max;
        reject_flag = true;
    }
    void enumerate(std::function<void(T)> callback) {
        current.clear();
        max.clear();
        // First, try to grab the max into current
        do {
            reject_flag = false;
            pindex = 0;
            current.push_back(1000);
            f(*this);
        } while(reject_flag);
        // Now we got the max, save it!
        current.pop_back();
        for(size_t i = 0; i < current.size(); i++) {
            max.push_back(current[i]);
            current[i] = 0;
        }
        // Current should iterate each parameter from 0 to max
        enumerate_r(0, callback);
    }
private:
    void enumerate_r(size_t index, std::function<void(T)> callback) {
        if(index == current.size()) {
            // We have a full set of parameters, call the function
            pindex = 0;
            callback(f(*this));
            return;
        }
        for(int i = 0; i <= max[index]; i++) {
            current[index] = i;
            enumerate_r(index+1, callback);
        }
    }
};

string get_statement(SearchSpace<string> &g);
