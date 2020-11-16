#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <chrono>

#define template_header int N, int U=N*(N-1)+1

struct Logo {
    int id;

    Logo() : id(-1) { }
    Logo(int id) : id(id) { }

    void next() {
        ++id;
    }

    bool operator==(const Logo& other) const { return id == other.id; }
    bool operator!=(const Logo& other) const { return id != other.id; }
};

template<template_header>
struct Card {

    Card() : logos(), active(), nz(0) {
        std::fill(active.begin(), active.end(), 0);
    }

    std::array<Logo, N> logos;
    std::array<int, U> active;
    int nz;


    bool compatibleWith(const Card& other) const {
        check();
        other.check();
        int collisions = 0;
        if(nz == 0) return true;
        if(other.active[logos[nz-1].id] == 0) return true;
        for(int i = 0; i < other.nz; ++i) {
            int id = other.logos[i].id;
            if(active[id]) collisions++;
        }
        return collisions <= 1;
    }

    bool push() {
        assert(nz < N);
        int id = (nz == 0 ? 0 : logos[nz-1].id+1);
        if(id >= U) return false;
        logos[nz] = Logo(id);
        if(id < U) active[id] = 1;
        ++nz;
        return true;
    }
    void push(Logo l) {
        assert(nz < N);
        logos[nz] = l;
        assert(l.id >= 0 && l.id < U);
        active[l.id] = 1;
        ++nz;
    }

    bool valid() const {
        return (nz == N) && std::all_of(logos.begin(), logos.begin()+nz, [](const Logo& l){ return l.id < U; });
    }

    bool next() {
        assert(logos[nz-1].id >= 0 && logos[nz-1].id < U);
        active[logos[nz-1].id] = 0;
        logos[nz-1].next();
        bool ok = (logos[nz-1].id < U);
        if(ok) {
            active[logos[nz-1].id] = 1;
        }
        return ok;
    }

    void check() const {
        for(int i = 0; i < nz; ++i) {
            assert(logos[i].id >= 0 && logos[i].id < U);
            assert(active[logos[i].id]);
        }
        int acc = 0;
        for(int ac : active) acc += ac;
        assert(acc == nz);
    }

    std::string toString() const {
        std::string s;
        for(int i = 0; i< nz; ++i) s += std::to_string(logos[i].id) + " ";
        return s;
    }

};

template<template_header>
struct Solution {
    std::vector<Card<N,U>> cards;
    bool nil;

    Solution() : cards(), nil(false) {}

    bool violates() const {
        if(nil) return true;
        if(cards.size() > U) return true;
        for(int i = 0; i < cards.size()-1; ++i) {
            if(!cards.back().compatibleWith(cards[i])) return true;
        }
        return false;
    }

    bool valid() const {
        if(nil) return false;
        if(cards.size() != U) return false;
        if(!std::all_of(cards.begin(), cards.end(), [](const Card<N,U>& c){ return c.valid(); })) return false;
        for(int i = 0; i < cards.size(); ++i) {
            for(int j = i+1; j < cards.size(); ++j) {
                if(!cards[i].compatibleWith(cards[j])) return false;
            }
        }
        return true;
    }

    void push() {
        if(cards.empty()) {
            cards.emplace_back();
        } else {
            if(cards.back().nz == N) {
                cards.emplace_back();
            }
        }
        if(!cards.back().push()) {
            nil = true;
        }
    }

    void next() {
        if(!cards.back().next()) {
            nil = true;
        }
    }

    bool isNil() const {
        return nil;
    }

    std::string toString() const {
        std::string s;
        s += "nil?" + std::to_string(nil) + " ";
        s += "s" + std::to_string(cards.size()) + " ";
        for(const Card<N,U>& c : cards) s += c.toString() + " - ";
        return s;
    }
};

template<template_header>
struct Solver {

    Solution<N, U> root() {
        Solution<N, U> r;
        for(int i = 0; i < 1+2*(N-1); ++i) { r.cards.emplace_back(); }
        // first card
        for(int i = 0; i < N; ++i) {
            r.cards[0].push(Logo{i});
        }

        // first "column" : cards with 0
        for(int c = 0; c < N-1; ++c) {
            r.cards[1+c].push(Logo{0});
        }
        int l = N;
        for(int c = 0; c < N-1; ++c) {
            for(int i = 1; i < N; ++i) {
                r.cards[1+c].push(Logo{l});
                ++l;
            }
        }

        // first "column" : cards with 1
        for(int c = 0; c < N-1; ++c) {
            r.cards[N+c].push(Logo{1});
        }
        l = N;
        for(int i = 1; i < N; ++i) {
            for(int c = 0; c < N-1; ++c) {
                r.cards[N+c].push(Logo{l});
                ++l;
            }
        }

        std::cout << "Root : " << r.toString() << "\n";
        return r;
    }

    bool reject(const Solution<N, U>& sol) {
        return sol.violates();
    }

    bool accept(const Solution<N, U>& sol) {
        return sol.valid();
    }

    Solution<N,U> first(const Solution<N, U>& candidate) {
        Solution<N, U> s = candidate;
        s.push();
        return s;
    }

    void next(Solution<N, U>& sol) {
        sol.next();
    }

    long long calls = 0;
    std::chrono::system_clock::time_point begin;
    std::chrono::system_clock::time_point current;

    void backtrack(const Solution<N, U>& candidate) {
        calls++;
        if(calls % 10000000 == 0) {
            current = std::chrono::high_resolution_clock::now();
            auto elapsed = current - begin;
            std::cout 
                << (calls / 1.0e6) / (elapsed.count() / 1.0e9) << " Mcalls/s"
                //<< elapsed.count() << " t"
                //<< candidate.toString() 
                << std::endl;    
        }
        if(reject(candidate)) return;
        if(accept(candidate)) {
            std::cout << "Solution found" << std::endl;
            std::cout << candidate.toString() << std::endl;
            std::cout << "Total calls : " << calls << "\n";
            std::exit(0);
        }
        Solution<N, U> s = first(candidate);
        while(!s.isNil()) {
            backtrack(s);
            next(s);
        }
    }

    Solver() {
        begin = std::chrono::high_resolution_clock::now();
    }
};

int main(int argc, const char* argv[]) {
    constexpr int N = 6;
    Solver<N> s; 
    s.backtrack(s.root());
}