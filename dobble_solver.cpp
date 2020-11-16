#include <array>
#include <iostream>
#include <vector>
#include <algorithm>


struct Problem {
    int N;
    int U;

    Problem(int n) : N(n), U(N*(N-1)+1) { }
};

struct Logo {
    int id;

    Logo(int id) : id(id) { }

    void next() {
        ++id;
    }

    bool operator==(const Logo& other) const { return id == other.id; }
    bool operator!=(const Logo& other) const { return id != other.id; }
};

std::vector<int> work(100, 0);

struct Card {
    std::vector<Logo> logos;

    bool compatible(const Problem& p, const Card& other) const {
        work.resize(p.U);
        std::fill(work.begin(), work.end(), 0);
        int collisions = 0;
        for(Logo l : logos) work[l.id]++;
        for(Logo l : other.logos) {
            if(work[l.id]) collisions++;
            work[l.id]++;
        }
        return collisions <= 1;
    }

    void push(const Problem& p) {
        if(logos.empty()) {
            logos.emplace_back(0);
        } else {
            logos.emplace_back(logos.back().id+1);
        }
    }

    bool valid(const Problem& p) const {
        return (logos.size() == p.N) && std::all_of(logos.begin(), logos.end(), [&p](const Logo& l){ return l.id < p.U; });
    }

    bool next(const Problem& p) {
        logos.back().next();
        return logos.back().id < p.U;
    }

    std::string toString() const {
        std::string s;
        for(Logo l : logos) s += std::to_string(l.id);
        return s;
    }

};

struct Solution {
    std::vector<Card> cards;
    bool nil;

    Solution() : cards(), nil(false) {}

    bool violates(const Problem& p) const {
        if(nil) return true;
        if(cards.size() > p.U) return true;
        for(int i = 0; i < cards.size(); ++i) {
            for(int j = i+1; j < cards.size(); ++j) {
                if(!cards[i].compatible(p, cards[j])) return true;
            }
        }
        return false;
    }

    bool valid(const Problem& p) const {
        if(nil) return false;
        if(cards.size() != p.U) return false;
        if(!std::all_of(cards.begin(), cards.end(), [&p](const Card& c){ return c.valid(p); })) return false;
        for(int i = 0; i < cards.size(); ++i) {
            for(int j = i+1; j < cards.size(); ++j) {
                if(!cards[i].compatible(p, cards[j])) return false;
            }
        }
        return true;
    }

    void push(const Problem& p) {
        if(cards.empty()) {
            cards.emplace_back();
            cards.back().push(p);
        } else {
            if(cards.back().logos.size() == p.N) {
                cards.emplace_back();
                cards.back().push(p);
            } else {
                cards.back().push(p);
            }
        }
    }

    void next(const Problem& p) {
        if(!cards.back().next(p)) {
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
        for(const Card& c : cards) s += c.toString() + " ";
        return s;
    }
};

Solution root(const Problem& p) {
    Solution r;
    for(int i = 0; i < 1+2*(p.N-1); ++i) { r.cards.emplace_back(); }
    r.cards[0].logos.resize(p.N, Logo{0});
    // first card
    for(int i = 0; i < p.N; ++i) {
        r.cards[0].logos[i] = Logo{i};
    }

    // first "column" : cards with 0
    for(int c = 0; c < p.N-1; ++c) {
        r.cards[1+c].logos.push_back(Logo{0});
    }
    int l = p.N;
    for(int c = 0; c < p.N-1; ++c) {
        for(int i = 1; i < p.N; ++i) {
            r.cards[1+c].logos.push_back(Logo{l});
            ++l;
        }
    }

    // first "column" : cards with 1
    for(int c = 0; c < p.N-1; ++c) {
        r.cards[p.N+c].logos.push_back(Logo{1});
    }
    l = p.N;
    for(int i = 1; i < p.N; ++i) {
        for(int c = 0; c < p.N-1; ++c) {
            r.cards[p.N+c].logos.push_back(Logo{l});
            ++l;
        }
    }

    std::cout << "Root : " << r.toString() << "\n";
    return r;
}

bool reject(const Problem& p, const Solution& sol) {
    return sol.violates(p);
}

bool accept(const Problem& p, const Solution& sol) {
    return sol.valid(p);
}

Solution first(const Problem& p, const Solution& candidate) {
    Solution s = candidate;
    s.push(p);
    return s;
}

void next(const Problem& p, Solution& sol) {
    sol.next(p);
}

int calls = 0;

void backtrack(const Problem& p, const Solution& candidate) {
    calls++;
    if(calls % 1000000 == 0) std::cout << calls << std::endl; 
    if(reject(p, candidate)) return;
    if(accept(p, candidate)) {
        std::cout << "Solution found" << std::endl;
        std::cout << candidate.toString() << std::endl;
        std::exit(0);
    }
    Solution s = first(p, candidate);
    while(!s.isNil()) {
        backtrack(p, s);
        next(p, s);
    }
}

int main(int argc, const char* argv[]) {
    if(argc > 1) {
        Problem p(std::atoi(argv[1]));
        std::cout << "N=" << p.N << " U=" << p.U << std::endl;
        backtrack(p, root(p));
    } else {
        std::cout << "Usage : exe N \n";
        std::cout << " with N = nb of logos per card\n"; 
    }
}