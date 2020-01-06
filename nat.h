#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using IP = uint32_t;
using Port = uint32_t;
struct Socket {
    IP ip;
    Port port;

    bool operator==(const Socket& other) const {
        return ip == other.ip && port == other.port;
    }
};
struct SocketHash {
    // Combined hash function for socket = (ip, port) to use it as a key for hashmap
    size_t operator()(const Socket& socket) const {
        return (static_cast<uint64_t>(socket.ip) << 8) + socket.port;
    }
};

class NAT {
public:
    NAT() {}

    void AddIPPortMapping(const Socket& from, const Socket& to) {
        ip_port_map_[from] = to;
    }

    void AddIPMapping(const IP& from, const Socket& to) {
        ip_only_map_[from] = to;
    }

    void AddPortMapping(const Port& from, const Socket& to) {
        port_only_map_[from] = to;
    }

    std::pair<Socket, bool> Translate(const Socket& socket) const {
        auto it0 = ip_port_map_.find(socket);
        if (it0 != ip_port_map_.end()) {
            return {it0->second, true};
        }
        auto it1 = ip_only_map_.find(socket.ip);
        if (it1 != ip_only_map_.end()) {
            return {it1->second, true};
        }
        auto it2 = port_only_map_.find(socket.port);
        if (it2 != port_only_map_.end()) {
            return {it2->second, true};
        }
        return {socket, false};
    }

private:
    std::unordered_map<Socket, Socket, SocketHash> ip_port_map_;
    std::unordered_map<IP, Socket> ip_only_map_;
    std::unordered_map<Port, Socket> port_only_map_;
};

class IPPortParser {
public:
    std::pair<std::pair<IP, bool>, std::pair<Port, bool>> ParseSocket(const std::string& str_socket) const {
        auto colon_pos = str_socket.find(':');
        return {ParseIP(str_socket.substr(0, colon_pos)), ParsePort(str_socket.substr(colon_pos + 1))};
    }

    std::string Socket2Str(const Socket& socket) const {
        std::string str_socket;
        auto ip = socket.ip;
        auto port = socket.port;
        for (int i = 0; i < 4; ++i) {
            str_socket = std::to_string(ip % 256) + "." + str_socket;
            ip /= 256;
        }
        str_socket.back() = ':';
        str_socket += std::to_string(port);

        return str_socket;
    }

private:
    // Second boolean argument indicates whether IP or port is given (true) or masked, i.e. * (false)
    std::pair<IP, bool> ParseIP(const std::string& str_ip) const {
        if (str_ip == "*") {
            return {0, false};
        }
        std::string cur;
        IP ip = 0;
        for (char c : str_ip) {
            if (c == '.') {
                ip = (ip << 8) + std::stoi(cur);
                cur = "";
            } else {
                cur += c;
            }
        }
        ip = (ip << 8) + std::stoi(cur);
        return {ip, true};
    }

    std::pair<Port, bool> ParsePort(const std::string& str_port) const {
        if (str_port == "*") {
            return {0, false};
        }
        return {std::stoi(str_port), true};
    }
};

NAT ReadNATMap(const std::string& filename);

std::vector<Socket> ReadFlow(const std::string& filename);

std::vector<std::pair<Socket, bool>> TranslateFlow(NAT& nat, const std::vector<Socket>& flow);

void WriteOutput(const std::vector<Socket>& flow, const std::vector<std::pair<Socket, bool>>& out, const std::string& filename);
