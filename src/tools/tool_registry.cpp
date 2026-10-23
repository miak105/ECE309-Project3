// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"
#include <utility>

// TODO: create your initial buckets
ToolRegistry::ToolRegistry() : buckets_(16) {       

}  
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

// TODO: FNV-1a (spec Appendix A) or another real string hash
std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::size_t result = 14695981039346656037ull;

    for (unsigned char ch : key) {
        result ^= ch;
        result *= 1099511628211ull;
    }

    return result;   
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
        // Check for a null pointer
    if (tool == nullptr) {
        return false;
    }

    // Get the name of the tool
    std::string name = tool->name();

    // Check if the tool already exists
    if (find(name) != nullptr) {
        return false;
    }

    // Resize if the load factor would exceed 0.75
    if (static_cast<double>(count_ + 1) / buckets_.size() > 0.75) {
        rehash(buckets_.size() * 2);
    }

    // Find the correct bucket
    std::size_t index = hash(name) % buckets_.size();

    // Insert the tool
    buckets_[index].push_back(std::move(tool));

    // Increase the number of tools
    count_++;

    return true;
}

Tool* ToolRegistry::find(std::string_view name) const {
    // Find the bucket where the tool should be
    std::size_t index = hash(name) % buckets_.size();

    // Search through the bucket
    for (const auto& tool : buckets_[index]) {
        if (tool->name() == name) {
            return tool.get();
        }
    }

    // Tool was not found
    return nullptr;
}

std::size_t ToolRegistry::size() const noexcept {
    return count_;
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const auto& bucket : buckets_) {
        for (const auto& tool : bucket) {
            fn(*tool);
        }
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();
}

double ToolRegistry::load_factor() const noexcept {
    if (buckets_.empty()) {
        return 0.0;
    }

    return static_cast<double>(count_) /
           static_cast<double>(buckets_.size());
}


std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;

    for (const auto& bucket : buckets_) {
        if (bucket.size() > longest) {
            longest = bucket.size();
        }
    }

    return longest;
}


void ToolRegistry::rehash(std::size_t new_bucket_count) {

    // Create a new hash table with more buckets
    std::vector<Bucket> newBuckets(new_bucket_count);

    // Go through every bucket in the old table
    for (auto& bucket : buckets_) {

        // Go through every tool in that bucket
        for (auto& tool : bucket) {

            // Calculate its new bucket index
            std::size_t index =
                hash(tool->name()) % new_bucket_count;

            // Move the tool into the new table
            newBuckets[index].push_back(std::move(tool));
        }
    }

    // Replace the old table with the new one
    buckets_ = std::move(newBuckets);
}
