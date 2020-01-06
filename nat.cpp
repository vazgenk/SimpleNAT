#include <fstream>

#include "nat.h"

NAT ReadNATMap(const std::string& filename="NAT") {
    NAT nat;
    IPPortParser parser;
    
    std::ifstream ifs(filename);
    std::string line;
    while (ifs >> line) {
        auto comma_pos = line.find(',');
        auto [ip1, port1] = parser.ParseSocket(line.substr(0, comma_pos));
        auto [ip2, port2] = parser.ParseSocket(line.substr(comma_pos + 1));
        Socket from = {ip1.first, port1.first};
        Socket to = {ip2.first, port2.first};
        if (ip1.second && port1.second) {
            // both ip1 and port1 are given
            nat.AddIPPortMapping(from, to);
        } else if (ip1.second) {
            // port1 is masked
            nat.AddIPMapping(ip1.first, to);
        } else {
            // ip1 is masked
            nat.AddPortMapping(port1.first, to);
        }
    }
    
    return nat;
}

std::vector<Socket> ReadFlow(const std::string& filename="FLOW") {
    IPPortParser parser;
    std::vector<Socket> flow;

    std::ifstream ifs(filename);
    std::string line;
    while (ifs >> line) {
        auto [ip, port] = parser.ParseSocket(line);
        flow.push_back({ip.first, port.first});
    }
    
    return flow;
}

std::vector<std::pair<Socket, bool>> TranslateFlow(NAT& nat, const std::vector<Socket>& flow) {
    std::vector<std::pair<Socket, bool>> out;
    for (const auto& socket : flow) {
        out.push_back(nat.Translate(socket));
    }

    return out;
}

void WriteOutput(const std::vector<Socket>& flow, const std::vector<std::pair<Socket, bool>>& out, const std::string& filename="OUTPUT") {
    IPPortParser parser;

    std::ofstream ofs(filename);
    for (size_t i = 0; i < flow.size(); ++i) {
        if (out[i].second) {
            // there IS a match for flow[i]
            ofs << parser.Socket2Str(flow[i]) + " -> " + parser.Socket2Str(out[i].first) + '\n';
        } else {
            // there is NO match for flow[i]
            ofs << "No NAT match for " + parser.Socket2Str(flow[i]) + '\n';
        }
    }
}

int main() {
    auto nat = ReadNATMap();
    auto flow = ReadFlow();
    
    auto out = TranslateFlow(nat, flow);
    
    WriteOutput(flow, out);
    return 0;
}
