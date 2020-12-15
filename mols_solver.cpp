#include <array>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>

#define template_header int P, int U = P*P

#define CHECK_IMMEDIATE_REJECT 1

struct Logo {
    short id;

    std::string toString(int P) const {
        std::string s;
        s += std::to_string(id%P);
        // s += std::to_string(id%P) + char(65+(id)/P);
        return s;
    }

};

template<template_header>
struct Card {
    short header;
    short nz;
    std::array<Logo, P> logos;
    std::array<short, U> active;
    std::array<short, P> number;

    constexpr Card() : header(-1), nz(0), logos(), active() { }

    void init(short header) {
        this->header = header;
        nz = 0;
        std::fill(active.begin(), active.end(), 0);
        std::fill(number.begin(), number.end(), 0);
    }

    bool hasNext() const {
        assert(nz > 0);
        bool ok = (logos[nz-1].id < P*nz-1);
        return ok;
    }

    void next() {
        check();
        assert(hasNext());
        active[logos[nz-1].id] = 0;
        --number[logos[nz-1].id % P];
        logos[nz-1].id++;
        if(header > 0 && nz < P) while(number[logos[nz-1].id % P] && logos[nz-1].id < P*nz-1) logos[nz-1].id++;
        active[logos[nz-1].id] = 1;
        ++number[logos[nz-1].id % P];
        check();
    }

    void push(int id) {
        check();
        if(header > 0) while(number[id % P] > 0) ++id;
        assert(active[id] == 0);
        logos[nz].id = id;
        active[id] = 1;
        ++number[id % P];
        ++nz;
        check();
    }

    void pushBest() {
        push(P*nz);
    }

    void pop() {
        check();
        --nz;
        assert(active[logos[nz].id] == 1);
        active[logos[nz].id] = 0;
        --number[logos[nz].id % P];
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
            assert(logos[i].id / P == i);
            if(active[logos[i].id] != 1) {
                std::cout << toString() << std::endl;
            }
            assert(active[logos[i].id] == 1);
            assert(number[logos[i].id % P] >= 0);
        }
        #endif
    }
};

template<template_header>
struct Solution {
    std::array<Card<P,U>, P*(P)> cards;
    short cursor;
    short abortHeight;
    bool abortFlag;

    Solution() : cards(), cursor(0), abortHeight(P), abortFlag(false) {
        for(short i = 0; i < P; ++i) {
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
        if(cards[cursor].nz > 0 && cards[cursor].logos[0].id != cursor % P) return true;
        for(int i = cursor; i --> 0;) {
            if(!cards[cursor].compatibleWith(cards[i])) {
                return true;
            }
        }
        return false;
    }

    bool accept() const {
        if(cursor < P*(P)-1) return false;
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
        if(cursor == abortHeight) abortFlag = true;
    }

    std::string toString() const { std::string s; for(short i = 0; i <= cursor; ++i) s += cards[i].toString() + ((1+i)%P == 0 ? "\n\n" : "\n"); return s;}
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
    bool debugMode = false;
    std::chrono::system_clock::time_point begin;
    std::chrono::system_clock::time_point current;

    short height = 0;
    short summit = 0;
    std::array<long long, P*U> spent;

    std::string repartition() const {
        std::string s;
        for(int i = 0; i < U; ++i) {
            double acc = 0;
            for(int p = 0; p < P; ++p) {
                acc += std::log10(1+spent[P*i+p]);
            }
            acc += (acc > 0);
            s += std::string((int)acc, '*') + '\n';
        }
        return s;
    }

    void log(const Solution<P>& candidate) {
        current = std::chrono::high_resolution_clock::now();
        auto elapsed = current - begin;
        std::cout << "\x1B[2J\x1B[H";
        std::cout 
            << "Current : " << height << '\n'
            << "Max     : " << summit << '\n'
            << "Target  : " << P*U << '\n'
            << (calls / 1.0e6) << " Mcalls\n"
            << (calls / 1.0e6) / (elapsed.count() / 1.0e9) << " Mcalls/s" << '\n'
#if CHECK_IMMEDIATE_REJECT
            << (100.0 * immediatelyRejected / calls) << "% reject \n"
#endif
            // << repartition()
            << candidate.toString() 
            << std::endl;    
    }

    void backtrack(Solution<P>& candidate) {
        calls++;
        height = P*candidate.cursor + candidate.cards[candidate.cursor].nz;
        summit = std::max(summit, height);
        ++spent[height];
        if(calls % ( 1 << 21 ) == 0) {
            log(candidate);
        }

        if(candidate.abort()) {
            log(candidate);
            std::cout << "No solution found" << std::endl;
            std::cout << "Total calls : " << calls << "\n";
            std::exit(0);
        }

#if CHECK_IMMEDIATE_REJECT
        immediateCandidate = true;
        if(candidate.reject()) {
            if(immediateCandidate) {
                if(debugMode) std::cout << candidate.toString() << std::endl;
                immediatelyRejected++;
            }
            immediateCandidate = false;
            return;
        }
        immediateCandidate = false;
#else
        if(candidate.reject()) return;
#endif
        if(candidate.accept()) {
            log(candidate);
            std::cout << "Solution found" << std::endl;
            std::cout << "Total calls : " << calls << "\n";
            std::exit(0);
        }

        candidate.push();
        backtrack(candidate);
        while(candidate.hasNext()) {
            candidate.next();
            backtrack(candidate);
        }
        candidate.pop();

    }

    Solver() {
        begin = std::chrono::high_resolution_clock::now();
        std::fill(spent.begin(), spent.end(), 0);
    }
};

template<int P>
void run(bool debugMode) {
    Solver<P> s;
    s.debugMode = debugMode;
    Solution<P> sol = Solution<P>::root();
    s.backtrack(sol);
    std::cout << "Total calls : " << s.calls << "\n";
}

int main(int argc, const char* argv[]) {
    if(argc != 2 && argc != 3) {
        std::cout << "Usage : exe P debugMode\n";
    } else {
        int P;
        int d = false;
        if(argc >= 2) P = std::atoi(argv[1]);
        if(argc >= 3) d = std::atoi(argv[2]);
        switch(P) {
            case 1: run<1>(d); break;
            case 2: run<2>(d); break;
            case 3: run<3>(d); break;
            case 4: run<4>(d); break;
            case 5: run<5>(d); break;
            case 6: run<6>(d); break;
            case 7: run<7>(d); break;
            case 8: run<8>(d); break;
            case 9: run<9>(d); break;
            case 10: run<10>(d); break;
            default: std::cout << "P = " << P << " not supported yet\n";
        }
    }
}