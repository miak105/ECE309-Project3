# Project 3 Design Log
**Author:** <Mia King> · **P2 code used:** <my own | instructor reference>
**Tools used:** <VS Code with WSL/Ubuntu, CMake, GCC, CTest, and ChatGPT. I used ChatGPT to help understand the specification, implement the C++ functions, develop additional tests, and troubleshoot errors. I reviewed, compiled, and tested the code in VS Code.>

## 1. Hash table
I implemented ToolRegistry using separate chaining. Each bucket contains a std::vector of std::unique_ptr<Tool> objects. This allows multiple tools to be stored in the same bucket if a collision occurs. I chose separate chaining instead of linear probing because it is straightforward to implement, and searching a bucket is simple.

For the hash function, I used FNV-1a. It processes every character of the tool name using XOR and multiplication. This helps distribute different names across the hash table. The resulting hash value is divided by the number of buckets using modulo to determine the correct bucket index.

My hash table starts with 16 buckets. I chose a maximum load factor of 0.75 because I wanted to reduce collisions as more tools were inserted. Before adding a new tool, the program checks whether the load factor would exceed 0.75. If it would, the number of buckets doubles and the existing tools are rehashed.

I tested the hash table by inserting 10,000 tools named tool_0 through tool_9999. With FNV-1a, the maximum chain length was 6. The expected final bucket count is 16,384, giving a load factor of approximately 0.6104. Using the weaker hash function s[0] + s.size() with the same tool names resulted in a maximum chain length of 9,000. This comparison shows why having a good hash function is important. Even with enough buckets, a poor hash function can cause many collisions.

The add() function has an average amortized time complexity of O(1), assuming well-distributed hashes and reasonably short names. Resizing takes O(n), but because the table doubles in size, resizing becomes less frequent as the table grows.

## 2. Pointer stability
The registry uses std::unique_ptr to manage ownership of each tool. When the hash table resizes, the unique pointers are moved into new buckets, but the actual tool objects stay at the same memory addresses.

This is important because find() returns a regular Tool* pointer. If resizing changed the tool's memory address, that pointer would become invalid. By moving ownership rather than moving the actual objects, pointers returned by find() remain valid after resizing. The registry also automatically destroys its owned tools when it is destroyed.

## 3. Calculator trace
I implemented the calculator using two stacks: one for numbers and another for operators. For the expression `2 * (3 + 4) - 5`, the calculator first pushes 2 onto the values stack and multiplication onto the operators stack. When it reaches the opening parenthesis, it pushes that onto the operators stack as well. Next, it pushes 3 onto the values stack, followed by the addition operator and then 4. When the closing parenthesis is reached, the calculator adds 3 and 4 to get 7. The values stack now contains 2 and 7. When the subtraction operator is encountered, multiplication is performed first because it has higher precedence. This gives 14. The calculator then pushes the subtraction operator and the number 5 onto their respective stacks. Finally, it subtracts 5 from 14 to get the final answer of 9. This process ensures that parentheses and operator precedence are handled correctly.

## 4. Errors as values
I used error messages and return values instead of exceptions for expected failures. For example, Workspace::resolve() returns false when a path is unsafe, while the calculator returns false when an expression divides by zero.

This makes sense because a failed tool request does not necessarily mean the program is broken. The AI assistant can receive the error message and try another action without stopping the entire program. Exceptions are more appropriate for unexpected programming errors or failures that cannot be handled normally.

## 5. What I would design differently
If I completed this project again, I would write more tests before implementing the functions. This would help identify edge cases earlier, especially for path validation and operator precedence. I would also organize the tests by functionality from the beginning, making it easier to identify which part of the program was causing a problem.