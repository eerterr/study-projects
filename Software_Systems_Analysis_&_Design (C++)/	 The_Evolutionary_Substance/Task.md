# The Evolutionary Substance

**time limit per test:** 1 second  
**memory limit per test:** 256 megabytes

## Task

Inspired by the movie *"Substance"*. Imagine we have a substance that allows us to create a "better" version of a living creature. In our case, applying the substance will only transform a living creature into a new type, **halving the current number of days lived (rounded up)**. 

> Keep in mind that in comparison with the movie, the second creature is not created.

- The number of days lived (`daysLived`) for each animal is limited to **10 days** – the animal dies as soon as the **11th day** arrives.
- It is **recommended** to apply the substance to the animal **only once**, but if the substance is **overused**, the "better" version of the animal will turn into a **monster** that **automatically kills** all nearby animals on the **same day**.

The substance **can be removed** from the organisms of only the `"better"` types of the animals, then the animal will revert to a **normal type**, and the days lived will be **doubled**.  
> If in this case the number of days lived exceeds 10, the animal will die immediately upon the arrival of the next day.

---

## Animal Type Transformations

The types of animals are shown in the **class diagram**.  
The substance can be applied to a creature of type:

- `Mouse` → `BetterMouse`
- `Fish` → `BetterFish`
- `Bird` → `BetterBird`

When the substance is applied **again** to a `"better"` type animal, it becomes a **`Monster`**, an **irrevertible** type of creature living only **1 day**.

---

## Animal Behavior

- Any living creature can **attack** any other creature in the **same container** – in this case, the one who is attacked **dies**.
- All animals are strictly distributed according to their type in template-based containers:

| Container Type   | Allowed Animals                       |
|------------------|----------------------------------------|
| `Cage<T>`        | Mice or Birds                         |
| `Aquarium<T>`    | Mice or Fish                          |
| `Freedom<A>`     | Any animals, even monsters            |

- `"Better"` types of animals have their own separate containers.
- Applying or removing the substance **moves** the animal to another container.
- When a **Monster appears** in a container, **all animals** in that container **die**, and the **Monster is moved to `Freedom`**.

---

## Containers

Create containers of **all possible animal types** except Monsters in advance, in the following order for updates:

1. Cage<Bird>
2. Cage<BetterBird>
3. Cage<Mouse>
4. Cage<BetterMouse>
5. Aquarium<Fish>
6. Aquarium<BetterFish>
7. Aquarium<Mouse>
8. Aquarium<BetterMouse>
9. Freedom<Animal>

> Reorder animals in the container on every update by:
> 1. Ascending order of `daysLived`
> 2. Lexicographical order of `name` (if tie)

---

## Notes

- Animals **cannot attack** in `Freedom`.
- Substance **cannot be applied or removed** in `Freedom`.
- `Cage<Fish>` and `Aquarium<Bird>` must be deleted via template specialization.
- Class hierarchy contains an abstract base class with `sayName()` as pure virtual.
- Implement all necessary constructors and methods.

---

## Console Commands

The input contains **C** – number of commands in the first line, then **C** lines with commands.  
Command reference:

| Command | Description | Output |
|--------|-------------|--------|
| `CREATE <TYPE> <NAME> IN <CONTAINER> <N>` | Creates an instance of `<TYPE>` with name `<NAME>` and days lived `<N>` and places in container `<CONTAINER>` | `My name is <NAME>, days lived: <N>` |
| `APPLY_SUBSTANCE <CONTAINER> <TYPE> <POS>` | Applies the substance to the animal at index `<POS>` | If in Freedom: `Substance cannot be applied in freedom` |
| `REMOVE_SUBSTANCE <CONTAINER> <TYPE> <POS>` | Removes the substance from the animal at index `<POS>` | If in Freedom: `Substance cannot be removed in freedom`<br>`Invalid substance removal` if not better type |
| `ATTACK <CONTAINER> <TYPE> <POS1> <POS2>` | Animal at `<POS1>` attacks animal at `<POS2>` | `<Classname> is attacking`, or `Animals cannot attack in Freedom` |
| `TALK <CONTAINER> <TYPE> <POS>` | Calls `sayName()` | `My name is <NAME>, days lived: <N>` |
| `PERIOD` | Adds +1 day to every animal’s age in order | If animal dies: `<NAME> has died of old days` |

- If no animal at `<POS>`, print `Animal not found`.
- Animal Type Codes:

| Code | Type |
|------|------|
| M | Mouse |
| BM | BetterMouse |
| F | Fish |
| BF | BetterFish |
| B | Bird |
| BB | BetterBird |

---

## Examples

### Input
```
5
CREATE M Sparkle IN Cage 5
CREATE M Sue IN Cage 8
APPLY_SUBSTANCE Cage M 0
TALK Cage M 0
TALK Cage BM 0
```

### Output
```
My name is Sparkle, days lived: 5
My name is Sue, days lived: 8
My name is Sue, days lived: 8
My name is Sparkle, days lived: 3
```

---

### Input
```
5
CREATE M Nickey IN Cage 8
APPLY_SUBSTANCE Cage M 0
TALK Cage BM 0
REMOVE_SUBSTANCE Cage BM 0
TALK Cage M 0
```

### Output
```
My name is Nickey, days lived: 8
My name is Nickey, days lived: 4
My name is Nickey, days lived: 8
```

---

### Input
```
4
CREATE M Nickey IN Cage 8
PERIOD
PERIOD
PERIOD
```

### Output
```
My name is Nickey, days lived: 8
Nickey has died of old days
```

---

### Input
```
5
CREATE M Mickey IN Cage 5
REMOVE_SUBSTANCE Cage M 0
CREATE M John IN Freedom 10
PERIOD
TALK Cage M 0
```

### Output
```
My name is Mickey, days lived: 5
Invalid substance removal
My name is John, days lived: 10
John has died of old days
My name is Mickey, days lived: 6
```

---

### Input
```
4
CREATE M Saprkle IN Cage 8
CREATE M Sue IN Cage 5
ATTACK Cage M 0 1
TALK Cage M 0
```

### Output
```
My name is Saprkle, days lived: 8
My name is Sue, days lived: 5
Mouse is attacking
My name is Sue, days lived: 5
```
