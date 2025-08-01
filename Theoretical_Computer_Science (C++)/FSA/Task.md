# FSA to RegExp Translator

**time limit per test:** 1 second  
**memory limit per test:** 256 megabytes

You are given a description of a Finite State Automaton (FSA), which can be either deterministic or non-deterministic. Your task is to validate the input and either report the first encountered error, or convert the FSA into a corresponding regular expression that accepts the same language.

The FSA may include epsilon (ε) transitions, denoted in the input as the special symbol `eps`, which allows transitions without consuming any input symbol.

## Constraints

- State and alphabet identifiers must consist of Latin letters, digits, or underscores.  
- The symbol `eps` (representing ε) is allowed only in non-deterministic FSAs.  
- The FSA must conform to the specified format and semantic rules.

## Input

The input consists of six labeled lines describing the FSA. All keys are in lowercase and followed by an equal sign and a list or value:

```
type=[t]                    t ∈ {deterministic, non-deterministic}
states=[s1, s2,...]         state names: letters, digits, or underscores
alphabet=[a1, a2,...]       symbols: letters, digits, underscores; ε is represented as eps
initial=[s]                 single initial state
accepting=[s1, s2,...]      list of accepting states
transitions=[s1>a>s2,...]   transitions using symbols from the alphabet or eps
```

## Output

Output format. Your program should output only **one** of the following:

- A valid regular expression corresponding to the FSA's language.  
- The first applicable error message, chosen from the list below.

### Validation errors

- `Input is malformed.` – syntax errors or invalid line structure  
- `Initial state is not defined.` – missing or undefined initial state  
- `Set of accepting states is empty.` – no accepting states defined  
- `A state 's' is not in the set of states.` – used in initial, accepting, or transitions but not declared  
- `A transition symbol 'a' is not in the alphabet.` – symbol not listed in the alphabet (excluding eps)  
- `Some states are disjoint.` – unreachable from initial state or cannot reach any accepting state  
- `FSA is non-deterministic.` – declared as deterministic but behaves as non-deterministic  

## Examples

### Input
```
type=[deterministic]
states=[a]
alphabet=[a]
initial=[a]
accepting=[a]
transitions=[a>a>a]
```

### Output
```
((a|eps)(a|eps)*(a|eps)|(a|eps))
```

---

### Input
```
type=[deterministic]
states=[on,off]
alphabet=[turn_on,turn_off]
initial=[]
accepting=[on,off]
transitions=[off>turn_on>on,on>turn_off>off,on>turn_on>on,off>turn_off>off]
```

### Output
```
Initial state is not defined.
```

---

### Input
```
type=[non-deterministic]
states=[q0,q1]
alphabet=[0,1]
initial=[q0]
accepting=[q1]
transitions=[q0>0>q0,q0>1>q0,q1>0>q1,q1>1>q1,q1>0>q0,q1>1>q0]
```

### Output
```
Some states are disjoint.
```

## Note

**Kleene's Algorithm:**

The Kleene's Algorithm should be used as presented in the Lab, but with the following modifications:

- Denote ϕ as `{}`  
- Denote ε as `eps`  
- Define update rule with the additional parentheses:  
  ```
  Rkij = (Rk−1ik)(Rk−1kk)* (Rk−1kj) | (Rk−1ij)
  ```

- At each step, each regular expression should be surrounded by parentheses, e.g.  
  ```
  Rkij = ((a|eps)(a|eps)*(a|eps)|(a|eps))
  ```

- Optimization of the regular expressions should **NOT** be done  
- The content of each regular expression part must follow these rules:
  - Accepted states must appear in **lexicographical order**.
  - The symbol `eps` must be placed **at the end** of each part.
  - All other elements should follow the same order as defined in the transitions.
- Assume that input and standard outputs end with a newline character.
