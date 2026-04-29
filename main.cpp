#include "store.cpp"

int main() {
    string line;
    while (getline(cin, line)) {
        stringstream ss(line); string cmd; ss >> cmd;
        if (cmd == "SET") { string k, v, e; int s; ss >> k >> v; if (ss >> e >> s) setKey(k, v, s); else setKey(k, v); }
        else if (cmd == "GET") { string k; ss >> k; getKey(k); }
        else if (cmd == "DEL") { string k; ss >> k; delKey(k); }
        else if (cmd == "TTL") { string k; ss >> k; ttlKey(k); }
        else if (cmd == "KEYS") { string p; ss >> p; keysPattern(p); }
        else if (cmd == "STATS") { stats(); }
        else if (cmd == "INCR") { string k; ss >> k; incrKey(k); }
        else if (cmd == "DECR") { string k; ss >> k; decrKey(k); }
        else if (cmd == "LPUSH") { string k, v; ss >> k >> v; lpush(k, v); }
        else if (cmd == "RPUSH") { string k, v; ss >> k >> v; rpush(k, v); }
        else if (cmd == "LPOP") { string k; ss >> k; lpop(k); }
        else if (cmd == "RPOP") { string k; ss >> k; rpop(k); }
        else if (cmd == "SAVE") { string f; if (ss >> f) saveSnapshot(f); else saveSnapshot(); }
        else if (cmd == "LOAD") { string f; if (ss >> f) loadSnapshot(f); else loadSnapshot(); }
        else if (cmd == "EXIT") { break; }
        else { cout << "Wrong Command\n"; }
    }
    return 0;
}