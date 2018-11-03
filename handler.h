#pragma once

#include <memory>

#include "responder.h"

namespace shitty { 

struct Handler {
    template <typename ResponderT>
    Handler(const std::string& path, const ResponderT& responder):
        path(path),
        responder(std::make_unique<ResponderT>(responder))
    {}

    //void invoke(RequestT&& request, Responder&& responder);

    std::string path;
    std::unique_ptr<Responder> responder;
};

} // namespace shitty
