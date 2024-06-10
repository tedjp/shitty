#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "RequestHandler.h"

namespace shitty {

class Routes {
public:
    void addHandler(std::string_view path, RequestHandler&& handler);

    void dispatch(Request&& request, Responder&& responder);

private:
    // Handlers are kept in the order they were added.
    std::vector<std::pair<std::string, RequestHandler>> handlers_;
};

}
