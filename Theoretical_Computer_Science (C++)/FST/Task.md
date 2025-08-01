# Sub-string Detector Using FST

**time limit per test:** 1 second  
**memory limit per test:** 256 megabytes

You are given a sub-string `C` and string `S`, both containing only lowercase letters (`a-z`). Your task is to build a FST that detect the existence of `C` in `S` and counting the number of repetitions. If it is not detected, print `NOT DETECTED`.

You must solve the problem using a Finite State Transducer (FST). Your implementation must follow these requirements:

Implement a class named `lastname_firstname_FST`, where `lastname` and `firstname` should be replaced with your actual last and first names.  
The class constructor must take a substring as a parameter.  
Implement a function inside the class that takes a character as input and returns `1` or `0`, depending on the FST's output.

```python
class lastname_firstname_FST:
    def __init__(self, substring):
        # Initialize states, transitions, and output logic
        self.substring = substring
        self.state = START_STATE  

    def process(self, char):
        # Update state based on input character and substring
        self.state = transition_function(self.state, char, self.substring)
        
        # Determine output based on new state
        return output_function(self.state)

# Example usage:
fst = lastname_firstname_FST("abc")  # Create an FST for substring "abc"
result = fst.process('a')  # Returns 1 or 0 based on the FST logic
```

## Input

The first line contains the sub-string `C` of length `M` :  
`(1 ≤ M ≤ 100)`

The next line contains the string `S` of length `N` :  
`(1 ≤ N ≤ 1000)`

## Output

The number of repetition of `C` in `S`, or if it is not detected, `NOT DETECTED`.
