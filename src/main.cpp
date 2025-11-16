#include <iostream>
#include <string>
#include <vector>

struct Args {
    std::string in, out, cmd;
};

bool parse(int argc, char* argv[], Args& a) {
    std::vector<std::string> v(argv + 1, argv + argc);
    bool has_i = false, has_o = false;
    size_t i = 0;
    while (i < v.size()) {
        if (v[i] == "-i") {
            if (has_i || i + 1 >= v.size()) return std::cerr << "No input directory given after -i\n", false;
            has_i = true; a.in = v[++i];
        }
        else if (v[i] == "-o") {
            if (has_o || i + 1 >= v.size()) return std::cerr << "No output directory given after -o\n", false;
            has_o = true; a.out = v[++i];
        }
        else if (v[i][0] != '-') break;
        else return false;
        ++i;
    }
    a.cmd.clear();
    for (; i < v.size(); ++i) {
        if (!a.cmd.empty()) a.cmd += " ";
        a.cmd += v[i];
    }

    if(!has_i) return std::cerr << "No input directory(-i)\n", false;
    if(!has_o) return std::cerr << "No output directory(-o)\n", false;
    if(a.cmd.empty()) return std::cerr << "No command to run your program\n", false;
    return true;
}

int main(int argc, char* argv[]) {
    Args a;
    if (!parse(argc, argv, a)) return 1;
    std::cerr << a.in << '\n' << a.out << '\n' << a.cmd << '\n';
    return 0;
}