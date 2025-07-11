#include "TunnelUtils.hpp"

namespace et {
vector<PortForwardSourceRequest> parseRangesToRequests(const string& input) {
  vector<PortForwardSourceRequest> pfsrs;
  auto j = split(input, ',');
  for (auto& pair : j) {
    vector<string> sourceDestination = split(pair, ':');
    if (sourceDestination.size() < 2) {
      throw TunnelParseException(
          "Tunnel argument must have source and destination between a ':'");
    }
    try {
      if (sourceDestination[0].find_first_not_of("0123456789-") !=
              string::npos &&
          sourceDestination[1].find_first_not_of("0123456789-") !=
              string::npos) {
        PortForwardSourceRequest pfsr;
        pfsr.set_environmentvariable(sourceDestination[0]);
        pfsr.mutable_destination()->set_name(sourceDestination[1]);
        pfsrs.push_back(pfsr);
      } else if (sourceDestination[0].find('-') != string::npos &&
                 sourceDestination[1].find('-') != string::npos) {
        vector<string> sourcePortRange = split(sourceDestination[0], '-');
        int sourcePortStart = stoi(sourcePortRange[0]);
        int sourcePortEnd = stoi(sourcePortRange[1]);

        vector<string> destinationPortRange = split(sourceDestination[1], '-');
        int destinationPortStart = stoi(destinationPortRange[0]);
        int destinationPortEnd = stoi(destinationPortRange[1]);

        if (sourcePortEnd - sourcePortStart !=
            destinationPortEnd - destinationPortStart) {
          throw TunnelParseException(
              "source/destination port range must have same length");
        } else {
          int portRangeLength = sourcePortEnd - sourcePortStart + 1;
          for (int i = 0; i < portRangeLength; ++i) {
            PortForwardSourceRequest pfsr;
            pfsr.mutable_source()->set_name("localhost");
            pfsr.mutable_source()->set_port(sourcePortStart + i);
            pfsr.mutable_destination()->set_port(destinationPortStart + i);
            pfsrs.push_back(pfsr);
          }
        }
      } else if (sourceDestination[0].find('-') != string::npos ||
                 sourceDestination[1].find('-') != string::npos) {
        throw TunnelParseException(
            "Invalid port range syntax: if source is a range, "
            "destination must be a range (and vice versa)");
      } else {
        PortForwardSourceRequest pfsr;
        pfsr.mutable_source()->set_name("localhost");
        pfsr.mutable_source()->set_port(stoi(sourceDestination[0]));
        pfsr.mutable_destination()->set_port(stoi(sourceDestination[1]));
        pfsrs.push_back(pfsr);
      }
    } catch (const TunnelParseException& e) {
      throw e;
    } catch (const std::logic_error& lr) {
      throw TunnelParseException("Invalid tunnel argument '" + input +
                                 "': " + lr.what());
    }
  }
  return pfsrs;
}

vector<PortForwardSourceRequest> parseLocalForwardToRequests(const string& input) {
  vector<PortForwardSourceRequest> pfsrs;
  if (input.empty()) {
    return pfsrs;
  }
  
  auto forwardEntries = split(input, ',');
  for (auto& entry : forwardEntries) {
    try {
      // SSH LocalForward format: "local_port remote_host:remote_port"
      // Our tunnel format: "local_port:remote_host:remote_port"
      
      // Split on first ':' to separate local_port from remote_host:remote_port
      size_t colonPos = entry.find(':');
      if (colonPos == string::npos) {
        throw TunnelParseException("Invalid LocalForward format: " + entry);
      }
      
      string localPort = entry.substr(0, colonPos);
      string remoteHostPort = entry.substr(colonPos + 1);
      
      // Parse remote_host:remote_port
      size_t remoteColonPos = remoteHostPort.find(':');
      if (remoteColonPos == string::npos) {
        throw TunnelParseException("Invalid LocalForward remote format: " + remoteHostPort);
      }
      
      string remoteHost = remoteHostPort.substr(0, remoteColonPos);
      string remotePort = remoteHostPort.substr(remoteColonPos + 1);
      
      // Handle IPv6 addresses in brackets [::1]:port
      if (remoteHost.front() == '[' && remoteHost.back() == ']') {
        remoteHost = remoteHost.substr(1, remoteHost.length() - 2);
      }
      
      // Handle bind address in local port [bind_address]:port
      string bindAddress = "localhost";
      if (localPort.front() == '[') {
        size_t closeBracket = localPort.find(']');
        if (closeBracket != string::npos) {
          bindAddress = localPort.substr(1, closeBracket - 1);
          localPort = localPort.substr(closeBracket + 2); // Skip ']:' 
        }
      } else if (localPort.find(':') != string::npos) {
        size_t bindColonPos = localPort.find(':');
        bindAddress = localPort.substr(0, bindColonPos);
        localPort = localPort.substr(bindColonPos + 1);
      }
      
      PortForwardSourceRequest pfsr;
      pfsr.mutable_source()->set_name(bindAddress);
      pfsr.mutable_source()->set_port(stoi(localPort));
      pfsr.mutable_destination()->set_name(remoteHost);
      pfsr.mutable_destination()->set_port(stoi(remotePort));
      pfsrs.push_back(pfsr);
      
    } catch (const TunnelParseException& e) {
      throw e;
    } catch (const std::logic_error& lr) {
      throw TunnelParseException("Invalid LocalForward argument '" + entry + "': " + lr.what());
    }
  }
  return pfsrs;
}

}  // namespace et
