#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <cassert>

using namespace std;

// Structure for Trie Node
struct TrieNode {
    // Using map for children allows flexibility with character sets (e.g., ASCII, Unicode)
    // Key: character, Value: pointer to the child node
    map<char, TrieNode*> children;

    // Failure link: points to the longest proper suffix of the current node's string
    // that is also a prefix of some pattern.
    TrieNode* failureLink;

    // Points to the nearest pattern-ending node in the failure path
    TrieNode* outputLink;

    // Stores indices of patterns ending EXACTLY at this node
    vector<int> patternIndices;

    // Constructor
    TrieNode() : failureLink(nullptr), outputLink(nullptr) {}
};

class AhoCorasick {
private:
    TrieNode* root;
    vector<string> patterns; // Store the original patterns for reference

    // Helper function to recursively delete the trie nodes
    void deleteTrie(TrieNode* node) {
        if (!node) return;
        for (auto const& [key, val] : node->children) {
            deleteTrie(val);
        }
        delete node;
    }

public:
    AhoCorasick() {
        root = new TrieNode();
        root->failureLink = root; // Root's failure link points to itself
    }

    ~AhoCorasick() {
        deleteTrie(root);
    }

    /**
     * @brief Adds a pattern to the Aho-Corasick automaton.
     * 
     * This function adds the given pattern to the trie. It traverses the trie
     * according to the characters in the pattern, creating new nodes if necessary.
     * @param pattern The pattern to be added.
     * 
     * @note Time Complexity: O(p), where p is the length of the pattern.
     * @note Space Complexity: O(p), in the worst case where the pattern does not
     *       share any prefix with the existing patterns.
     */
    void addPattern(const string& pattern) {
        TrieNode* currentNode = root;
        int patternIndex = patterns.size();
        patterns.push_back(pattern);

        for (char ch : pattern) {
            if (currentNode->children.find(ch) == currentNode->children.end()) {
                currentNode->children[ch] = new TrieNode();
            }
            currentNode = currentNode->children[ch];
        }
        currentNode->patternIndices.push_back(patternIndex);
    }

    /**
     * @brief Builds the failure links and output links for the Aho-Corasick automaton.
     * 
     * The function uses a breadth-first search (BFS) to traverse the trie and set
     * the failure and output links for each node.
     * 
     * @note Time Complexity: O(m), where m is the total number of characters in all
     *       patterns.
     * @note Space Complexity: O(m), mainly for storing the nodes of the Trie. The
     *       queue used for BFS has a maximum size of the number of nodes.
     */
    void buildFailureLinks() {
        queue<TrieNode*> q;

        for (auto const& [key, val] : root->children) {
            TrieNode* childNode = val;
            childNode->failureLink = root;
             childNode->outputLink = nullptr;
            q.push(childNode);
        }

        while (!q.empty()) {
            TrieNode* currentNode = q.front();
            q.pop();

            for (auto const& [key, val] : currentNode->children) {
                char transitionChar = key;
                TrieNode* targetNode = val;

                TrieNode* tempFailureNode = currentNode->failureLink;
                while (tempFailureNode != root && tempFailureNode->children.find(transitionChar) == tempFailureNode->children.end()) {
                    tempFailureNode = tempFailureNode->failureLink;
                }
                if (tempFailureNode->children.find(transitionChar) != tempFailureNode->children.end()) {
                    targetNode->failureLink = tempFailureNode->children[transitionChar];
                } else {
                    targetNode->failureLink = root;
                }

                TrieNode* failNode = targetNode->failureLink;
                if (!failNode->patternIndices.empty()) {
                    targetNode->outputLink = failNode;
                } else {
                    targetNode->outputLink = failNode->outputLink;
                }

                q.push(targetNode);
            }
        }
    }

    /**
     * @brief Searches for all patterns in the given text using the Aho-Corasick algorithm.
     * 
     * This function traverses the text character by character. For each character,
     * it follows the transitions in the Aho-Corasick automaton. If it reaches a
     * node that corresponds to the end of one or more patterns, it records the
     * pattern index and the ending position of the match in the text.
     * 
     * @param text The text to search within.
     * @return A vector of pairs. Each pair contains the index of a found pattern
     *         and the ending position (index in the text) where the pattern ends.
     *         The starting position of the match can be derived by subtracting
     *         the pattern length from the ending position.
     * 
     * @note Time Complexity: O(n + z), where n is the length of the text, 
     *       and z is the number of matches found.
     * @note Space Complexity: O(m), where m is the total length of all patterns.
     */
    vector<pair<int, int>> search(const string& text) {
        vector<pair<int, int>> matches;
        TrieNode* currentNode = root;

        for (int i = 0; i < text.length(); ++i) {
            char currentChar = text[i];

            while (currentNode != root && currentNode->children.find(currentChar) == currentNode->children.end()) {
                currentNode = currentNode->failureLink;
            }

            if (currentNode->children.find(currentChar) != currentNode->children.end()) {
                currentNode = currentNode->children[currentChar];
            }

            TrieNode* outputNode = currentNode;
            while (outputNode != nullptr) {
                if (!outputNode->patternIndices.empty()) {
                    for (int patternIndex : outputNode->patternIndices) {
                        matches.push_back({patternIndex, i});
                    }
                }
                outputNode = outputNode->outputLink;
            }
        }
        return matches;
    }

    /**
     * @brief Retrieves the pattern at the specified index.
     * 
     * This function returns a const reference to the pattern stored at the given
     * index in the internal list of patterns. If the index is out of bounds, it
     * returns a const reference to an empty string.
     * 
     * @param index The index of the pattern to retrieve.
     * @return A const reference to the pattern at the specified index, or a const
     *         reference to an empty string if the index is invalid.
     */

    const string& getPattern(int index) const {
         if (index >= 0 && index < patterns.size()) {
            return patterns[index];
        }
        static const string empty = "";
        return empty;
    }
};

void runTest(const string& testName,
             const vector<string>& patterns,
             const string& text,
             vector<pair<int, int>> expected_matches
) {
    cout << "Running test: " << testName << "..." << endl;
    AhoCorasick ac;
    for (const auto& p : patterns) {
        ac.addPattern(p);
    }
    ac.buildFailureLinks();

    vector<pair<int, int>> actual_matches = ac.search(text);

    auto sort_key = [](const pair<int, int>& a, const pair<int, int>& b) {
        if (a.second != b.second) {
            return a.second < b.second;
        }
        return a.first < b.first;
    };
    sort(actual_matches.begin(), actual_matches.end(), sort_key);
    sort(expected_matches.begin(), expected_matches.end(), sort_key);

    assert(actual_matches == expected_matches);

    cout << "Test '" << testName << "' PASSED." << endl << endl;
}

void testAhoCorasick() {
    cout << "--- Starting AhoCorasick Tests ---" << endl;

    // Test Case 1: Simple Non-Overlapping
    runTest("Simple Non-Overlapping",
            {"a", "b", "c"},
            "abc",
            {{0, 0}, {1, 1}, {2, 2}}
    );

    // Test Case 2: Standard Overlap Example
    runTest("Standard Overlap",
            {"he", "she", "his", "hers"},
            "ushers",
            {{0, 3}, {1, 3}, {3, 5}}
    );

    // Test Case 3: Prefix/Suffix/Overlap Mix
    runTest("Prefix/Suffix/Overlap Mix",
            {"a", "ab", "bab", "bc", "bca", "c", "caa"},
            "abccab",
            {{0, 0}, {1, 1}, {3, 2}, {5, 2}, {5, 3}, {0, 4}, {1, 5}}
    );

    // Test Case 4: Multiple Occurrences of Same Pattern
     runTest("Multiple Occurrences",
            {"aba"},
            "ababaxaba",
            {{0, 2}, {0, 4}, {0, 8}}
    );

     // Test Case 5: No Matches
    runTest("No Matches",
            {"xyz", "123"},
            "abcde",
            {}
    );

     // Test Case 6: Empty Text
    runTest("Empty Text",
            {"a", "b"},
            "",
            {}
    );

    // Test Case 7: Empty Patterns
    runTest("Empty Patterns",
            {},
            "abc",
            {}
    );

    // Test Case 8: Longer Text and More Overlaps (mississippi example)
    runTest("Complex Overlaps (Mississippi)",
            {"i", "is", "ppi", "sip", "mississippi"},
            "mississippi",
            {
                {0, 1},
                {1, 2},
                {0, 4},
                {1, 5},
                {0, 7},
                {3, 8},
                {0, 10},
                {2, 10},
                {4, 10}
            }
    );

    // Test Case 9: Patterns ending at the same place
    runTest("Patterns Ending Together",
            {"a", "ba", "cba"},
            "dcba",
            { {0, 3}, {1, 3}, {2, 3} }
    );


    cout << "--- All AhoCorasick Tests Passed! ---" << endl;
}

void runAhoCorasickSample() {
    AhoCorasick ac;

    vector<string> patterns_to_add = {"a", "ab", "bab", "bc", "bca", "c", "caa"};
    for (const auto& p : patterns_to_add) {
        ac.addPattern(p);
    }

    ac.buildFailureLinks();

    string text = "abcabcabcabcaab";

    cout << "Searching in text: \"" << text << "\"" << endl;
    cout << "Patterns: ";
    for(size_t i = 0; i < patterns_to_add.size(); ++i) {
        cout << "'" << patterns_to_add[i] << "'(" << i << ") ";
    }
    cout << endl << "Matches found:" << endl;

    vector<pair<int, int>> matches = ac.search(text);

    for (const auto& match : matches) {
        int patternIndex = match.first;
        int endPosition = match.second;
        string patternStr = ac.getPattern(patternIndex);
        int startPosition = endPosition - patternStr.length() + 1;

        cout << "  Pattern '" << patternStr << "' (Index " << patternIndex << ") found ending at index " << endPosition
                  << " (Span: [" << startPosition << ", " << endPosition << "])" << endl;
    }
     if (matches.empty()) {
        cout << "  No matches found." << endl;
    }

}

int main() {
    testAhoCorasick();
    runAhoCorasickSample();
    return 0;
}