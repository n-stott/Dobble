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

    constexpr Card() : header(-1), nz(0), logos(), active() { }

    void init(short header) {
        this->header = header;
        nz = 0;
        std::fill(active.begin(), active.end(), 0);
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
        // push(nz == 0 ? nz : logos[nz-1].id+1);
        push(P*nz);
        // push(nz == 0 ? nz : std::max(P*nz, logos[nz-1].id+1));
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
            assert(logos[i].id / P == i);
            if(active[logos[i].id] != 1) {
                std::cout << toString() << std::endl;
            }
            assert(active[logos[i].id] == 1);
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
    std::chrono::system_clock::time_point begin;
    std::chrono::system_clock::time_point current;

    void log(const Solution<P>& candidate) {
        current = std::chrono::high_resolution_clock::now();
        auto elapsed = current - begin;
        std::cout << "\x1B[2J\x1B[H";
        std::cout 
            << "Cursor : " << candidate.cursor << '\n'
            << "AbortH : " << candidate.abortHeight << '\n'
            << (calls / 1.0e6) << " Mcalls\n"
            << (calls / 1.0e6) / (elapsed.count() / 1.0e9) << " Mcalls/s" << '\n'
#if CHECK_IMMEDIATE_REJECT
            << (100.0 * immediatelyRejected / calls) << "% reject \n"
#endif
            << candidate.toString() 
            << std::endl;    
    }

    void backtrack(Solution<P>& candidate) {
        // std::cout << candidate.toString() << '\n';
        calls++;
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
#endif
        if(candidate.reject()) {
#if CHECK_IMMEDIATE_REJECT
            if(immediateCandidate) {
                // std::cout << candidate.toString() << '\n';
                immediatelyRejected++;
            }
            immediateCandidate = false;
#endif
            return;
        }
#if CHECK_IMMEDIATE_REJECT
        immediateCandidate = false;
#endif
        if(candidate.accept()) {
            log(candidate);
            std::cout << "Solution found" << std::endl;
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

template<int P>
void run() {
    Solver<P> s;
    Solution<P> sol = Solution<P>::root();
    s.backtrack(sol);
    std::cout << "Total calls : " << s.calls << "\n";
}

int main(int argc, const char* argv[]) {
    if(argc != 2) {
        std::cout << "Usage : exe P\n";
    } else {
        int P = std::atoi(argv[1]);
        switch(P) {
            case 1: run<1>(); break;
            case 2: run<2>(); break;
            case 3: run<3>(); break;
            case 4: run<4>(); break;
            case 5: run<5>(); break;
            case 6: run<6>(); break;
            case 7: run<7>(); break;
            case 8: run<8>(); break;
            case 9: run<9>(); break;
            case 10: run<10>(); break;
            default: std::cout << "P = " << P << " not supported yet\n";
        }
    }
}