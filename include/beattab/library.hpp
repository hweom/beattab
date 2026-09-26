#pragma once
#include "model.hpp"
#include <filesystem>
namespace bt {
struct Entry { std::filesystem::path path; Song song; };
struct Library {
    std::vector<Entry> entries;
    std::vector<Diagnostic> errors;
    void scan(const std::filesystem::path& root);
    std::vector<size_t> search(std::string query) const;
};
}
