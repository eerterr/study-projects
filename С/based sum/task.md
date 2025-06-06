# A. Based Sum

**Time limit per test:** 1 second  
**Memory limit per test:** 256 megabytes  
**Input:** input.txt  
**Output:** output.txt  

## Problem Statement
You will be given:
- An integer N representing the amount of numbers to process
- A string S consisting of N space-separated numbers in binary, octal, decimal or hexadecimal
- An array A consisting of the bases for each number in S (values [2, 8, 10, 16])

### Task:
1. Convert all numbers to decimal
2. For even-indexed numbers: subtract 10
3. For odd-indexed numbers: add 10
4. Output the sum of resulting numbers or "Invalid inputs" if any number is invalid for its base

### Input Validation:
Numbers must be valid for their specified bases:
- Binary (2): digits [0,1]
- Octal (8): digits [0-7]
- Decimal (10): digits [0-9]
- Hexadecimal (16): digits [0-9,A-F]

## Examples

### Example 1
**Input:**
2
2059 8163
16 8

text
**Output:**
Invalid inputs

text
*Reason: "8163" contains invalid digit '8' for octal*

### Example 2
**Input:**
4
100101 1EF 274 2024
2 16 8 10

text
**Output:**
2744
