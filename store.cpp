#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <chrono>
#include <deque>
#include <fstream>
#include <vector>

using namespace std;
using namespace std::chrono;

enum Type { STRING, LIST };

struct Data {
    Type type = STRING;
    string value;
    deque<string> list;
    bool hasExpiry = false;
    time_point<system_clock> expiryTime;
};

unordered_map<string, Data> db;
int expiredCount = 0;

bool isExpired(Data d) {
    if (!d.hasExpiry) return false;
    return system_clock::now() >= d.expiryTime;
}

void setKey(string key, string value, int sec = -1) {
    Data d;
    d.value = value;
    if (sec != -1) {
        d.hasExpiry = true;
        d.expiryTime = system_clock::now() + seconds(sec);
    }
    db[key] = d;
    cout << "OK\n";
}

void getKey(string key) {
    if (db.find(key) == db.end()) { cout << "(nil)\n"; return; }
    if (isExpired(db[key])) { db.erase(key); expiredCount++; cout << "(nil)\n"; return; }
    cout << db[key].value << "\n";
}

void delKey(string key) {
    cout << (db.erase(key) ? "OK" : "(nil)") << "\n";
}

void ttlKey(string key) {
    if (db.find(key) == db.end()) { cout << -2 << "\n"; return; }
    if (isExpired(db[key])) { db.erase(key); expiredCount++; cout << -2 << "\n"; return; }
    if (!db[key].hasExpiry) { cout << -1 << "\n"; return; }
    cout << duration_cast<seconds>(db[key].expiryTime - system_clock::now()).count() << "\n";
}

void keysPattern(string pattern) {
    string prefix = pattern.substr(0, pattern.size() - 1);
    for (auto& kv : db)
        if (kv.first.find(prefix) == 0 && !isExpired(kv.second))
            cout << kv.first << "\n";
}

void stats() {
    size_t mem = 0;
    for (auto& kv : db) {
        mem += kv.first.size() + sizeof(Data);
        if (kv.second.type == STRING) mem += kv.second.value.size();
        else for (auto& item : kv.second.list) mem += item.size();
    }
    cout << "Total Keys: " << db.size() << "\nExpired Removed: " << expiredCount << "\nMemory Used: " << mem << " bytes\n";
}

void incrKey(string key) {
    if (db.find(key) == db.end() || isExpired(db[key])) {
        db[key] = Data(); db[key].value = "1"; cout << 1 << "\n";
    } else {
        try { int v = stoi(db[key].value) + 1; db[key].value = to_string(v); cout << v << "\n"; }
        catch (...) { cout << "ERR: not an integer\n"; }
    }
}

void decrKey(string key) {
    if (db.find(key) == db.end() || isExpired(db[key])) {
        db[key] = Data(); db[key].value = "-1"; cout << -1 << "\n";
    } else {
        try { int v = stoi(db[key].value) - 1; db[key].value = to_string(v); cout << v << "\n"; }
        catch (...) { cout << "ERR: not an integer\n"; }
    }
}

void lpush(string key, string value) {
    if (db.find(key) == db.end() || db[key].type != LIST) { db[key] = Data(); db[key].type = LIST; }
    db[key].list.push_front(value);
    cout << db[key].list.size() << "\n";
}

void rpush(string key, string value) {
    if (db.find(key) == db.end() || db[key].type != LIST) { db[key] = Data(); db[key].type = LIST; }
    db[key].list.push_back(value);
    cout << db[key].list.size() << "\n";
}

void lpop(string key) {
    if (db.find(key) == db.end() || db[key].type != LIST || db[key].list.empty()) { cout << "(nil)\n"; return; }
    string val = db[key].list.front(); db[key].list.pop_front();
    cout << val << "\n";
}

void rpop(string key) {
    if (db.find(key) == db.end() || db[key].type != LIST || db[key].list.empty()) { cout << "(nil)\n"; return; }
    string val = db[key].list.back(); db[key].list.pop_back();
    cout << val << "\n";
}

static string esc(const string& s) {
    string r;
    for (char c : s) {
        if (c == '\\') r += "\\\\";
        else if (c == '|') r += "\\p";
        else if (c == '\n') r += "\\n";
        else r += c;
    }
    return r;
}

static string unesc(const string& s) {
    string r;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            if (s[i+1] == '\\') { r += '\\'; i++; }
            else if (s[i+1] == 'p') { r += '|'; i++; }
            else if (s[i+1] == 'n') { r += '\n'; i++; }
            else r += s[i];
        } else r += s[i];
    }
    return r;
}

static vector<string> splitPipe(const string& line) {
    vector<string> fields; string cur;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == '\\' && i + 1 < line.size()) { cur += line[i]; cur += line[++i]; }
        else if (line[i] == '|') { fields.push_back(cur); cur = ""; }
        else cur += line[i];
    }
    fields.push_back(cur); return fields;
}

void saveSnapshot(string filename = "snapshot.db") {
    ofstream f(filename);
    for (auto& kv : db) {
        if (isExpired(kv.second)) continue;
        long long epoch = kv.second.hasExpiry ? duration_cast<milliseconds>(kv.second.expiryTime.time_since_epoch()).count() : 0;
        if (kv.second.type == STRING) f << "S|" << esc(kv.first) << "|" << epoch << "|" << esc(kv.second.value) << "\n";
        else {
            f << "L|" << esc(kv.first) << "|" << epoch << "|" << kv.second.list.size();
            for (auto& item : kv.second.list) f << "|" << esc(item);
            f << "\n";
        }
    }
    f.close();
    cout << "OK\n";
}

void loadSnapshot(string filename = "snapshot.db") {
    ifstream f(filename);
    if (!f.is_open()) { cout << "ERR: cannot open " << filename << "\n"; return; }
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p = splitPipe(line);
        if (p.size() < 4) continue;
        long long epoch = stoll(p[2]);
        Data d;
        if (epoch != 0) {
            d.hasExpiry = true; d.expiryTime = system_clock::time_point(milliseconds(epoch));
            if (isExpired(d)) continue;
        }
        if (p[0] == "S") { d.type = STRING; d.value = unesc(p[3]); }
        else if (p[0] == "L") { d.type = LIST; for (size_t i = 4; i < p.size(); i++) d.list.push_back(unesc(p[i])); }
        db[unesc(p[1])] = d;
    }
    cout << "OK\n";
}
