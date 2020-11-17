#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <chrono>

#define template_header int P, int U = P*P

#define CHECK_IMMEDIATE_REJECT 1

struct Logo {
    short id;

    std::string toString(int P) const {
        std::string s;
        // s += std::to_string(id);
        s += std::to_string(id%P) + char(65+(id)/P);
        return s;
    }

};

template<template_header>
struct Card {
    short header;
    short nz;
    std::array<Logo, P> logos;
    std::array<short, U> active;

    constexpr Card() : header(-1), nz(0), logos(), active() { }

    void init(short header) {
        this->header = header;
        nz = 0;
        std::fill(active.begin(), active.end(), 0);
    }

    bool hasNext() const {
        assert(nz > 0);
        bool ok = logos[nz-1].id + (P-nz) < U-1;
        return ok;
    }

    void next() {
        check();
        assert(hasNext());
        active[logos[nz-1].id] = 0;
        logos[nz-1].id++;
        active[logos[nz-1].id] = 1;
        check();
    }

    void push(int id) {
        check();
        assert(active[id] == 0);
        logos[nz].id = id;
        active[id] = 1;
        ++nz;
        check();
    }

    void pushBest() {
        push(nz == 0 ? nz : logos[nz-1].id+1);
    }

    void pop() {
        check();
        --nz;
        assert(active[logos[nz].id] == 1);
        active[logos[nz].id] = 0;
        check();
    }

    bool compatibleWith(const Card& other) const {
        short collisions = (header == other.header);
        for(short i = 0; i < nz; ++i) {
            collisions += (other.active[logos[i].id]);
        }
        return collisions <= 1;
    }

    std::string toString() const { 
        std::string s;
        s += 's' + std::to_string(nz) + " " + 'h' + std::to_string(header) + " ";
        for(short i = 0; i < nz; ++i) s += logos[i].toString(P) + " "; 
        return s;
    }

    void check() {
        #ifndef NDEBUG
        for(short i = 0; i < nz; ++i) {
            assert(active[logos[i].id] == 1);
        }
        #endif
    }
};

template<template_header>
struct Solution {
    std::array<Card<P,U>, P*(P+1)> cards;
    short cursor;
    bool abortFlag;

    Solution() : cards(), cursor(0), abortFlag(false) {
        for(short i = 0; i < P+1; ++i) {
            for(short j = 0; j < P; ++j) {
                cards[P*i+j].init(i);
            }
        }
    }

    static Solution root() {
        return Solution();
    }

    bool abort() const {
        return abortFlag;
    }

    bool reject() const {
        for(int i = 0; i < cursor; ++i) {
            if(!cards[cursor].compatibleWith(cards[i])) {
                // std::cout << "rejected : incompatibility : " << std::endl;
                // std::cout << cards[cursor].toString() << std::endl;
                // std::cout << "and"  << std::endl;
                // std::cout << cards[i].toString()  << std::endl;
                return true;
            }
        }
        return false;
    }

    bool accept() const {
        if(cursor < P*(P+1)-1) return false;
        if(cards[cursor].nz != P) return false;
        return true;
    }

    bool hasNext() const {
        return cards[cursor].hasNext();
    }

    void next() {
        cards[cursor].next();
    }

    void push() {
        if(cards[cursor].nz == P) ++cursor;
        assert(cursor < P*(P+1));
        // cards[cursor].push(0);
        cards[cursor].pushBest();
    }

    void pop() {
        cards[cursor].pop();
        if(cards[cursor].nz == 0) --cursor;
        if(cursor == 2*P) abortFlag = true;
    }

    void registerColumnUsage(short id) {
        columnUsage[cursor/P][id] = 1;
    }

    void deregisterColumnUsage(short id) {
        columnUsage[cursor/P][id] = 0;
    }

    void registerGlobalUsage(short id) {
        const Card<P>& c = cards[cursor];
        for(short i = 0; i < c.nz; ++i) {
            globalUsage[c.logos[i].id][id] = 1;
            globalUsage[i][c.logos[id].id] = 1;
        }
    }

    void deregisterGlobalUsage(short id) {
        const Card<P>& c = cards[cursor];
        for(short i = 0; i < c.nz-1; ++i) {
            globalUsage[c.logos[i].id][id] = 0;
            globalUsage[i][c.logos[id].id] = 0;
        }
    }

    std::string toString() const { std::string s; for(short i = 0; i <= cursor; ++i) s += cards[i].toString() + '\n'; return s;}
};


template<template_header>
struct Solver {

    Solution<P> first(const Solution<P>& candidate) {
        Solution<P> s = candidate;
        s.push();
        return s;
    }

    long long calls = 0;

#if CHECK_IMMEDIATE_REJECT
    long long immediatelyRejected = 0;
    bool immediateCandidate = false;
#endif
    std::chrono::system_clock::time_point begin;
    std::chrono::system_clock::time_point current;

    void backtrack(Solution<P>& candidate) {
        // std::cout << candidate.toString() << '\n';
        calls++;
        if(calls % 10000000 == 0) {
            current = std::chrono::high_resolution_clock::now();
            auto elapsed = current - begin;
            std::cout << "\x1B[2J\x1B[H";
            std::cout 
                << (calls / 1.0e6) << " Mcalls\n"
                << (calls / 1.0e6) / (elapsed.count() / 1.0e9) << " Mcalls/s" << '\n'
#if CHECK_IMMEDIATE_REJECT
                << (100.0 * immediatelyRejected / calls) << "% reject \n"
#endif
                << candidate.toString() 
                << std::endl;    
        }

        if(candidate.abort()) {
            std::cout << "No solution found" << std::endl;
            std::cout << "Total calls : " << calls << "\n";
            std::exit(0);
        }

#if CHECK_IMMEDIATE_REJECT
        immediateCandidate = true;
#endif
        if(candidate.reject()) {
#if CHECK_IMMEDIATE_REJECT
            if(immediateCandidate) immediatelyRejected++;
            immediateCandidate = false;
#endif
            return;
        }
#if CHECK_IMMEDIATE_REJECT
        immediateCandidate = false;
#endif
        if(candidate.accept()) {
            std::cout << "Solution found" << std::endl;
            std::cout << candidate.toString() << std::endl;
            std::cout << "Total calls : " << calls << "\n";
            std::exit(0);
        }

        candidate.push();
        backtrack(candidate);
        while(true) {
            if(candidate.hasNext()) {
                candidate.next();
                backtrack(candidate);
            } else {
                break;
            }
        }
        candidate.pop();

    }

    Solver() {
        begin = std::chrono::high_resolution_clock::now();
    }
};

int main(int argc, const char* argv[]) {
    constexpr int P = 5;
    Solver<P> s;
    Solution<P> sol = Solution<P>::root();
    s.backtrack(sol);

    std::cout << "Total calls : " << s.calls << "\n";
}